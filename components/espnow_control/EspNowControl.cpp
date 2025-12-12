#include "EspNowControl.h"

EspNowControl* EspNowControl::instance_ = nullptr;

EspNowControl::EspNowControl()
    : initialized_(false)
    , channel_(1)
    , recv_queue_(nullptr)
    , rx_task_handle_(nullptr)
    , recv_callback_(nullptr)
    , send_callback_(nullptr)
    , button_command_callback_(nullptr)
    , command_process_callback_(nullptr)
    , ros_bridge_(nullptr)
    , can_control_(nullptr)
{
    instance_ = this;
}

EspNowControl::~EspNowControl()
{
    end();
    instance_ = nullptr;
}

esp_err_t EspNowControl::begin(uint8_t channel)
{
    if (initialized_) {
        ESP_LOGW(TAG, "ESP-NOW already initialized");
        return ESP_OK;
    }

    channel_ = channel;
    
    // 큐 생성
    recv_queue_ = xQueueCreate(QUEUE_SIZE, sizeof(RecvData));
    if (recv_queue_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create receive queue");
        return ESP_FAIL;
    }
    
    // ESP-NOW 초기화
    esp_err_t ret = esp_now_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ESP-NOW: %s", esp_err_to_name(ret));
        vQueueDelete(recv_queue_);
        recv_queue_ = nullptr;
        return ret;
    }
    
    // 콜백 등록
    ret = esp_now_register_recv_cb(recvCallbackStatic);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register recv callback: %s", esp_err_to_name(ret));
        esp_now_deinit();
        vQueueDelete(recv_queue_);
        recv_queue_ = nullptr;
        return ret;
    }
    
    ret = esp_now_register_send_cb(sendCallbackStatic);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register send callback: %s", esp_err_to_name(ret));
        esp_now_deinit();
        vQueueDelete(recv_queue_);
        recv_queue_ = nullptr;
        return ret;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "ESP-NOW initialized on channel %d", channel_);
    return ESP_OK;
}

void EspNowControl::end()
{
    if (!initialized_) {
        return;
    }

    stopRxTask();
    esp_now_deinit();
    
    if (recv_queue_ != nullptr) {
        vQueueDelete(recv_queue_);
        recv_queue_ = nullptr;
    }
    
    initialized_ = false;
    ESP_LOGI(TAG, "ESP-NOW deinitialized");
}

esp_err_t EspNowControl::initialize(uint8_t channel, CommandProcessCallback cmd_callback, uint32_t stack_size, UBaseType_t priority)
{
    esp_err_t ret = begin(channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW begin failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    setCommandProcessCallback(cmd_callback);
    
    ret = startRxTask(stack_size, priority);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW RX task start failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "ESP-NOW fully initialized");
    return ESP_OK;
}

