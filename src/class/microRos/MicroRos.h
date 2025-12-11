#ifndef MICROROS_H
#define MICROROS_H

#include <Arduino.h>

// micro-ROS 라이브러리 사용 시 활성화
#define USE_MICRO_ROS 1  // 0: 비활성화, 1: 활성화

#if USE_MICRO_ROS
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>
#include <std_msgs/msg/int32.h>
#endif

class MicroRos {
public:
    MicroRos();
    ~MicroRos();
    
    // 초기화 및 설정
    bool begin(const char* node_name = "esp32_micro_hub");
    void end();
    
    // 통신 설정
    bool setupWiFiTransport(const char* agent_ip, uint16_t agent_port = 8888);
    bool setupSerialTransport(HardwareSerial& serial = Serial, uint32_t baud_rate = 115200);
    
    // 연결 관리
    bool connect();
    void disconnect();
    bool isConnected();
    
    // 메시지 발행
    bool publishString(const char* topic, const char* message);
    bool publishInt(const char* topic, int32_t value);
    bool publishRemoteControl(uint8_t buttonId, uint8_t buttonState);
    
    // 주기적 업데이트 (loop에서 호출)
    void spin();
    void spinOnce();
    
    // Task 관리
    bool startTask(uint32_t stack_size = 8192, UBaseType_t priority = 5);
    void stopTask();
    bool isTaskRunning();
    
    // 상태 확인
    void printStatus();
    const char* getNodeName();
    
private:
    // FreeRTOS Task
    TaskHandle_t task_handle_;
    bool task_running_;
    static void taskFunction(void* parameter);
    
#if USE_MICRO_ROS
    // micro-ROS 객체
    rcl_allocator_t allocator_;
    rclc_support_t support_;
    rcl_node_t node_;
    rclc_executor_t executor_;
    
    // Publisher
    rcl_publisher_t string_publisher_;
    rcl_publisher_t int_publisher_;
    
    // 메시지
    std_msgs__msg__String string_msg_;
    std_msgs__msg__Int32 int_msg_;
    
    // 내부 함수
    bool createPublisher(rcl_publisher_t* publisher, const char* topic_name, 
                        const rosidl_message_type_support_t* type_support);
#endif
    
    // 상태
    bool initialized_;
    bool connected_;
    char node_name_[64];
    void cleanup();
    bool checkConnection();
};

#endif // MICROROS_H
