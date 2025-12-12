#ifndef ESPNOW_CONTROL_H
#define ESPNOW_CONTROL_H

#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <cstring>

// 전방 선언
class RosBridge;
class CanControl;

class EspNowControl {
public:
    // ESP-NOW 데이터 구조
    struct ButtonData {
        uint8_t button_id;
        uint8_t button_state;
        uint32_t timestamp;
    };
    
    struct VehicleStatus {
        uint8_t speed;
        uint8_t direction;
        uint8_t battery_level;
        int16_t motor_temp;
        uint16_t motor_current;
        int16_t fet_temp;
    };
    
    struct RecvData {
        uint8_t mac[6];
        uint8_t data[250];
        int len;
    };
    
    // 콜백 타입
    typedef void (*RecvCallback)(const uint8_t* mac, const uint8_t* data, int len);
    typedef void (*SendCallback)(const uint8_t* mac, esp_now_send_status_t status);
    typedef void (*ButtonCommandCallback)(uint8_t button_id);
    typedef void (*CommandProcessCallback)(uint8_t command_id);  // ROS + CAN 명령 처리
    
    // 생성자/소멸자
    EspNowControl();
    ~EspNowControl();
    
    // 초기화 및 종료
    esp_err_t begin(uint8_t channel = 1);
    void end();
    
    // 통합 초기화 (콜백 포함)
    esp_err_t initialize(uint8_t channel, CommandProcessCallback cmd_callback, uint32_t stack_size = 4096, UBaseType_t priority = 5);
    
    // 데이터 송수신
    esp_err_t send(const uint8_t* mac, const uint8_t* data, size_t len);
    esp_err_t sendBroadcast(const uint8_t* data, size_t len);
    
    // 태스크 관리
    esp_err_t startRxTask(uint32_t stack_size = 4096, UBaseType_t priority = 5);
    void stopRxTask();
    
    // 콜백 설정
    void setRecvCallback(RecvCallback callback);
    void setSendCallback(SendCallback callback);
    void setButtonCommandCallback(ButtonCommandCallback callback);
    void setCommandProcessCallback(CommandProcessCallback callback);
    
    // ROS/CAN 컨트롤러 설정
    void setRosBridge(RosBridge* ros_bridge);
    void setCanControl(CanControl* can_control);
    
    // 상태 확인
    bool isInitialized() const { return initialized_; }
    bool isTaskRunning() const { return rx_task_handle_ != nullptr; }
    
    // 리모컨 데이터 처리
    bool parseButtonData(const uint8_t* data, int len, ButtonData* out_button);
    bool parseVehicleStatus(const uint8_t* data, int len, VehicleStatus* out_status);
    void processRemoteControl(const uint8_t* mac, const uint8_t* data, int len);
    
private:
    bool initialized_;
    uint8_t channel_;
    QueueHandle_t recv_queue_;
    TaskHandle_t rx_task_handle_;
    RecvCallback recv_callback_;
    SendCallback send_callback_;
    ButtonCommandCallback button_command_callback_;
    CommandProcessCallback command_process_callback_;
    
    // 외부 컴포넌트 포인터
    RosBridge* ros_bridge_;
    CanControl* can_control_;
    
    // ESP-NOW 콜백 (정적)
    static void recvCallbackStatic(const esp_now_recv_info_t* recv_info, const uint8_t* data, int len);
    static void sendCallbackStatic(const wifi_tx_info_t* tx_info, esp_now_send_status_t status);
    
    // 수신 태스크
    static void rxTaskWrapper(void* arg);
    void rxTask();
    
    // 싱글톤 인스턴스 (콜백용)
    static EspNowControl* instance_;
    
    // 로그 태그
    static constexpr const char* TAG = "EspNowControl";
    static constexpr size_t QUEUE_SIZE = 10;
};

#endif // ESPNOW_CONTROL_H
