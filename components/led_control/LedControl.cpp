#include "LedControl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

LedControl::LedControl(gpio_num_t led_pin)
    : led_pin_(led_pin)
    , is_on_(false)
    , brightness_(255)
    , current_pattern_(PATTERN_OFF)
{
}

LedControl::~LedControl() {
    end();
}

esp_err_t LedControl::begin() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << led_pin_);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LED GPIO %d: %s", 
                 led_pin_, esp_err_to_name(ret));
        return ret;
    }
    
    off(); // 초기 상태는 OFF
    ESP_LOGI(TAG, "LED initialized on GPIO %d", led_pin_);
    return ESP_OK;
}

void LedControl::end() {
    off();
    gpio_reset_pin(led_pin_);
}

void LedControl::on() {
    gpio_set_level(led_pin_, 1);
    is_on_ = true;
}

void LedControl::off() {
    gpio_set_level(led_pin_, 0);
    is_on_ = false;
}

void LedControl::toggle() {
    if (is_on_) {
        off();
    } else {
        on();
    }
}

void LedControl::setBrightness(uint8_t brightness) {
    brightness_ = brightness;
    // TODO: PWM 구현 (ledc 사용)
    if (brightness > 0) {
        on();
    } else {
        off();
    }
}

void LedControl::setPattern(Pattern pattern) {
    current_pattern_ = pattern;
    
    switch (pattern) {
        case PATTERN_OFF:
            off();
            break;
        case PATTERN_ON:
            on();
            break;
        case PATTERN_BLINK_SLOW:
            // TODO: Task로 구현
            break;
        case PATTERN_BLINK_FAST:
            // TODO: Task로 구현
            break;
        case PATTERN_PULSE:
            // TODO: Task로 구현
            break;
    }
}

void LedControl::blink(uint8_t times, uint16_t delay_ms) {
    for (uint8_t i = 0; i < times; i++) {
        blinkOnce(delay_ms, delay_ms);
    }
    off();
}

void LedControl::blinkOnce(uint16_t on_ms, uint16_t off_ms) {
    on();
    vTaskDelay(pdMS_TO_TICKS(on_ms));
    off();
    vTaskDelay(pdMS_TO_TICKS(off_ms));
}

void LedControl::showSuccess() {
    blink(3, 100);
    ESP_LOGI(TAG, "Show SUCCESS");
}

void LedControl::showError() {
    blink(5, 200);
    ESP_LOGE(TAG, "Show ERROR");
}

void LedControl::showWarning() {
    blink(3, 300);
    ESP_LOGW(TAG, "Show WARNING");
}

void LedControl::showInfo() {
    blink(1, 100);
    ESP_LOGI(TAG, "Show INFO");
}
