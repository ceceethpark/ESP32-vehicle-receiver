#ifndef LOCAL_LED_H
#define LOCAL_LED_H

#include <Arduino.h>

class LocalLED {
public:
    LocalLED();
    bool begin();
    
    void showSuccess();
    void showError();
    void showWarning();
    void showInfo();
    void off();
    
private:
    static const uint8_t LED_PIN = 2;
    void blink(uint8_t times, uint16_t delayMs);
};

#endif
