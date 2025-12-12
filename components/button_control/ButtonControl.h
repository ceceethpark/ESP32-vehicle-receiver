#ifndef BUTTON_CONTROL_H
#define BUTTON_CONTROL_H

#include "driver/gpio.h"
#include "driver/i2c.h"
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
    
    // 원격 명령 콜백 (CAN/ROS 제어용)
    typedef void (*RemoteCommandCallback)(uint8_t command_id);
    
    // 생성자/소멸자
    ButtonControl();
    ~ButtonControl();
    
    // 초기화 (GPIO 핀 배열)
    esp_err_t begin(const gpio_num_t* button_pins, uint8_t num_buttons = BUTTON_COUNT);
    
    // I2C 초기화 (PCA9555 내장)
    esp_err_t beginI2C(i2c_port_t i2c_port, gpio_num_t sda_pin, gpio_num_t scl_pin, 
                      uint8_t i2c_addr, const uint8_t* button_pins, uint8_t num_buttons = 6);
    void end();
    
    // 통합 초기화 (콜백 포함)
    esp_err_t initialize(const gpio_num_t* button_pins, uint8_t num_buttons, 
                        ButtonCallback btn_callback, RemoteCommandCallback remote_callback);
    
    // I2C 통합 초기화 (PCA9555 내장)
    esp_err_t initializeI2C(i2c_port_t i2c_port, gpio_num_t sda_pin, gpio_num_t scl_pin,
                           uint8_t i2c_addr, const uint8_t* button_pins, uint8_t num_buttons,
                           ButtonCallback btn_callback, RemoteCommandCallback remote_callback);
    
    // 버튼 상태 확인
    bool isPressed(ButtonID btn);
    bool wasJustPressed(ButtonID btn);
    bool wasJustReleased(ButtonID btn);
    
    // 토글 상태
    ToggleState getToggleState() const { return toggle_state_; }
    
    // 콜백 설정
    void setButtonCallback(ButtonCallback callback);
    void setRemoteCommandCallback(RemoteCommandCallback callback);
    
    // 원격 명령 처리 (ESP-NOW 등에서 호출)
    void handleRemoteCommand(uint8_t command_id);
    
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
    
    // I2C 기반 (PCA9555 내장)
    bool use_i2c_;
    i2c_port_t i2c_port_;
    uint8_t i2c_addr_;
    bool i2c_initialized_by_this_;
    uint8_t pca_pins_[BUTTON_COUNT];  // PCA9555 포트 번호 (0-15)
    
    // 버튼 상태
    ButtonState button_states_[BUTTON_COUNT];
    ToggleState toggle_state_;
    
    // 콜백
    ButtonCallback button_callback_;
    RemoteCommandCallback remote_command_callback_;
    
    // 디바운스 설정
    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t LONG_PRESS_MS = 1000;
    
    // FreeRTOS 태스크
    TaskHandle_t scan_task_handle_;
    bool scan_task_running_;
    
    // 내부 함수
    void processButtonEvent(ButtonID id, ButtonEvent event);
    
    // PCA9555 I2C 통신 (내장)
    esp_err_t pca9555WriteRegister(uint8_t reg, uint8_t value);
    esp_err_t pca9555ReadRegister(uint8_t reg, uint8_t* value);
    bool pca9555ReadPin(uint8_t pin);
    
    // 정적 태스크 래퍼
    static void scanTaskWrapper(void* parameter);
    
    // 로그 태그
    static constexpr const char* TAG = "ButtonControl";
};

#endif // BUTTON_CONTROL_H
