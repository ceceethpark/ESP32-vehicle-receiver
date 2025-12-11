#include "LcdControl.h"
#include <string.h>
#include <stdio.h>
#include "./font/kssm_font.h"
#include "./font/english.h"

LcdControl::LcdControl(spi_host_device_t spi_host, gpio_num_t cs_pin, 
                       gpio_num_t dc_pin, gpio_num_t rst_pin, gpio_num_t bl_pin)
    : spi_host_(spi_host)
    , spi_handle_(nullptr)
    , cs_pin_(cs_pin)
    , dc_pin_(dc_pin)
    , rst_pin_(rst_pin)
    , bl_pin_(bl_pin)
    , initialized_(false)
    , brightness_(255)
    , ui_task_handle_(nullptr)
    , ui_task_running_(false)
    , data_mutex_(nullptr)
{
    memset(&vehicle_data_, 0, sizeof(vehicle_data_));
}

LcdControl::~LcdControl() {
    end();
}

esp_err_t LcdControl::begin() {
    if (initialized_) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }
    
    // GPIO 설정 (DC, RST, BL)
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << dc_pin_) | (1ULL << rst_pin_) | (1ULL << bl_pin_);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // SPI 버스 초기화
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = GPIO_NUM_23;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = GPIO_NUM_18;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2 + 8;
    
    ret = spi_bus_initialize(spi_host_, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // SPI 디바이스 추가
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 40 * 1000 * 1000; // 40 MHz
    devcfg.mode = 0;
    devcfg.spics_io_num = cs_pin_;
    devcfg.queue_size = 7;
    devcfg.pre_cb = nullptr;
    
    ret = spi_bus_add_device(spi_host_, &devcfg, &spi_handle_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // LCD 리셋
    gpio_set_level(rst_pin_, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(rst_pin_, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // LCD 초기화 시퀀스
    initSequence();
    
    // 백라이트 ON
    gpio_set_level(bl_pin_, 1);
    
    // Mutex 생성
    data_mutex_ = xSemaphoreCreateMutex();
    if (data_mutex_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "LCD initialized (%dx%d)", LCD_WIDTH, LCD_HEIGHT);
    
    clear(BLACK);
    return ESP_OK;
}

void LcdControl::end() {
    stopUITask();
    
    if (data_mutex_) {
        vSemaphoreDelete(data_mutex_);
        data_mutex_ = nullptr;
    }
    
    if (spi_handle_) {
        displayOff();
        gpio_set_level(bl_pin_, 0);
        spi_bus_remove_device(spi_handle_);
        spi_handle_ = nullptr;
    }
    
    initialized_ = false;
}

void LcdControl::initSequence() {
    // ST7789 초기화 명령 (간단 버전)
    writeCommand(0x01); // Software Reset
    vTaskDelay(pdMS_TO_TICKS(150));
    
    writeCommand(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(255));
    
    writeCommand(0x3A); // Color Mode
    writeData(0x55);    // 16-bit RGB565
    
    writeCommand(0x36); // Memory Access Control
    writeData(0x00);    // Normal
    
    writeCommand(0x29); // Display ON
    vTaskDelay(pdMS_TO_TICKS(100));
}

void LcdControl::writeCommand(uint8_t cmd) {
    gpio_set_level(dc_pin_, 0); // Command mode
    spiWrite(&cmd, 1);
}

void LcdControl::writeData(uint8_t data) {
    gpio_set_level(dc_pin_, 1); // Data mode
    spiWrite(&data, 1);
}

void LcdControl::writeData16(uint16_t data) {
    uint8_t buf[2] = {(uint8_t)(data >> 8), (uint8_t)(data & 0xFF)};
    gpio_set_level(dc_pin_, 1);
    spiWrite(buf, 2);
}

void LcdControl::spiWrite(const uint8_t* data, size_t len) {
    if (len == 0) return;
    
    spi_transaction_t trans = {};
    trans.length = len * 8;
    trans.tx_buffer = data;
    spi_device_transmit(spi_handle_, &trans);
}

void LcdControl::clear(Color color) {
    fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void LcdControl::setBrightness(uint8_t brightness) {
    brightness_ = brightness;
    // TODO: PWM으로 구현
}

void LcdControl::displayOn() {
    writeCommand(0x29);
}

void LcdControl::displayOff() {
    writeCommand(0x28);
}

void LcdControl::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(0x2A); // Column Address Set
    writeData16(x0);
    writeData16(x1);
    
    writeCommand(0x2B); // Row Address Set
    writeData16(y0);
    writeData16(y1);
    
    writeCommand(0x2C); // Memory Write
}

void LcdControl::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    
    setWindow(x, y, x + w - 1, y + h - 1);
    
    gpio_set_level(dc_pin_, 1);
    uint16_t pixel_count = w * h;
    uint8_t buf[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
    
    for (uint16_t i = 0; i < pixel_count; i++) {
        spiWrite(buf, 2);
    }
}

void LcdControl::drawPixel(uint16_t x, uint16_t y, Color color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    setWindow(x, y, x, y);
    writeData16(color);
}

void LcdControl::printText(const char* text, uint16_t x, uint16_t y, Color color, uint8_t size) {
    // 간단한 텍스트 출력 (실제로는 폰트 라이브러리 필요)
    ESP_LOGI(TAG, "Text at (%d,%d): %s", x, y, text);
}

void LcdControl::printKoreanText(const char* text, uint16_t x, uint16_t y, Color fg_color, Color bg_color, uint8_t size, bool no_bg) {
    uint16_t xpos = x;
    uint16_t ypos = y;
    
    while (*text) {
        uint8_t ch1 = *text++;
        if (ch1 < 0x80) {
            // ASCII
            draw16English(ch1, xpos, ypos, fg_color, bg_color, size, no_bg);
            xpos += 8 * size;
        } else {
            // 한글 (UTF-8 3바이트)
            uint8_t ch2 = *text++;
            uint8_t ch3 = *text++;
            uint16_t hangeul = (ch1 & 0x0F) << 12 | (ch2 & 0x3F) << 6 | (ch3 & 0x3F);
            draw16Korean(hangeul, xpos, ypos, fg_color, bg_color, size, no_bg);
            xpos += 16 * size;
        }
        
        // 화면 끝에 도달하면 다음 줄로
        if (xpos > LCD_WIDTH - 16 * size) {
            xpos = x;
            ypos += 16 * size;
        }
    }
}

void LcdControl::draw16English(uint8_t ch, uint16_t x, uint16_t y, Color fg, Color bg, uint8_t size, bool no_bg) {
    setWindow(x, y, x + 8 * size - 1, y + 16 * size - 1);
    gpio_set_level(dc_pin_, 1);
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            for (int xx = 0; xx < size; xx++) {
                for (int k = 0; k < 8; k++) {
                    uint8_t data = (english[ch][i * 8 + k]) & (1 << j);
                    uint8_t buf[2];
                    Color pixel_color = data ? fg : bg;
                    
                    if (data || !no_bg) {
                        buf[0] = (uint8_t)(pixel_color >> 8);
                        buf[1] = (uint8_t)(pixel_color & 0xFF);
                        for (int t = 0; t < size; t++) {
                            spiWrite(buf, 2);
                        }
                    }
                }
            }
        }
    }
}

void LcdControl::draw16Korean(uint16_t hangeul, uint16_t x, uint16_t y, Color fg, Color bg, uint8_t size, bool no_bg) {
    uint8_t cho1[22] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 };
    uint8_t cho2[22] = { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 };
    uint8_t jong[22] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 };
    
    uint8_t Korean_buffer[32] = {0,};
    
    // 한글 유니코드 분해 (0xAC00='가' ~ 0xD7A3='힣')
    hangeul -= 0xAC00;
    uint8_t last = hangeul % 28;  // 종성
    hangeul /= 28;
    uint8_t first = hangeul / 21 + 1;  // 초성
    uint8_t mid = hangeul % 21 + 1;    // 중성
    
    uint8_t firstType, midType, lastType = 0;
    
    if (last == 0) {  // 받침 없는 경우
        firstType = cho1[mid];
        midType = (first == 1 || first == 24) ? 0 : 1;
    } else {  // 받침 있는 경우
        firstType = cho2[mid];
        midType = (first == 1 || first == 24) ? 2 : 3;
        lastType = jong[mid];
    }
    
    // 초성 조합
    uint16_t pF_temp = firstType * 20 + first;
    for (int i = 0; i < 32; i++) {
        Korean_buffer[i] = K_font[pF_temp][i];
    }
    
    // 중성 조합
    pF_temp = 160 + midType * 22 + mid;
    for (int i = 0; i < 32; i++) {
        Korean_buffer[i] |= K_font[pF_temp][i];
    }
    
    // 종성 조합
    if (last) {
        pF_temp = 248 + lastType * 28 + last;
        for (int i = 0; i < 32; i++) {
            Korean_buffer[i] |= K_font[pF_temp][i];
        }
    }
    
    // 화면에 출력
    setWindow(x, y, x + 16 * size - 1, y + 16 * size - 1);
    gpio_set_level(dc_pin_, 1);
    
    for (int i = 0; i < 16; i++) {
        for (int xx = 0; xx < size; xx++) {
            uint16_t data = (Korean_buffer[i] << 8) | Korean_buffer[i + 16];
            for (int j = 0; j < 16; j++) {
                uint8_t buf[2];
                Color pixel_color = (data & 0x8000) ? fg : bg;
                
                if ((data & 0x8000) || !no_bg) {
                    buf[0] = (uint8_t)(pixel_color >> 8);
                    buf[1] = (uint8_t)(pixel_color & 0xFF);
                    for (int t = 0; t < size; t++) {
                        spiWrite(buf, 2);
                    }
                }
                data <<= 1;
            }
        }
    }
}

