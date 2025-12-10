#include "LocalButton.h"
#include "../ybcar/YbCar.h"

LocalButton::LocalButton() {
    pYbCar = nullptr;
    debounceDelay = 50;
    
    for (int i = 0; i < LOCAL_BUTTON_COUNT; i++) {
        buttons[i].isPressed = false;
        buttons[i].wasPressed = false;
        buttons[i].pressTime = 0;
        buttons[i].debounceTime = 0;
    }
    
    toggle.jenhujin = 0;
    toggle.lift_ud = 0;
    toggle.cast_ud = 0;
}

bool LocalButton::begin(YbCar* carPtr) {
    pYbCar = carPtr;
    
    printf("Local button init...\r\n");
    
    pinMode(PIN_FORWARD, INPUT_PULLUP);
    pinMode(PIN_BACKWARD, INPUT_PULLUP);
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_LIFT_UP, INPUT_PULLUP);
    pinMode(PIN_LIFT_DOWN, INPUT_PULLUP);
    pinMode(PIN_CAST_UP, INPUT_PULLUP);
    pinMode(PIN_CAST_DOWN, INPUT_PULLUP);
    pinMode(PIN_EMERGENCY, INPUT_PULLUP);
    pinMode(PIN_RUN, INPUT_PULLUP);
    
    pinMode(PIN_THROTTLE, INPUT);
    pinMode(PIN_SPEED_LIMIT, INPUT);
    
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    printf("Local button ready: 10 buttons + 2 ADC\r\n");
    return true;
}

void LocalButton::update() {
    scan();
}

uint8_t LocalButton::getPinByButton(LocalButtonID btn) {
    switch(btn) {
        case BTN_FORWARD: return PIN_FORWARD;
        case BTN_BACKWARD: return PIN_BACKWARD;
        case BTN_LEFT: return PIN_LEFT;
        case BTN_RIGHT: return PIN_RIGHT;
        case BTN_LIFT_UP: return PIN_LIFT_UP;
        case BTN_LIFT_DOWN: return PIN_LIFT_DOWN;
        case BTN_CAST_UP: return PIN_CAST_UP;
        case BTN_CAST_DOWN: return PIN_CAST_DOWN;
        case BTN_EMERGENCY: return PIN_EMERGENCY;
        case BTN_RUN: return PIN_RUN;
        default: return 0;
    }
}

void LocalButton::scan() {
    scanButtons();
    scanToggles();
}

void LocalButton::scanButtons() {
    unsigned long currentTime = millis();
    
    for (int i = 0; i < LOCAL_BUTTON_COUNT; i++) {
        uint8_t pin = getPinByButton((LocalButtonID)i);
        bool currentState = (digitalRead(pin) == LOW);
        
        if (currentState != buttons[i].isPressed) {
            if (currentTime - buttons[i].debounceTime > debounceDelay) {
                buttons[i].wasPressed = buttons[i].isPressed;
                buttons[i].isPressed = currentState;
                
                if (currentState) {
                    buttons[i].pressTime = currentTime;
                } else {
                    buttons[i].pressTime = 0;
                }
                
                buttons[i].debounceTime = currentTime;
            }
        }
    }
}

void LocalButton::scanToggles() {
    uint8_t jen_state = 0;
    if (digitalRead(PIN_FORWARD) == LOW) jen_state = 1;
    else if (digitalRead(PIN_BACKWARD) == LOW) jen_state = 2;
    toggle.jenhujin = jen_state;
    
    uint8_t lift_state = 0;
    if (digitalRead(PIN_LIFT_UP) == LOW) lift_state = 1;
    else if (digitalRead(PIN_LIFT_DOWN) == LOW) lift_state = 2;
    toggle.lift_ud = lift_state;
    
    uint8_t cast_state = 0;
    if (digitalRead(PIN_CAST_UP) == LOW) cast_state = 1;
    else if (digitalRead(PIN_CAST_DOWN) == LOW) cast_state = 2;
    toggle.cast_ud = cast_state;
}

bool LocalButton::isPressed(LocalButtonID btn) {
    if (btn >= LOCAL_BUTTON_COUNT) return false;
    return buttons[btn].isPressed;
}

bool LocalButton::wasJustPressed(LocalButtonID btn) {
    if (btn >= LOCAL_BUTTON_COUNT) return false;
    return buttons[btn].isPressed && !buttons[btn].wasPressed;
}

bool LocalButton::wasJustReleased(LocalButtonID btn) {
    if (btn >= LOCAL_BUTTON_COUNT) return false;
    return !buttons[btn].isPressed && buttons[btn].wasPressed;
}

uint8_t LocalButton::getJenhujinState() {
    return toggle.jenhujin;
}

uint8_t LocalButton::getLiftState() {
    return toggle.lift_ud;
}

uint8_t LocalButton::getCastState() {
    return toggle.cast_ud;
}

uint16_t LocalButton::getThrottle() {
    return analogRead(PIN_THROTTLE);
}

uint16_t LocalButton::getSpeedLimit() {
    return analogRead(PIN_SPEED_LIMIT);
}

void LocalButton::setHandler(YbCar* ybcar) {
    pYbCar = ybcar;
}

void LocalButton::setDebounceTime(unsigned long ms) {
    debounceDelay = ms;
}

void LocalButton::processEvents() {
    if (!pYbCar) return;
    
    if (wasJustPressed(BTN_EMERGENCY)) {
        pYbCar->emergencyStop();
        printf("Emergency stop!\r\n");
        return;
    }
    
    bool running = isPressed(BTN_RUN);
    if (!running) {
        pYbCar->stop();
        return;
    }
    
    uint8_t jen = getJenhujinState();
    uint16_t throttle = getThrottle();
    uint8_t speed = map(throttle, 0, 4095, 0, 100);
    
    if (jen == 1) {
        pYbCar->moveForward(speed);
    } else if (jen == 2) {
        pYbCar->moveBackward(speed);
    } else {
        if (isPressed(BTN_LEFT)) {
            pYbCar->turnLeft(30);
        } else if (isPressed(BTN_RIGHT)) {
            pYbCar->turnRight(30);
        } else {
            pYbCar->stop();
        }
    }
    
    uint8_t lift = getLiftState();
    if (lift == 1) {
        pYbCar->liftUp();
    } else if (lift == 2) {
        pYbCar->liftDown();
    } else {
        pYbCar->liftStop();
    }
    
    uint8_t cast = getCastState();
    if (cast == 1) {
        pYbCar->castUp();
    } else if (cast == 2) {
        pYbCar->castDown();
    } else {
        pYbCar->castStop();
    }
}

void LocalButton::handleButtonEvent(LocalButtonID btn) {
    // Individual button event handling (future implementation)
}
