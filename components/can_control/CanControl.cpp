#include "CanControl.h"
#include <string.h>

CanControl::CanControl(gpio_num_t tx_pin, gpio_num_t rx_pin)
    : tx_pin_(tx_pin)
    , rx_pin_(rx_pin)
    , bitrate_(500000)
    , initialized_(false)
    , can_buf_idx_(0)
    , can_alive_timeout_(0)
    , response_callback_(nullptr)
    , status_callback_(nullptr)
    , lcd_update_callback_(nullptr)
    , rx_task_handle_(nullptr)
    , rx_task_running_(false)
{
    memset(can_rx_buf_, 0, sizeof(can_rx_buf_));
    memset(&vehicle_status_, 0, sizeof(vehicle_status_));
}

CanControl::~CanControl() {
    end();
}

esp_err_t CanControl::begin(uint32_t bitrate) {
    if (initialized_) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }
    
    bitrate_ = bitrate;
    
    // TWAI 일반 설정
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_pin_, rx_pin_, TWAI_MODE_NORMAL);
    g_config.tx_queue_len = 10;
    g_config.rx_queue_len = 10;
    
    // TWAI 타이밍 설정 (500 Kbps)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // TWAI 필터 설정 (STM32 tja1050 방식: 0x5B0~0x5BF 범위 수신)
    twai_filter_config_t f_config = {
        .acceptance_code = (CAN_RX_DATA_ID << 21),  // Standard ID in bits 28:18
        .acceptance_mask = ~((0x7F0) << 21),         // Mask for 0x5B0~0x5BF
        .single_filter = true
    };
    
    // TWAI 드라이버 설치
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // TWAI 시작
    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(ret));
        twai_driver_uninstall();
        return ret;
    }
    
    initialized_ = true;
    can_alive_timeout_ = CAN_ALIVE_TIMEOUT;
    ESP_LOGI(TAG, "CAN initialized (TX: GPIO%d, RX: GPIO%d, %d bps)", 
             tx_pin_, rx_pin_, bitrate_);
    return ESP_OK;
}

void CanControl::end() {
    if (!initialized_) return;
    
    stopRxTask();
    
    twai_stop();
    twai_driver_uninstall();
    
    initialized_ = false;
    ESP_LOGI(TAG, "CAN terminated");
}

esp_err_t CanControl::initialize(uint32_t bitrate, StatusCallback status_cb, LcdUpdateCallback lcd_cb,
                                 uint32_t stack_size, UBaseType_t priority)
{
    esp_err_t ret = begin(bitrate);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN begin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    setStatusCallback(status_cb);
    setLcdUpdateCallback(lcd_cb);
    
    ret = startRxTask(stack_size, priority);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN RX task start failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "CAN fully initialized (bitrate: %lu Kbps)", bitrate / 1000);
    return ESP_OK;
}

esp_err_t CanControl::sendMotorCommand(uint8_t speed, uint8_t direction) {
    uint8_t data[8] = {speed, direction, 0, 0, 0, 0, 0, 0};
    return sendMessage(CAN_TX_PUT_CMD, data, 8);
}

esp_err_t CanControl::sendLiftCommand(uint8_t state) {
    uint8_t data[8] = {0, 0, state, 0, 0, 0, 0, 0};
    return sendMessage(CAN_TX_PUT_CMD, data, 8);
}

esp_err_t CanControl::sendCasterCommand(uint8_t state) {
    uint8_t data[8] = {0, 0, 0, state, 0, 0, 0, 0};
    return sendMessage(CAN_TX_PUT_CMD, data, 8);
}

esp_err_t CanControl::sendGetConfig() {
    uint8_t data[8] = {0};
    return sendMessage(CAN_TX_GET_CONFIG, data, 8);
}

esp_err_t CanControl::sendSaveConfig() {
    uint8_t data[8] = {0};
    return sendMessage(CAN_TX_SAVE_CMD, data, 8);
}