esp_err_t EspNowControl::send(const uint8_t* mac, const uint8_t* data, size_t len)
{
    if (!initialized_) {
        ESP_LOGE(TAG, "ESP-NOW not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return esp_now_send(mac, data, len);
}

esp_err_t EspNowControl::sendBroadcast(const uint8_t* data, size_t len)
{
    uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return send(broadcast_mac, data, len);
}

esp_err_t EspNowControl::startRxTask(uint32_t stack_size, UBaseType_t priority)
{
    if (!initialized_) {
        ESP_LOGE(TAG, "ESP-NOW not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (rx_task_handle_ != nullptr) {
        ESP_LOGW(TAG, "RX task already running");
        return ESP_OK;
    }

    BaseType_t ret = xTaskCreate(rxTaskWrapper, "espnow_rx_task", stack_size, this, priority, &rx_task_handle_);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create RX task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "ESP-NOW RX task started");
    return ESP_OK;
}

void EspNowControl::stopRxTask()
{
    if (rx_task_handle_ != nullptr) {
        vTaskDelete(rx_task_handle_);
        rx_task_handle_ = nullptr;
        ESP_LOGI(TAG, "ESP-NOW RX task stopped");
    }
}

void EspNowControl::setRecvCallback(RecvCallback callback)
{
    recv_callback_ = callback;
}

void EspNowControl::setSendCallback(SendCallback callback)
{
    send_callback_ = callback;
}

void EspNowControl::setButtonCommandCallback(ButtonCommandCallback callback)
{
    button_command_callback_ = callback;
}

void EspNowControl::setCommandProcessCallback(CommandProcessCallback callback)
{
    command_process_callback_ = callback;
}

void EspNowControl::setRosBridge(RosBridge* ros_bridge)
{
    ros_bridge_ = ros_bridge;
}

void EspNowControl::setCanControl(CanControl* can_control)
{
    can_control_ = can_control;
}

void EspNowControl::recvCallbackStatic(const esp_now_recv_info_t* recv_info, const uint8_t* data, int len)
{
    if (instance_ == nullptr || recv_info->src_addr == nullptr || data == nullptr || len <= 0) {
        return;
    }
    
    RecvData evt;
    std::memcpy(evt.mac, recv_info->src_addr, 6);
    evt.len = len > 250 ? 250 : len;
    std::memcpy(evt.data, data, evt.len);
    
    if (xQueueSend(instance_->recv_queue_, &evt, 0) != pdTRUE) {
        ESP_LOGW(instance_->TAG, "Receive queue full");
    }
}

void EspNowControl::sendCallbackStatic(const wifi_tx_info_t* tx_info, esp_now_send_status_t status)
{
    if (instance_ != nullptr && instance_->send_callback_ != nullptr) {
        // MAC 주소는 tx_info에서 얻을 수 없으므로 nullptr 전달
        instance_->send_callback_(nullptr, status);
    }
    
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGD(instance_->TAG, "ESP-NOW send success");
    } else {
        ESP_LOGW(instance_->TAG, "ESP-NOW send fail");
    }
}

void EspNowControl::rxTaskWrapper(void* arg)
{
    EspNowControl* instance = static_cast<EspNowControl*>(arg);
    instance->rxTask();
}

void EspNowControl::rxTask()
{
    RecvData evt;
    
    ESP_LOGI(TAG, "ESP-NOW RX task running");

    while (true) {
        if (xQueueReceive(recv_queue_, &evt, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Received ESP-NOW data: len=%d from %02X:%02X:%02X:%02X:%02X:%02X",
                    evt.len, evt.mac[0], evt.mac[1], evt.mac[2], 
                    evt.mac[3], evt.mac[4], evt.mac[5]);
            
            // 내부 처리
            processRemoteControl(evt.mac, evt.data, evt.len);
            
            // 사용자 콜백 호출
            if (recv_callback_ != nullptr) {
                recv_callback_(evt.mac, evt.data, evt.len);
            }
        }
    }
}

bool EspNowControl::parseButtonData(const uint8_t* data, int len, ButtonData* out_button)
{
    if (len < sizeof(ButtonData)) {
        ESP_LOGW(TAG, "Data too short for ButtonData: %d bytes", len);
        return false;
    }
    
    std::memcpy(out_button, data, sizeof(ButtonData));
    ESP_LOGI(TAG, "Button parsed - ID:%d State:%d Time:%lu", 
             out_button->button_id, out_button->button_state, out_button->timestamp);
    return true;
}

bool EspNowControl::parseVehicleStatus(const uint8_t* data, int len, VehicleStatus* out_status)
{
    if (len < sizeof(VehicleStatus)) {
        ESP_LOGW(TAG, "Data too short for VehicleStatus: %d bytes", len);
        return false;
    }
    
    std::memcpy(out_status, data, sizeof(VehicleStatus));
    ESP_LOGI(TAG, "Vehicle status - Speed:%d Dir:%d Batt:%d", 
             out_status->speed, out_status->direction, out_status->battery_level);
    return true;
}

void EspNowControl::processRemoteControl(const uint8_t* mac, const uint8_t* data, int len)
{
    ESP_LOGD(TAG, "Processing remote control data: %d bytes", len);
    
    // 데이터 길이에 따라 타입 판별
    if (len == sizeof(ButtonData)) {
        ButtonData button;
        if (parseButtonData(data, len, &button)) {
            ESP_LOGI(TAG, "Remote button: ID=%d, State=%d", button.button_id, button.button_state);
        }
    }
    else if (len == sizeof(VehicleStatus)) {
        VehicleStatus status;
        if (parseVehicleStatus(data, len, &status)) {
            ESP_LOGI(TAG, "Vehicle status: Speed=%d, Battery=%d%%", status.speed, status.battery_level);
        }
    }
    else if (len >= 1) {
        // Raw 데이터 처리 (단순 버튼 ID)
        uint8_t command_id = data[0];
        ESP_LOGI(TAG, "Processing remote command: %d", command_id);
        
        // ROS로 발행 (ros_bridge가 설정된 경우)
        if (ros_bridge_ != nullptr) {
            ros_bridge_->publish(command_id);
        }
        
        // CAN 명령 전송 (can_control이 설정된 경우)
        if (can_control_ != nullptr) {
            switch (command_id) {
                case 1: // 전진
                    can_control_->sendMotorCommand(50, 1);
                    break;
                case 2: // 후진
                    can_control_->sendMotorCommand(50, 2);
                    break;
                case 3: // 정지
                    can_control_->sendMotorCommand(0, 0);
                    break;
                case 4: // 리프트 업
                    can_control_->sendLiftCommand(1);
                    break;
                case 5: // 리프트 다운
                    can_control_->sendLiftCommand(2);
                    break;
                case 6: // 캐스터 회전
                    can_control_->sendCasterCommand(1);
                    break;
                default:
                    ESP_LOGW(TAG, "Unknown command ID: %d", command_id);
                    break;
            }
        }
        
        // 버튼 명령 콜백 호출 (레거시)
        if (button_command_callback_ != nullptr) {
            button_command_callback_(command_id);
        }
        
        // 명령 처리 콜백 호출 (추가 처리)
        if (command_process_callback_ != nullptr) {
            command_process_callback_(command_id);
        }
    }
    else {
        ESP_LOGW(TAG, "Unknown data format: len=%d", len);
    }
}
