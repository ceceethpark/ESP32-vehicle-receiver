#ifndef BUTTON_CONTROL_H
#define BUTTON_CONTROL_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

class ButtonControl {
public:
    // 버튼 ID (PCA9555 기반)
    enum ButtonID {
        BUTTON_SELECT = 0,
        BUTTON_DOWN,
        BUTTON_RIGHT,
        BUTTON_LEFT_DIR,
        BUTTON_UP,
        BUTTON_POWER,
        BUTTON_EMERGENCY,
        BUTTON_RUN,
        BUTTON_COUNT
    };
    
    // 버튼 이벤트
    enum ButtonEvent {
        EVENT_PRESSED,
        EVENT_RELEASED,
        EVENT_LONG_PRESS
    };
    
    // 버튼 상태
    struct ButtonState {
        bool isPressed;
        bool wasPressed;
        uint32_t pressTime;
        uint32_t debounceTime;
    };
    
    // 토글 상태
    struct ToggleState {
        uint8_t direction;  // 0:neutral, 1:forward, 2:backward
        uint8_t lift;       // 0:neutral, 1:up, 2:down
        uint8_t caster;     // 0:neutral, 1:up, 2:down
    };
    
    // 버튼 이벤트 콜백
    typedef void (*ButtonCallback)(ButtonID id, ButtonEvent event);
    
    // 생성자/소멸자
    ButtonControl();
    ~ButtonControl();
    
    // 초기화 (GPIO 핀 배열)
    esp_err_t begin(const gpio_num_t* button_pins, uint8_t num_buttons = BUTTON_COUNT);
    void end();
    
    // 버튼 상태 확인
    bool isPressed(ButtonID btn);
    bool wasJustPressed(ButtonID btn);
    bool wasJustReleased(ButtonID btn);
    
    // 토글 상태
    ToggleState getToggleState() const { return toggle_state_; }
    
    // 콜백 설정
    void setButtonCallback(ButtonCallback callback);
    
    // 스캔 태스크 관리
    esp_err_t startScanTask(uint32_t stack_size = 4096, UBaseType_t priority = 5);
    void stopScanTask();
    
    // 수동 스캔 (외부 태스크에서 호출 가능)
    void scanButtons();
    void updateToggleState();
    
private:
    // GPIO 핀
    gpio_num_t button_pins_[BUTTON_COUNT];
    uint8_t num_buttons_;
    
    // 버튼 상태
    ButtonState button_states_[BUTTON_COUNT];
    ToggleState toggle_state_;
    
    // 콜백
    ButtonCallback button_callback_;
    
    // 디바운스 설정
    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t LONG_PRESS_MS = 1000;
    
    // FreeRTOS 태스크
    TaskHandle_t scan_task_handle_;
    bool scan_task_running_;
    
    // 내부 함수
    void processButtonEvent(ButtonID id, ButtonEvent event);
    
    // 정적 태스크 래퍼
    static void scanTaskWrapper(void* parameter);
    
    // 로그 태그
    static constexpr const char* TAG = "ButtonControl";
};

#endif // BUTTON_CONTROL_H
