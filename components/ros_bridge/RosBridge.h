#ifndef ROS_BRIDGE_H
#define ROS_BRIDGE_H

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>
#include <uxr/client/transport.h>
#include "driver/uart.h"
#include "esp_log.h"

class RosBridge {
public:
    // 생성자/소멸자
    RosBridge(uart_port_t uart_port = UART_NUM_1, 
              int tx_pin = 17, 
              int rx_pin = 16, 
              int baud_rate = 921600);
    ~RosBridge();
    
    // 초기화 및 종료
    esp_err_t begin(const char* node_name = "esp32_micro_hub");
    void end();
    
    // 통합 초기화 (퍼블리셔 생성 포함)
    esp_err_t initialize(const char* node_name, const char* topic_name);
    
    // 퍼블리셔 생성
    esp_err_t createPublisher(const char* topic_name);
    
    // 메시지 발행
    esp_err_t publish(int32_t data);
    
    // Agent 연결 확인
    bool pingAgent(int timeout_ms = 1000, int attempts = 1);
    bool waitForAgent(int timeout_ms = 30000);
    
    // 상태 확인
    bool isInitialized() const { return initialized_; }
    bool isConnected() const { return connected_; }
    
private:
    // UART 설정
    uart_port_t uart_port_;
    int tx_pin_;
    int rx_pin_;
    int baud_rate_;
    
    // 상태
    bool initialized_;
    bool connected_;
    
    // micro-ROS 객체
    rcl_allocator_t allocator_;
    rclc_support_t support_;
    rcl_node_t node_;
    rcl_publisher_t publisher_;
    std_msgs__msg__Int32 msg_;
    
    // CUSTOM_TRANSPORT 함수들 (정적)
    static bool transportOpen(struct uxrCustomTransport* transport);
    static bool transportClose(struct uxrCustomTransport* transport);
    static size_t transportWrite(struct uxrCustomTransport* transport, 
                                  const uint8_t* buf, size_t len, uint8_t* err);
    static size_t transportRead(struct uxrCustomTransport* transport, 
                                 uint8_t* buf, size_t len, int timeout, uint8_t* err);
    
    // UART 초기화
    esp_err_t initUart();
    
    // 싱글톤 인스턴스 (transport 콜백용)
    static RosBridge* instance_;
    
    // 로그 태그
    static constexpr const char* TAG = "RosBridge";
};

#endif // ROS_BRIDGE_H
