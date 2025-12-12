#ifndef WIFI_CONTROL_H
#define WIFI_CONTROL_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"

class WifiControl {
public:
    // 생성자/소멸자
    WifiControl();
    ~WifiControl();
    
    // 초기화 및 종료
    esp_err_t begin(wifi_mode_t mode = WIFI_MODE_STA);
    esp_err_t initialize(wifi_mode_t mode = WIFI_MODE_STA);
    void end();
    
    // WiFi 모드 설정
    esp_err_t setMode(wifi_mode_t mode);
    wifi_mode_t getMode() const;
    
    // WiFi 시작/정지
    esp_err_t start();
    esp_err_t stop();
    
    // 상태 확인
    bool isInitialized() const { return initialized_; }
    bool isStarted() const { return started_; }
    
private:
    bool initialized_;
    bool started_;
    wifi_mode_t mode_;
    
    // 로그 태그
    static constexpr const char* TAG = "WifiControl";
};

#endif // WIFI_CONTROL_H
