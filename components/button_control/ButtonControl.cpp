#include "ButtonControl.h"
#include <string.h>

// PCA9555 레지스터 주소
#define PCA9555_INPUT_PORT_0    0x00
#define PCA9555_INPUT_PORT_1    0x01
#define PCA9555_OUTPUT_PORT_0   0x02
#define PCA9555_OUTPUT_PORT_1   0x03
#define PCA9555_POLARITY_INV_0  0x04
#define PCA9555_POLARITY_INV_1  0x05
#define PCA9555_CONFIG_PORT_0   0x06
#define PCA9555_CONFIG_PORT_1   0x07

ButtonControl::ButtonControl()
    : num_buttons_(0)
    , use_i2c_(false)
    , i2c_port_(I2C_NUM_0)
    , i2c_addr_(0x20)
    , i2c_initialized_by_this_(false)
    , button_callback_(nullptr)
    , remote_command_callback_(nullptr)
    , scan_task_handle_(nullptr)
    , scan_task_running_(false)
{
    memset(button_pins_, 0, sizeof(button_pins_));
    memset(pca_pins_, 0, sizeof(pca_pins_));
    memset(button_states_, 0, sizeof(button_states_));
    memset(&toggle_state_, 0, sizeof(toggle_state_));
}

ButtonControl::~ButtonControl() {
    end();
}

esp_err_t ButtonControl::begin(const gpio_num_t* button_pins, uint8_t num_buttons) {
    if (num_buttons > BUTTON_COUNT) {
        ESP_LOGE(TAG, "Too many buttons: %d (max %d)", num_buttons, BUTTON_COUNT);
        return ESP_ERR_INVALID_ARG;
    }
    
    use_i2c_ = false;
    num_buttons_ = num_buttons;
    memcpy(button_pins_, button_pins, num_buttons * sizeof(gpio_num_t));
    
    // GPIO 설정
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    
    for (uint8_t i = 0; i < num_buttons_; i++) {
        io_conf.pin_bit_mask = (1ULL << button_pins_[i]);
        esp_err_t ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure button GPIO %d: %s", 
                     button_pins_[i], esp_err_to_name(ret));
            return ret;
        }
    }
    
    // 초기 상태 설정
    for (uint8_t i = 0; i < num_buttons_; i++) {
        button_states_[i].debounceTime = DEBOUNCE_MS;
    }
    
    ESP_LOGI(TAG, "Initialized %d buttons", num_buttons_);
    return ESP_OK;
}

void ButtonControl::end() {
    stopScanTask();
    
    // GPIO 해제
    for (uint8_t i = 0; i < num_buttons_; i++) {
        gpio_reset_pin(button_pins_[i]);
    }
    
    num_buttons_ = 0;
    ESP_LOGI(TAG, "Button control terminated");
}

esp_err_t ButtonControl::initialize(const gpio_num_t* button_pins, uint8_t num_buttons, 
                                    ButtonCallback btn_callback, RemoteCommandCallback remote_callback)
{
    esp_err_t ret = begin(button_pins, num_buttons);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Button control begin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    setButtonCallback(btn_callback);
    setRemoteCommandCallback(remote_callback);
    
    ESP_LOGI(TAG, "Button control fully initialized (%d buttons)", num_buttons);
    return ESP_OK;
}

