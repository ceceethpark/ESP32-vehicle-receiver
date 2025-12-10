#ifndef RECV_ESP_NOW_H
#define RECV_ESP_NOW_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Data structures
typedef struct struct_message {
    uint8_t buttonId;
    uint8_t buttonState;
    uint32_t timestamp;
} struct_message;

typedef struct struct_vehicle_status {
    uint8_t speed;
    uint8_t direction;
    uint8_t batteryLevel;
    int16_t motorTemp;
    uint16_t motorCurrent;
    int16_t fetTemp;
} struct_vehicle_status;

typedef void (*DataRecvCallback)(const uint8_t* mac, const uint8_t* data, int len);

class RecvESPNow {
public:
    RecvESPNow();
    bool begin();
    void setReceiveCallback(DataRecvCallback callback);
    bool send(const uint8_t* mac, const uint8_t* data, size_t len);
    
private:
    static DataRecvCallback userCallback;
    static void onDataRecv(const uint8_t* mac, const uint8_t* data, int len);
};

#endif