void LcdControl::printTextCentered(const char* text, uint16_t y, Color color, uint8_t size) {
    uint16_t x = (LCD_WIDTH - strlen(text) * 6 * size) / 2;
    printText(text, x, y, color, size);
}

void LcdControl::drawMainScreen() {
    clear(BLACK);
    
    // 상단 헤더 - 파란색 배경
    fillRect(0, 0, LCD_WIDTH, 40, BLUE);
    printTextCentered("Vehicle Receiver", 12, WHITE, 2);
    
    // 구분선
    fillRect(0, 40, LCD_WIDTH, 2, GRAY);
}

void LcdControl::updateVehicleData(uint8_t speed, uint8_t battery, int16_t motor_temp,
                                   uint16_t motor_current, int16_t fet_temp) {
    if (xSemaphoreTake(data_mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        vehicle_data_.speed = speed;
        vehicle_data_.battery = battery;
        vehicle_data_.motor_temp = motor_temp;
        vehicle_data_.motor_current = motor_current;
        vehicle_data_.fet_temp = fet_temp;
        vehicle_data_.data_changed = true;
        xSemaphoreGive(data_mutex_);
    }
}

void LcdControl::updateControlMode(const char* mode) {
    if (xSemaphoreTake(data_mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        strncpy(vehicle_data_.mode, mode, sizeof(vehicle_data_.mode) - 1);
        vehicle_data_.data_changed = true;
        xSemaphoreGive(data_mutex_);
    }
}

void LcdControl::updateConnectionStatus(bool connected) {
    if (xSemaphoreTake(data_mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        vehicle_data_.connected = connected;
        vehicle_data_.data_changed = true;
        xSemaphoreGive(data_mutex_);
    }
}

esp_err_t LcdControl::startUITask(uint32_t stack_size, UBaseType_t priority) {
    if (ui_task_running_) {
        ESP_LOGW(TAG, "UI task already running");
        return ESP_OK;
    }
    
    BaseType_t ret = xTaskCreate(
        uiTaskWrapper,
        "lcd_ui",
        stack_size,
        this,
        priority,
        &ui_task_handle_
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UI task");
        return ESP_FAIL;
    }
    
    ui_task_running_ = true;
    ESP_LOGI(TAG, "UI task started");
    return ESP_OK;
}

void LcdControl::stopUITask() {
    if (ui_task_running_ && ui_task_handle_) {
        ui_task_running_ = false;
        vTaskDelete(ui_task_handle_);
        ui_task_handle_ = nullptr;
        ESP_LOGI(TAG, "UI task stopped");
    }
}

void LcdControl::renderUI() {
    if (xSemaphoreTake(data_mutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        if (vehicle_data_.data_changed) {
            // UI 업데이트 - 이전 영역 지우기
            fillRect(0, 42, LCD_WIDTH, LCD_HEIGHT - 42, BLACK);
            
            char buf[64];
            uint16_t y_pos = 50;
            
            // 속도 (한글)
            snprintf(buf, sizeof(buf), "속도: %d km/h", vehicle_data_.speed);
            printKoreanText(buf, 10, y_pos, WHITE, BLACK, 1, false);
            y_pos += 20;
            
            // 배터리 (한글)
            Color bat_color = vehicle_data_.battery > 30 ? GREEN : (vehicle_data_.battery > 15 ? YELLOW : RED);
            snprintf(buf, sizeof(buf), "배터리: %d%%", vehicle_data_.battery);
            printKoreanText(buf, 10, y_pos, bat_color, BLACK, 1, false);
            y_pos += 20;
            
            // 모터 온도 (한글)
            snprintf(buf, sizeof(buf), "모터온도: %dC", vehicle_data_.motor_temp);
            printKoreanText(buf, 10, y_pos, CYAN, BLACK, 1, false);
            y_pos += 20;
            
            // 모터 전류 (한글)
            snprintf(buf, sizeof(buf), "전류: %dmA", vehicle_data_.motor_current);
            printKoreanText(buf, 10, y_pos, YELLOW, BLACK, 1, false);
            y_pos += 20;
            
            // FET 온도 (한글)
            snprintf(buf, sizeof(buf), "FET: %dC", vehicle_data_.fet_temp);
            printKoreanText(buf, 10, y_pos, MAGENTA, BLACK, 1, false);
            y_pos += 20;
            
            // 모드 표시 (한글)
            snprintf(buf, sizeof(buf), "모드: %s", vehicle_data_.mode);
            printKoreanText(buf, 10, y_pos, YELLOW, BLACK, 1, false);
            y_pos += 20;
            
            // 연결 상태 (한글)
            if (vehicle_data_.connected) {
                printKoreanText("연결됨", 10, y_pos, GREEN, BLACK, 1, false);
            } else {
                printKoreanText("연결끊김", 10, y_pos, RED, BLACK, 1, false);
            }
            
            vehicle_data_.data_changed = false;
        }
        xSemaphoreGive(data_mutex_);
    }
}

void LcdControl::uiTaskWrapper(void* parameter) {
    LcdControl* self = static_cast<LcdControl*>(parameter);
    
    self->drawMainScreen();
    
    while (self->ui_task_running_) {
        self->renderUI();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz 업데이트
    }
    
    vTaskDelete(NULL);
}

void LcdControl::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
    // TODO: 구현
}

void LcdControl::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, Color color) {
    // TODO: 구현
}

void LcdControl::drawCircle(uint16_t x, uint16_t y, uint16_t r, Color color) {
    // TODO: 구현
}

void LcdControl::fillCircle(uint16_t x, uint16_t y, uint16_t r, Color color) {
    // TODO: 구현
}

void LcdControl::updateVehicleDisplay(const VehicleData& data) {
    // TODO: 구현
}

void LcdControl::drawStatusBar() {
    // TODO: 구현
}

void LcdControl::drawVehicleInfo() {
    // TODO: 구현
}
