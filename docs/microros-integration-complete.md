# micro-ROS 통합 완료 기록

**날짜**: 2025-12-11  
**프로젝트**: ESP32 Micro Hub  
**상태**: ✅ 빌드 성공, 테스트 대기

---

## 통합 완료 내역

### 1. 라이브러리 빌드 (WSL2)

**빌드 환경**:
- OS: WSL2 Ubuntu-22.04
- ESP-IDF: v5.2
- 빌드 시간: 58분 12초

**빌드 결과**:
```
Summary: 75 packages finished [58min 12s]

생성된 파일:
- libmicroros.a: 18MB (병합된 정적 라이브러리)
- include/: micro-ROS 헤더 파일
```

**위치**:
```
components/micro_ros_espidf_component/
├── libmicroros.a          # 18MB
├── include/               # 헤더 파일
└── micro_ros_src/         # 빌드 소스 (보존)
```

---

### 2. ESP32 프로젝트 통합

**main.c 추가 내용**:

#### micro-ROS 헤더
```c
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>
#include <uxr/client/transport.h>
```

#### CUSTOM_TRANSPORT 구현
```c
bool uart_transport_open(struct uxrCustomTransport* transport);
bool uart_transport_close(struct uxrCustomTransport* transport);
size_t uart_transport_write(struct uxrCustomTransport*, const uint8_t*, size_t, uint8_t*);
size_t uart_transport_read(struct uxrCustomTransport*, uint8_t*, size_t, int, uint8_t*);
```

#### micro-ROS 초기화
```c
static esp_err_t microros_init(void)
{
    rmw_uros_set_custom_transport(
        true, NULL,
        uart_transport_open,
        uart_transport_close,
        uart_transport_write,
        uart_transport_read
    );
    
    allocator = rcl_get_default_allocator();
    rclc_support_init(&support, 0, NULL, &allocator);
    rclc_node_init_default(&node, "esp32_micro_hub", "", &support);
    rclc_publisher_init_default(&publisher, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "espnow_button");
    
    return ESP_OK;
}
```

#### ESP-NOW → ROS2 퍼블리시
```c
if (ros_enabled) {
    msg.data = (button_id << 8) | button_state;
    rcl_ret_t ret = rcl_publish(&publisher, &msg, NULL);
    if (ret == RCL_RET_OK) {
        ESP_LOGI(TAG, "Published to ROS: %d", msg.data);
    }
}
```

---

### 3. 빌드 설정 수정

**CMakeLists.txt 수정**:
```cmake
# libmicroros.a가 이미 존재하면 재빌드 건너뛰기
if(NOT EXISTS "${COMPONENT_DIR}/libmicroros.a")
    message(STATUS "Building libmicroros.a...")
    execute_process(...)
else()
    message(STATUS "Using existing libmicroros.a (built by WSL2)")
endif()
```

**이유**: Windows에서 매번 WSL2 빌드 시도 방지

---

## 최종 빌드 결과

### 메모리 사용량

| 항목 | micro-ROS 전 | micro-ROS 후 | 증가량 |
|------|-------------|-------------|--------|
| **RAM** | 32,736 bytes (10.0%) | 43,504 bytes (13.3%) | +10,768 bytes (+33%) |
| **Flash** | 758,501 bytes (72.3%) | 804,477 bytes (76.7%) | +45,976 bytes (+6%) |

### 여유 공간

- **RAM**: 284,176 bytes (86.7%) 여유
- **Flash**: 244,099 bytes (23.3%) 여유

✅ **LiDAR, Encoder, LCD, CAN 추가 가능**

---

## 통신 구조

```
ESP-NOW 리모컨
    ↓ (WiFi 2.4GHz)
ESP32 (ESP-NOW 수신)
    ↓ (큐 전달)
espnow_task
    ↓ (ROS 발행)
micro-ROS Publisher (/espnow_button)
    ↓ (UART1 @ 921600 bps, CUSTOM_TRANSPORT)
micro-ROS Agent (PC/라즈베리파이)
    ↓ (DDS)
ROS2 Nodes
```

---

## 테스트 절차

### 1. ESP32 펌웨어 플래시

