#include "YbCar.h"

YbCar::YbCar() {
    currentSpeed = 0;
    currentDirection = 0;
    batteryLevel = 100;
    motorTemp = 25;
    motorCurrent = 0;
    fetTemp = 25;
    lastUpdateTime = 0;
}

bool YbCar::begin() {
    printf("YbCar init...\r\n");
    return true;
}

void YbCar::update() {
    unsigned long now = millis();
    if (now - lastUpdateTime < 100) return;
    lastUpdateTime = now;
    
    // Simulate sensor updates
    if (currentSpeed > 0) {
        motorTemp = 25 + (currentSpeed / 10);
        motorCurrent = currentSpeed * 10;
        fetTemp = 25 + (currentSpeed / 15);
    }
}

void YbCar::moveForward(uint8_t speed) {
    currentSpeed = speed;
    currentDirection = 1;
    printf("Forward: %d\r\n", speed);
}

void YbCar::moveBackward(uint8_t speed) {
    currentSpeed = speed;
    currentDirection = 2;
    printf("Backward: %d\r\n", speed);
}

void YbCar::turnLeft(uint8_t speed) {
    currentSpeed = speed;
    currentDirection = 3;
    printf("Left: %d\r\n", speed);
}

void YbCar::turnRight(uint8_t speed) {
    currentSpeed = speed;
    currentDirection = 4;
    printf("Right: %d\r\n", speed);
}

void YbCar::stop() {
    currentSpeed = 0;
    currentDirection = 0;
    motorCurrent = 0;
}

void YbCar::emergencyStop() {
    stop();
    printf("EMERGENCY STOP!\r\n");
}

void YbCar::liftUp() {
    printf("Lift UP\r\n");
}

void YbCar::liftDown() {
    printf("Lift DOWN\r\n");
}

void YbCar::liftStop() {
}

void YbCar::castUp() {
    printf("Cast UP\r\n");
}

void YbCar::castDown() {
    printf("Cast DOWN\r\n");
}

void YbCar::castStop() {
}

uint8_t YbCar::getSpeed() {
    return currentSpeed;
}

uint8_t YbCar::getDirection() {
    return currentDirection;
}

const char* YbCar::getDirectionString() {
    switch(currentDirection) {
        case 0: return "Stop";
        case 1: return "Forward";
        case 2: return "Backward";
        case 3: return "Left";
        case 4: return "Right";
        default: return "Unknown";
    }
}

uint8_t YbCar::getBatteryLevel() {
    return batteryLevel;
}

int16_t YbCar::getMotorTemp() {
    return motorTemp;
}

uint16_t YbCar::getMotorCurrent() {
    return motorCurrent;
}

int16_t YbCar::getFetTemp() {
    return fetTemp;
}
