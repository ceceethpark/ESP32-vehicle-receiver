#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "driver/gpio.h"
#include "esp_log.h"

class LedControl {
public:
    // LED 패턴
    enum Pattern {
        PATTERN_OFF,
        PATTERN_ON,
        PATTERN_BLINK_SLOW,
        PATTERN_BLINK_FAST,
        PATTERN_PULSE
    };
    
    // 생성자/소멸자
    LedControl(gpio_num_t led_pin = GPIO_NUM_2);
    ~LedControl();
    
    // 초기화
    esp_err_t begin();
    void end();
    
    // 기본 제어
    void on();
    void off();
    void toggle();
    void setBrightness(uint8_t brightness); // 0-255 (PWM)
    
    // 패턴
    void setPattern(Pattern pattern);
    void blink(uint8_t times, uint16_t delay_ms = 200);
    
    // 상태 표시
    void showSuccess();   // 빠른 3회 깜박임
    void showError();     // 느린 연속 깜박임
    void showWarning();   // 중간 속도 깜박임
    void showInfo();      // 짧은 1회 깜박임
    
    // 상태
    bool isOn() const { return is_on_; }
    Pattern getPattern() const { return current_pattern_; }
    
private:
    gpio_num_t led_pin_;
    bool is_on_;
    uint8_t brightness_;
    Pattern current_pattern_;
    
    // 블링크 헬퍼
    void blinkOnce(uint16_t on_ms, uint16_t off_ms);
    
    // 로그 태그
    static constexpr const char* TAG = "LedControl";
};

#endif // LED_CONTROL_H