esp_err_t CanControl::sendMessage(uint32_t id, const uint8_t* data, uint8_t len) {
    if (!initialized_) {
        ESP_LOGE(TAG, "Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (len > 8) {
        ESP_LOGE(TAG, "Data too long: %d bytes (max 8)", len);
        return ESP_ERR_INVALID_ARG;
    }
    
    twai_message_t message = {};
    message.identifier = id;
    message.data_length_code = len;
    message.extd = 0;  // Standard ID
    message.rtr = 0;   // Data frame
    memcpy(message.data, data, len);
    
    esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(1000));
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Sent CAN message: ID=0x%03lX, Len=%d", id, len);
    } else {
        ESP_LOGE(TAG, "Failed to send CAN message: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t CanControl::receiveMessage(twai_message_t& message, uint32_t timeout_ms) {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return twai_receive(&message, pdMS_TO_TICKS(timeout_ms));
}

void CanControl::setResponseCallback(ResponseCallback callback) {
    response_callback_ = callback;
}

void CanControl::setStatusCallback(StatusCallback callback) {
    status_callback_ = callback;
}

void CanControl::setLcdUpdateCallback(LcdUpdateCallback callback) {
    lcd_update_callback_ = callback;
}

esp_err_t CanControl::startRxTask(uint32_t stack_size, UBaseType_t priority) {
    if (rx_task_running_) {
        ESP_LOGW(TAG, "RX task already running");
        return ESP_OK;
    }
    
    BaseType_t ret = xTaskCreate(
        rxTaskWrapper,
        "can_rx",
        stack_size,
        this,
        priority,
        &rx_task_handle_
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create RX task");
        return ESP_FAIL;
    }
    
    rx_task_running_ = true;
    ESP_LOGI(TAG, "RX task started");
    return ESP_OK;
}

void CanControl::stopRxTask() {
    if (rx_task_running_ && rx_task_handle_) {
        rx_task_running_ = false;
        vTaskDelete(rx_task_handle_);
        rx_task_handle_ = nullptr;
        ESP_LOGI(TAG, "RX task stopped");
    }
}

void CanControl::rxTaskWrapper(void* parameter) {
    CanControl* self = static_cast<CanControl*>(parameter);
    
    while (self->rx_task_running_) {
        twai_message_t message;
        esp_err_t ret = self->receiveMessage(message, 100);
        
        if (ret == ESP_OK) {
            self->processReceivedMessage(message);
            self->can_alive_timeout_ = CAN_ALIVE_TIMEOUT;
        } else if (ret != ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "Error receiving CAN message: %s", esp_err_to_name(ret));
        }
        
        // Timeout 카운트다운
        if (self->can_alive_timeout_ > 0) {
            self->can_alive_timeout_--;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    vTaskDelete(nullptr);
}

void CanControl::processReceivedMessage(const twai_message_t& message) {
    uint32_t std_id = message.identifier;
    
    ESP_LOGD(TAG, "Received CAN message: ID=0x%03lX, Len=%d", 
             std_id, message.data_length_code);
    
    // STM32 tja1050 방식: 0x5B0~0x5B5 범위 데이터 처리
    if (std_id >= CAN_RX_DATA_ID && std_id <= (CAN_RX_DATA_ID + CAN_RX_BUF_SIZE - 1)) {
        uint8_t rx_idx = std_id - CAN_RX_DATA_ID;  // 0~5
        memcpy(can_rx_buf_[rx_idx], message.data, 8);
        can_buf_idx_ |= (1 << rx_idx);  // 비트 설정
        
        ESP_LOGD(TAG, "RX Buffer[%d] updated, buf_idx=0x%02X", rx_idx, can_buf_idx_);
        
        // 모든 버퍼가 준비되면 차량 상태 업데이트
        if (can_buf_idx_ == 0x3F) {  // 0b111111 = 모든 버퍼
            updateVehicleStatus();
            can_buf_idx_ = 0;  // 리셋
        }
    }
    // 응답 메시지 처리
    else if (std_id == CAN_RX_RESPONSE_ID) {
        ESP_LOGD(TAG, "Response message received");
        processResponseMessage(message.data);
    }
}

void CanControl::processResponseMessage(const uint8_t* data) {
    if (response_callback_) {
        response_callback_(data, 8);
    }
}

void CanControl::updateVehicleStatus() {
    // CAN RX 버퍼에서 차량 상태 데이터 추출
    // 버퍼 0: 전압 데이터
    vehicle_status_.volt_main = (can_rx_buf_[0][0] << 8) | can_rx_buf_[0][1];
    vehicle_status_.volt_dcdc = (can_rx_buf_[0][2] << 8) | can_rx_buf_[0][3];
    
    // 버퍼 1: 전류 데이터
    vehicle_status_.current_avg = (can_rx_buf_[1][0] << 8) | can_rx_buf_[1][1];
    vehicle_status_.consumption = (can_rx_buf_[1][2] << 8) | can_rx_buf_[1][3];
    
    // 버퍼 2: 온도 데이터
    vehicle_status_.motor_temp = (can_rx_buf_[2][0] << 8) | can_rx_buf_[2][1];
    vehicle_status_.fet_temp = (can_rx_buf_[2][2] << 8) | can_rx_buf_[2][3];
    
    // 버퍼 3: SOC 및 에러 코드
    vehicle_status_.soc = can_rx_buf_[3][0];
    vehicle_status_.error_code = can_rx_buf_[3][1];
    
    // 로그 출력
    ESP_LOGI(TAG, "CAN Status - VMain:%d DCDC:%d Curr:%d Cons:%d MotorT:%d FetT:%d SOC:%d Err:%d",
             vehicle_status_.volt_main, vehicle_status_.volt_dcdc, 
             vehicle_status_.current_avg, vehicle_status_.consumption,
             vehicle_status_.motor_temp, vehicle_status_.fet_temp, 
             vehicle_status_.soc, vehicle_status_.error_code);
    
    // StatusCallback 호출 (ROS 등)
    if (status_callback_) {
        status_callback_(vehicle_status_);
    }
    
    // LCD 업데이트 콜백 호출
    if (lcd_update_callback_) {
        lcd_update_callback_(vehicle_status_.volt_main, vehicle_status_.soc,
                            vehicle_status_.motor_temp, vehicle_status_.current_avg,
                            vehicle_status_.fet_temp);
    }
}
