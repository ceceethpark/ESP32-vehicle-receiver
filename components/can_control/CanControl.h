#ifndef CAN_CONTROL_H
#define CAN_CONTROL_H

#include "driver/twai.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

class CanControl {
public:
    // CAN 메시지 ID (STM32 tja1050 기반)
    static constexpr uint32_t CAN_TX_GET_CONFIG = 0x0700;
    static constexpr uint32_t CAN_TX_PUT_CMD = 0x0701;
    static constexpr uint32_t CAN_TX_SAVE_CMD = 0x0708;
    static constexpr uint32_t CAN_RX_DATA_ID = 0x05B0;    // 0x5B0~0x5B5 범위
    static constexpr uint32_t CAN_RX_RESPONSE_ID = 0x05B8;
    
    // CAN 버퍼 크기
    static constexpr uint8_t CAN_RX_BUF_SIZE = 6;  // 0x5B0~0x5B5
    static constexpr uint8_t CAN_ALIVE_TIMEOUT = 100;  // 100ms
    
    // 차량 제어 데이터 구조체
    struct VehicleControlData {
        uint8_t speed;
        uint8_t direction;
        uint8_t lift_state;
        uint8_t caster_state;
        uint8_t brake_state;
        uint8_t mode;
    };
    
    // 차량 상태 데이터 구조체
    struct VehicleStatusData {
        int16_t volt_main;      // 메인 전압
        int16_t volt_dcdc;      // DCDC 전압
        int16_t current_avg;    // 평균 전류
        int16_t consumption;    // 소비전력
        int16_t motor_temp;     // 모터 온도
        int16_t fet_temp;       // FET 온도
        uint8_t soc;            // 배터리 잔량(%)
        uint8_t error_code;
    };
    
    // 응답 데이터 콜백
    typedef void (*ResponseCallback)(const uint8_t* data, uint8_t len);
    typedef void (*StatusCallback)(const VehicleStatusData& status);
    typedef void (*LcdUpdateCallback)(int16_t volt_main, uint8_t soc, int16_t motor_temp, int16_t current_avg, int16_t fet_temp);
    
    // 생성자
    CanControl(gpio_num_t tx_pin, gpio_num_t rx_pin);
    ~CanControl();
    
    // 초기화/종료
    esp_err_t begin(uint32_t bitrate = 500000);
    void end();
    
    // 통합 초기화 (콜백 포함)
    esp_err_t initialize(uint32_t bitrate, StatusCallback status_cb, LcdUpdateCallback lcd_cb, 
                        uint32_t stack_size = 4096, UBaseType_t priority = 4);
    
    // 명령 전송 (STM32 tja1050 기반)
    esp_err_t sendMotorCommand(uint8_t speed, uint8_t direction);
    esp_err_t sendLiftCommand(uint8_t state);
    esp_err_t sendCasterCommand(uint8_t state);
    esp_err_t sendGetConfig();
    esp_err_t sendSaveConfig();
    
    // 일반 CAN 메시지 전송
    esp_err_t sendMessage(uint32_t id, const uint8_t* data, uint8_t len);
    
    // 메시지 수신
    esp_err_t receiveMessage(twai_message_t& message, uint32_t timeout_ms = 100);
    
    // 콜백 설정
    void setResponseCallback(ResponseCallback callback);
    void setStatusCallback(StatusCallback callback);
    void setLcdUpdateCallback(LcdUpdateCallback callback);
    
    // 수신 태스크 관리
    esp_err_t startRxTask(uint32_t stack_size = 4096, UBaseType_t priority = 4);
    void stopRxTask();
    
    // 상태 확인
    VehicleStatusData getVehicleStatus() const { return vehicle_status_; }
    bool isInitialized() const { return initialized_; }
    uint8_t getCanAliveTimeout() const { return can_alive_timeout_; }
    
    // CAN RX 버퍼 접근
    const uint8_t* getRxBuffer(uint8_t index) const {
        if (index < CAN_RX_BUF_SIZE) return can_rx_buf_[index];
        return nullptr;
    }
    bool isRxBufferReady(uint8_t index) const {
        return (can_buf_idx_ & (1 << index)) != 0;
    }
    
private:
    // GPIO 핀
    gpio_num_t tx_pin_;
    gpio_num_t rx_pin_;
    
    // TWAI 설정
    uint32_t bitrate_;
    bool initialized_;
    
    // CAN RX 버퍼 (STM32 tja1050 방식)
    uint8_t can_rx_buf_[CAN_RX_BUF_SIZE][8];
    uint8_t can_buf_idx_;  // 비트마스크 (어떤 버퍼에 데이터가 있는지)
    uint8_t can_alive_timeout_;
    
    // 차량 상태
    VehicleStatusData vehicle_status_;
    
    // 콜백
    ResponseCallback response_callback_;
    StatusCallback status_callback_;
    LcdUpdateCallback lcd_update_callback_;
    
    // FreeRTOS 태스크
    TaskHandle_t rx_task_handle_;
    bool rx_task_running_;
    
    // 내부 함수
    void processReceivedMessage(const twai_message_t& message);
    void processResponseMessage(const uint8_t* data);
    void updateVehicleStatus();
    
    // 정적 태스크 래퍼
    static void rxTaskWrapper(void* parameter);
    
    // 로그 태그
    static constexpr const char* TAG = "CanControl";
};

#endif // CAN_CONTROL_H
