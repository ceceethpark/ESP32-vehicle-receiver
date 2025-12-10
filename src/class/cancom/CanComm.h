#ifndef CAN_COMM_H
#define CAN_COMM_H

#include <Arduino.h>

class CanComm {
public:
    CanComm();
    bool begin();
    void sendMotorCommand(uint8_t speed, uint8_t direction);
    void sendLiftCommand(uint8_t state);
    void sendCastCommand(uint8_t state);
    
private:
    static const uint8_t CAN_TX_PIN = 21;
    static const uint8_t CAN_RX_PIN = 22;
};

#endif
