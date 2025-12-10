#include "RecvESPNow.h"

DataRecvCallback RecvESPNow::userCallback = nullptr;

RecvESPNow::RecvESPNow() {
}

bool RecvESPNow::begin() {
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        printf("ESP-NOW init failed\r\n");
        return false;
    }
    
    esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
    
    printf("ESP-NOW initialized\r\n");
    return true;
}

void RecvESPNow::setReceiveCallback(DataRecvCallback callback) {
    userCallback = callback;
}

bool RecvESPNow::send(const uint8_t* mac, const uint8_t* data, size_t len) {
    esp_err_t result = esp_now_send(mac, data, len);
    return (result == ESP_OK);
}

void RecvESPNow::onDataRecv(const uint8_t* mac, const uint8_t* data, int len) {
    if (userCallback) {
        userCallback(mac, data, len);
    }
}
