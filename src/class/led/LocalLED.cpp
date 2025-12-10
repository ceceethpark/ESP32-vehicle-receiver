#include "LocalLED.h"

LocalLED::LocalLED() {
}

bool LocalLED::begin() {
    pinMode(LED_PIN, OUTPUT);
    off();
    return true;
}

void LocalLED::showSuccess() {
    blink(2, 100);
}

void LocalLED::showError() {
    blink(5, 200);
}

void LocalLED::showWarning() {
    blink(3, 150);
}

void LocalLED::showInfo() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void LocalLED::off() {
    digitalWrite(LED_PIN, LOW);
}

void LocalLED::blink(uint8_t times, uint16_t delayMs) {
    for (uint8_t i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(delayMs);
        digitalWrite(LED_PIN, LOW);
        delay(delayMs);
    }
}
