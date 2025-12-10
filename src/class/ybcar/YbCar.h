#ifndef YBCAR_H
#define YBCAR_H

#include <Arduino.h>

class YbCar {
public:
    YbCar();
    bool begin();
    void update();
    
    // Motor control
    void moveForward(uint8_t speed);
    void moveBackward(uint8_t speed);
    void turnLeft(uint8_t speed);
    void turnRight(uint8_t speed);
    void stop();
    void emergencyStop();
    
    // Lift control
    void liftUp();
    void liftDown();
    void liftStop();
    
    // Cast control
    void castUp();
    void castDown();
    void castStop();
    
    // Status getters
    uint8_t getSpeed();
    uint8_t getDirection();
    const char* getDirectionString();
    uint8_t getBatteryLevel();
    int16_t getMotorTemp();
    uint16_t getMotorCurrent();
    int16_t getFetTemp();
    
private:
    uint8_t currentSpeed;
    uint8_t currentDirection;  // 0:stop, 1:forward, 2:backward, 3:left, 4:right
    uint8_t batteryLevel;
    int16_t motorTemp;
    uint16_t motorCurrent;
    int16_t fetTemp;
    
    unsigned long lastUpdateTime;
};

#endif