esp_err_t ButtonControl::beginI2C(i2c_port_t i2c_port, gpio_num_t sda_pin, gpio_num_t scl_pin,
                                  uint8_t i2c_addr, const uint8_t* button_pins, uint8_t num_buttons)
{
    if (num_buttons > BUTTON_COUNT) {
        ESP_LOGE(TAG, "Too many buttons: %d (max %d)", num_buttons, BUTTON_COUNT);
        return ESP_ERR_INVALID_ARG;
    }
    
    use_i2c_ = true;
    i2c_port_ = i2c_port;
    i2c_addr_ = i2c_addr;
    num_buttons_ = num_buttons;
    memcpy(pca_pins_, button_pins, num_buttons * sizeof(uint8_t));
    
    // I2C 마스터 설정
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin;
    conf.scl_io_num = scl_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    conf.clk_flags = 0;
    
    esp_err_t ret = i2c_param_config(i2c_port_, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(i2c_port_, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    i2c_initialized_by_this_ = true;
    
    // PCA9555 통신 테스트
    uint8_t test_value;
    ret = pca9555ReadRegister(PCA9555_INPUT_PORT_0, &test_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCA9555 communication test failed at address 0x%02X", i2c_addr_);
        i2c_driver_delete(i2c_port_);
        i2c_initialized_by_this_ = false;
        return ret;
    }
    
    // PCA9555 핀을 입력으로 설정
    for (uint8_t i = 0; i < num_buttons_; i++) {
        uint8_t pin = pca_pins_[i];
        uint8_t port = pin / 8;
        uint8_t bit = pin % 8;
        uint8_t reg = (port == 0) ? PCA9555_CONFIG_PORT_0 : PCA9555_CONFIG_PORT_1;
        
        // 현재 설정 읽기
        uint8_t config;
        ret = pca9555ReadRegister(reg, &config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read PCA9555 config: %s", esp_err_to_name(ret));
            return ret;
        }
        
        // 비트 설정 (1=입력)
        config |= (1 << bit);
        ret = pca9555WriteRegister(reg, config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure PCA9555 pin %d: %s", pin, esp_err_to_name(ret));
            return ret;
        }
    }
    
    // 초기 상태 설정
    for (uint8_t i = 0; i < num_buttons_; i++) {
        button_states_[i].debounceTime = DEBOUNCE_MS;
    }
    
    ESP_LOGI(TAG, "Initialized %d I2C buttons (PCA9555 at 0x%02X)", num_buttons, i2c_addr_);
    return ESP_OK;
}

esp_err_t ButtonControl::initializeI2C(i2c_port_t i2c_port, gpio_num_t sda_pin, gpio_num_t scl_pin,
                                       uint8_t i2c_addr, const uint8_t* button_pins, uint8_t num_buttons,
                                       ButtonCallback btn_callback, RemoteCommandCallback remote_callback)
{
    esp_err_t ret = beginI2C(i2c_port, sda_pin, scl_pin, i2c_addr, button_pins, num_buttons);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Button control I2C begin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    setButtonCallback(btn_callback);
    setRemoteCommandCallback(remote_callback);
    
    ESP_LOGI(TAG, "Button control I2C fully initialized (%d buttons)", num_buttons);
    return ESP_OK;
}

bool ButtonControl::isPressed(ButtonID btn) {
    if (btn >= num_buttons_) return false;
    return button_states_[btn].isPressed;
}

bool ButtonControl::wasJustPressed(ButtonID btn) {
    if (btn >= num_buttons_) return false;
    bool result = button_states_[btn].isPressed && !button_states_[btn].wasPressed;
    return result;
}

bool ButtonControl::wasJustReleased(ButtonID btn) {
    if (btn >= num_buttons_) return false;
    bool result = !button_states_[btn].isPressed && button_states_[btn].wasPressed;
    return result;
}

void ButtonControl::setButtonCallback(ButtonCallback callback) {
    button_callback_ = callback;
}

void ButtonControl::scanButtons() {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    for (uint8_t i = 0; i < num_buttons_; i++) {
        bool current_state;
        
        // I2C 또는 GPIO에서 버튼 상태 읽기
        if (use_i2c_) {
            current_state = !pca9555ReadPin(pca_pins_[i]);  // PCA9555는 Active LOW (풀업)
        } else {
            current_state = gpio_get_level(button_pins_[i]) == 0;  // GPIO도 Active LOW
        }
        
        // 디바운스 처리
        if (current_state != button_states_[i].isPressed) {
            if (current_time - button_states_[i].pressTime >= button_states_[i].debounceTime) {
                button_states_[i].wasPressed = button_states_[i].isPressed;
                button_states_[i].isPressed = current_state;
                button_states_[i].pressTime = current_time;
                
                // 이벤트 처리
                if (current_state) {
                    processButtonEvent(static_cast<ButtonID>(i), EVENT_PRESSED);
                } else {
                    processButtonEvent(static_cast<ButtonID>(i), EVENT_RELEASED);
                }
            }
        } else if (current_state) {
            // 롱 프레스 체크
            uint32_t press_duration = current_time - button_states_[i].pressTime;
            if (press_duration >= LONG_PRESS_MS && !button_states_[i].wasPressed) {
                button_states_[i].wasPressed = true; // 한 번만 발생
                processButtonEvent(static_cast<ButtonID>(i), EVENT_LONG_PRESS);
            }
        }
    }
}

void ButtonControl::updateToggleState() {
    // PCA9555 기반 버튼은 토글 상태 없음 - 필요시 구현
    // 현재는 빈 함수로 유지
}

void ButtonControl::processButtonEvent(ButtonID id, ButtonEvent event) {
    const char* event_str = (event == EVENT_PRESSED) ? "PRESSED" : 
                           (event == EVENT_RELEASED) ? "RELEASED" : "LONG_PRESS";
    ESP_LOGI(TAG, "Button %d %s", id, event_str);
    
    if (button_callback_) {
        button_callback_(id, event);
    }
}

esp_err_t ButtonControl::startScanTask(uint32_t stack_size, UBaseType_t priority) {
    if (scan_task_running_) {
        ESP_LOGW(TAG, "Scan task already running");
        return ESP_OK;
    }
    
    BaseType_t ret = xTaskCreate(
        scanTaskWrapper,
        "button_scan",
        stack_size,
        this,
        priority,
        &scan_task_handle_
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create scan task");
        return ESP_FAIL;
    }
    
    scan_task_running_ = true;
    ESP_LOGI(TAG, "Scan task started");
    return ESP_OK;
}

void ButtonControl::stopScanTask() {
    if (scan_task_running_ && scan_task_handle_) {
        scan_task_running_ = false;
        vTaskDelete(scan_task_handle_);
        scan_task_handle_ = nullptr;
        ESP_LOGI(TAG, "Scan task stopped");
    }
}

void ButtonControl::setRemoteCommandCallback(RemoteCommandCallback callback) {
    remote_command_callback_ = callback;
}

void ButtonControl::handleRemoteCommand(uint8_t command_id) {
    ESP_LOGI(TAG, "Remote command received: %d", command_id);
    
    // 원격 명령 콜백 호출
    if (remote_command_callback_ != nullptr) {
        remote_command_callback_(command_id);
    }
}

void ButtonControl::scanTaskWrapper(void* parameter) {
    ButtonControl* self = static_cast<ButtonControl*>(parameter);
    
    while (self->scan_task_running_) {
        self->scanButtons();
        self->updateToggleState();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz 스캔
    }
    
    vTaskDelete(NULL);
}

// ==================== PCA9555 I2C 통신 메서드 ====================

esp_err_t ButtonControl::pca9555WriteRegister(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr_ << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(i2c_port_, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "PCA9555 write register 0x%02X failed: %s", reg, esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t ButtonControl::pca9555ReadRegister(uint8_t reg, uint8_t* value)
{
    // 레지스터 주소 쓰기
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr_ << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(i2c_port_, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "PCA9555 write reg address 0x%02X failed: %s", reg, esp_err_to_name(ret));
        return ret;
    }
    
    // 데이터 읽기
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr_ << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(i2c_port_, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "PCA9555 read register 0x%02X failed: %s", reg, esp_err_to_name(ret));
    }
    
    return ret;
}

bool ButtonControl::pca9555ReadPin(uint8_t pin)
{
    if (pin >= 16) {
        ESP_LOGE(TAG, "Invalid PCA9555 pin number: %d", pin);
        return false;
    }
    
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = (port == 0) ? PCA9555_INPUT_PORT_0 : PCA9555_INPUT_PORT_1;
    
    uint8_t value;
    esp_err_t ret = pca9555ReadRegister(reg, &value);
    if (ret != ESP_OK) {
        return false;
    }
    
    return (value & (1 << bit)) != 0;
}
