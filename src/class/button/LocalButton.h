#ifndef LOCAL_BUTTON_H
#define LOCAL_BUTTON_H

#include <Arduino.h>

class YbCar;

#define LOCAL_BUTTON_COUNT 10

enum LocalButtonID {
    BTN_FORWARD = 0,
    BTN_BACKWARD,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_LIFT_UP,
    BTN_LIFT_DOWN,
    BTN_CAST_UP,
    BTN_CAST_DOWN,
    BTN_EMERGENCY,
    BTN_RUN
};

struct ButtonState {
    bool isPressed;
    bool wasPressed;
    unsigned long pressTime;
    unsigned long debounceTime;
};

struct ToggleState {
    uint8_t jenhujin;  // 0:neutral, 1:forward, 2:backward
    uint8_t lift_ud;   // 0:neutral, 1:up, 2:down
    uint8_t cast_ud;   // 0:neutral, 1:up, 2:down
};

class LocalButton {
public:
    LocalButton();
    bool begin(YbCar* carPtr = nullptr);
    
    void update();
    void scan();
    void scanButtons();
    void scanToggles();
    
    bool isPressed(LocalButtonID btn);
    bool wasJustPressed(LocalButtonID btn);
    bool wasJustReleased(LocalButtonID btn);
    
    uint8_t getJenhujinState();
    uint8_t getLiftState();
    uint8_t getCastState();
    
    uint16_t getThrottle();
    uint16_t getSpeedLimit();
    
    void setHandler(YbCar* ybcar);
    void setDebounceTime(unsigned long ms);
    
    void processEvents();
    void handleButtonEvent(LocalButtonID btn);
    
private:
    YbCar* pYbCar;
    ButtonState buttons[LOCAL_BUTTON_COUNT];
    ToggleState toggle;
    unsigned long debounceDelay;
    
    uint8_t getPinByButton(LocalButtonID btn);
    
    // GPIO pins
    static const uint8_t PIN_FORWARD = 32;
    static const uint8_t PIN_BACKWARD = 33;
    static const uint8_t PIN_LEFT = 25;
    static const uint8_t PIN_RIGHT = 26;
    static const uint8_t PIN_LIFT_UP = 27;
    static const uint8_t PIN_LIFT_DOWN = 14;
    static const uint8_t PIN_CAST_UP = 12;
    static const uint8_t PIN_CAST_DOWN = 13;
    static const uint8_t PIN_EMERGENCY = 15;
    static const uint8_t PIN_RUN = 4;
    
    // ADC pins
    static const uint8_t PIN_THROTTLE = 34;
    static const uint8_t PIN_SPEED_LIMIT = 35;
};

#endif
