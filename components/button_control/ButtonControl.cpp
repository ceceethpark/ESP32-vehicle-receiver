#include "ButtonControl.h"
#include <string.h>

ButtonControl::ButtonControl()
    : num_buttons_(0)
    , button_callback_(nullptr)
    , scan_task_handle_(nullptr)
    , scan_task_running_(false)
{
    memset(button_pins_, 0, sizeof(button_pins_));
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
        bool current_state = gpio_get_level(button_pins_[i]) == 0; // Active LOW
        
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

void ButtonControl::scanTaskWrapper(void* parameter) {
    ButtonControl* self = static_cast<ButtonControl*>(parameter);
    
    while (self->scan_task_running_) {
        self->scanButtons();
        self->updateToggleState();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz 스캔
    }
    
    vTaskDelete(NULL);
}