```bash
# VS Code에서
Ctrl+Shift+P → "ESP-IDF: Flash your project"

# 또는 PlatformIO
pio run -t upload
```

### 2. micro-ROS Agent 실행

**라즈베리파이 5 (Ubuntu 24.04 + ROS2 Jazzy)**:
```bash
sudo apt install ros-jazzy-micro-ros-agent
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 921600
```

**PC (Ubuntu 22.04 + ROS2 Humble)**:
```bash
sudo apt install ros-humble-micro-ros-agent
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 921600
```

**Windows (WSL2 + ROS2)**:
```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyS3 -b 921600
```

### 3. 시리얼 모니터 확인

```bash
pio device monitor -b 115200
```

**예상 로그**:
```
ESP32 Micro Hub starting...
WiFi initialized for ESP-NOW
ESP-NOW initialized
ROS Serial initialized on UART1 @ 921600 bps
micro-ROS initialized
ESP-NOW task started
ROS task started
LCD UI task started
ESP32 Micro Hub initialized
Waiting for micro-ROS agent...
micro-ROS agent connected! ← 성공!
```

### 4. ROS2 토픽 확인

```bash
# 토픽 리스트
ros2 topic list
# /espnow_button
# /parameter_events
# /rosout

# 토픽 데이터 모니터링
ros2 topic echo /espnow_button

# ESP-NOW 리모컨 버튼 누르면 출력:
# data: 257  (button_id=1, state=1)
```

---

## 하드웨어 연결

### UART1 (micro-ROS Agent 통신)

| ESP32 | USB-Serial |
|-------|-----------|
| GPIO17 (TXD) | RXD |
| GPIO16 (RXD) | TXD |
| GND | GND |

**설정**: 921600 bps, 8N1, No flow control

### ESP-NOW (무선 리모컨)

- WiFi 채널: 1
- 자동 페어링 (브로드캐스트 수신)

---

## 문제 해결

### Agent 연결 실패

**증상**:
```
Waiting for micro-ROS agent...
Agent not available, retrying...
```

**해결**:
1. USB-Serial 연결 확인
2. Agent 실행 확인: `ps aux | grep micro_ros_agent`
3. 포트 확인: `ls -l /dev/ttyUSB*`
4. 권한 추가: `sudo usermod -a -G dialout $USER`
5. 재로그인

### 토픽 발행 실패

**증상**:
```
Failed to publish: -1
```

**원인**: Agent와 연결되지 않음

**해결**:
```c
// ros_task에서 연결 대기 확인
while (rmw_uros_ping_agent(1000, 1) != RMW_RET_OK) {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
```

---

## 향후 작업

### 1. 센서 추가

**LiDAR (UART2)**:
```c
#define LIDAR_UART UART_NUM_2
#define LIDAR_TXD 25
#define LIDAR_RXD 26
```

**Encoder (PCNT)**:
```c
#define ENCODER_A_GPIO 32
#define ENCODER_B_GPIO 33
```

### 2. LCD 추가 (ILI9488, SPI)

```c
#define LCD_MOSI 23
#define LCD_MISO 19
#define LCD_SCK 18
#define LCD_CS 5
#define LCD_DC 4
#define LCD_RST 2
```

### 3. CAN 추가

```c
#define CAN_TX_GPIO 21
#define CAN_RX_GPIO 22
```

### 4. 메시지 타입 확장

현재: `std_msgs/msg/Int32`  
→ 변경: 커스텀 메시지 (`ButtonEvent.msg`)

```
uint8 button_id
uint8 button_state
uint64 timestamp
```

---

## 참고 자료

- [micro-ROS 공식 문서](https://micro.ros.org/)
- [ESP-IDF API 레퍼런스](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/index.html)
- [ROS2 Humble 문서](https://docs.ros.org/en/humble/index.html)
- [micro-ROS CUSTOM_TRANSPORT](https://github.com/micro-ROS/micro_ros_espidf_component#custom-transports)

---

## 성공 기준

✅ **빌드 성공**: RAM 13.3%, Flash 76.7%  
⏳ **Agent 연결**: 대기 중  
⏳ **토픽 발행**: 대기 중  
⏳ **ESP-NOW 수신**: 대기 중  

**다음**: ESP32 플래시 → Agent 연결 → 통신 테스트
