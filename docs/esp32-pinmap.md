# ESP32 Micro Hub - Pin Map

## 하드웨어 정보
- **MCU**: ESP32-WROOM-32
- **Clock**: 240 MHz (Dual Core)
- **RAM**: 320 KB
- **Flash**: 4 MB
- **회로도 기반**: 실제 하드웨어 회로도 검증 완료

## GPIO 핀 할당표

| 기능 | GPIO | 방향 | 설명 | 비고 |
|------|------|------|------|------|
| **UART0 (Debug)** |
| Debug TX | GPIO 1 | OUT | USB Serial TXD0 | 프로그래밍/디버그 |
| Debug RX | GPIO 3 | IN | USB Serial RXD0 | 프로그래밍/디버그 |
| **CAN Bus (TWAI)** |
| CAN TX | GPIO 16 | OUT | CAN 송신 | 회로도 IO27 |
| CAN RX | GPIO 17 | IN | CAN 수신 | 회로도 IO26 |
| **micro-ROS (UART1)** |
| ROS TX | GPIO 23 | OUT | UART1 TXD | 921600 bps, 헤더 |
| ROS RX | GPIO 22 | IN | UART1 RXD | 921600 bps, 헤더 |
| **I2C (IMU)** |
| I2C SDA | GPIO 13 | I/O | I2C Data | pSDA (회로도) |
| I2C SCL | GPIO 2 | OUT | I2C Clock | pSCL (회로도) |
| IMU INT | GPIO 15 | IN | IMU 인터럽트 | IMU_INT (회로도) |
| **ST7789 LCD (SPI2)** |
| LCD MOSI | GPIO 26 | OUT | SPI Data | 40 MHz |
| LCD MISO | GPIO 12 | IN | SPI Data In | 회로도 확인 |
| LCD SCLK | GPIO 19 | OUT | SPI Clock | 40 MHz |
| LCD CS | GPIO 25 | OUT | Chip Select | Active Low |
| LCD DC | GPIO 35 | OUT | Data/Command | Input Only |
| LCD RST | GPIO 0 | OUT | Reset | Boot 핀 주의 |
| **LED** |
| LED1 | GPIO 27 | OUT | LED 1 | |
| LED2 | GPIO 14 | OUT | LED 2 | |
| **PCA9555 I2C GPIO Expander** |
| PCA INT | GPIO - | IN | 인터럽트 | pCA_INT |
| **PCA9555 Input (IOI_x) - 10개** |
| IOI_0 | PCA:0 | IN | SELECT 버튼 | KEY 커넥터 |
| IOI_1 | PCA:1 | IN | DOWN 버튼 | KEY 커넥터 |
| IOI_2 | PCA:2 | IN | RIGHT 버튼 | KEY 커넥터 |
| IOI_3 | PCA:3 | IN | LEFT 버튼 | KEY 커넥터 |
| IOI_4 | PCA:4 | IN | UP 버튼 | KEY 커넥터 |
| IOI_5 | PCA:5 | IN | POWER 버튼 | pI_PWR_N |
| IOI_6 | PCA:6 | IN | 입력 6 | 확장 가능 |
| IOI_7 | PCA:7 | IN | 입력 7 | 확장 가능 |
| IOI_8 | PCA:8 | IN | 입력 8 | 확장 가능 |
| IOI_9 | PCA:9 | IN | 입력 9 | 확장 가능 |
| **PCA9555 Output (IOO_x) - 6개** |
| IOO_0 | PCA:10 | OUT | 출력 0 | 제어 가능 |
| IOO_1 | PCA:11 | OUT | 출력 1 | 제어 가능 |
| IOO_2 | PCA:12 | OUT | 출력 2 | 제어 가능 |
| IOO_3 | PCA:13 | OUT | 출력 3 | 제어 가능 |
| IOO_4 | PCA:14 | OUT | 출력 4 | 제어 가능 |
| IOO_5 | PCA:15 | OUT | 출력 5 | 제어 가능 |
| **ESP32 Direct Buttons** |
| BTN Emergency | GPIO 34 | IN | 비상정지 | Input Only |
| BTN Run | GPIO 35 | IN | 실행/정지 | Input Only |

## 핀 제약사항

### Input Only 핀 (Pull-up/Pull-down 불가)
- GPIO 34, 35, 36, 37, 38, 39

### 사용 불가 핀 (Flash 연결)
- GPIO 6, 7, 8, 9, 10, 11

### 부팅 관련 핀
- GPIO 0: Boot mode (LOW = Download mode)
- GPIO 2: Boot mode selection
- GPIO 15: JTAG 사용 시 주의

### ADC2 핀 (WiFi 사용 시 ADC2 불가)
- GPIO 0, 2, 4, 12-15, 25-27

## 통신 프로토콜

### CAN Bus (TWAI)
- **속도**: 500 Kbps
- **트랜시버**: TJA1050
- **용도**: 차량 제어 명령 및 상태 수신

### UART1 (micro-ROS)
- **속도**: 921600 bps
- **프로토콜**: micro-ROS CUSTOM_TRANSPORT
- **용도**: ROS2 통신 (Agent와 연결)

### SPI2 (LCD)
- **속도**: 40 MHz
- **Mode**: SPI Mode 0
- **LCD**: ST7789 240x320
- **용도**: UI 표시 (한글 지원)

### ESP-NOW (WiFi)
- **채널**: 1
- **Mode**: Station + ESP-NOW
- **용도**: 무선 리모컨 수신

## 전원 핀

| 핀 | 전압 | 설명 |
|----|------|------|
| 3.3V | 3.3V | 출력 전류 최대 500mA |
| 5V | 5V | USB 전원 (최대 500mA) |
| GND | 0V | 공통 접지 |
| EN | - | Reset (LOW = Reset) |

## 회로 연결도 (회로도 기반)

```
ESP32-WROOM-32 (실제 회로도)
┌─────────────────────┐
│  GPIO  1 (TXD0) ───●── USB Serial (Debug)
│  GPIO  3 (RXD0) ───●── USB Serial (Debug)
│                     │
│  GPIO 16 ──────────●── CAN TX
│  GPIO 17 ──────────●── CAN RX
│                     │
│  GPIO 23 ──────────●── UART TX (micro-ROS)
│  GPIO 22 ──────────●── UART RX (micro-ROS)
│                     │
│  GPIO 13 ──────────●── I2C SDA (pSDA)
│  GPIO  2 ──────────●── I2C SCL (pSCL)
│  GPIO 15 ──────────●── IMU INT
│                     │
│  GPIO 26 ──────────●── LCD MOSI
│  GPIO 12 ──────────●── LCD MISO
│  GPIO 19 ──────────●── LCD SCLK
│  GPIO 25 ──────────●── LCD CS
│  GPIO 35 ──────────●── LCD DC
│  GPIO  0 ──────────●── LCD RST
│                     │
│  GPIO 27 ──────────●── LED1
│  GPIO 14 ──────────●── LED2
│                     │
│  GPIO 34 ──────────●── BTN Emergency (Input Only)
│  GPIO 35 ──────────●── BTN Run (Input Only)
│                     │
│       ┌─────────────────────┐
│       │   PCA9555 (I2C)     │
│       │  Addr: 0x20         │
│       ├─────────────────────┤
│       │ IOI_0: SELECT       │
│       │ IOI_1: DOWN         │
│       │ IOI_2: RIGHT        │
│       │ IOI_3: LEFT         │
│       │ IOI_4: UP           │
│       │ IOI_5: POWER        │
│       │ IOI_6-9: (확장)     │
│       │ IOO_0-5: Outputs    │
│       └─────────────────────┘
│                     │
└─────────────────────┘
```

## ✅ PCA9555 I2C GPIO Expander

### 하드웨어 구성
- **칩**: PCA9555 (16-bit I2C GPIO expander)
- **I2C 주소**: 0x20 (기본값, A0-A2 핀으로 변경 가능)
- **인터럽트**: pCA_INT 핀 (GPIO 연결 가능)
- **전원**: VCC3.3V
- **I2C 버스**: ESP32 GPIO21(SDA), GPIO22(SCL)

### 포트 구성
**입력 포트 (IOI_0 ~ IOI_9)**: 10개 입력
- KEY 커넥터 연결:
  * IOI_0: SELECT 버튼
  * IOI_1: DOWN 버튼
  * IOI_2: RIGHT 버튼
  * IOI_3: LEFT 버튼
  * IOI_4: UP 버튼
  * IOI_5: pI_PWR_N (POWER 버튼)
  * IOI_6~9: 확장 가능 입력 (4개)
- PISUGAR: pI_PWR_P, pI_PWR_N, pI_CTYPE
- CHARGER: pI_CTYPE

**출력 포트 (IOO_0 ~ IOO_5)**: 6개 출력
- 6개 출력 제어 가능
- 용도: LED, 릴레이, 기타 출력 장치

### 장점
✅ GPIO 핀 충돌 완전 해결
✅ ESP32 GPIO 절약 (I2C 2개 핀으로 16개 GPIO 확장)
✅ 인터럽트 기반 버튼 감지 가능
✅ 추가 입출력 확장 용이
✅ 전원 관리 기능 통합 (PISUGAR, CHARGER)

## 태스크 우선순위

| 태스크 | 우선순위 | 스택 크기 | 주기 | 설명 |
|--------|----------|-----------|------|------|
| micro-ROS Task | 5 | 10240 | 100ms | ROS2 Pub/Sub |
| ESP-NOW RX Task | 5 | 4096 | Event | 리모컨 수신 |
| CAN RX Task | 4 | 4096 | Event | 차량 상태 수신 |
| UI Task | 3 | 8192 | Button:100ms<br>LCD:1000ms | 버튼/LCD 업데이트 |

## 참고사항

1. **GPIO 1, 3**: UART0 (TXD0/RXD0) - USB 시리얼 디버그 전용, 코드에서 사용 금지
2. **GPIO 34-39**: Input only, 내부 pull-up/pull-down 없음
3. **GPIO 0**: 부팅 시 LOW = Download mode, 사용 가능하지만 부팅 시 주의
4. **GPIO 2**: 부팅 시 LOW로 pull되므로 외부 회로 주의
5. **GPIO 15**: 부팅 시 HIGH 필요
6. **GPIO 21/22**: I2C (SDA/SCL) - PCA9555와 IMU 센서 공유
7. **WiFi 사용 시**: ADC2 (GPIO 0,2,4,12-15,25-27) 사용 불가
8. **PCA9555**: I2C 주소 0x20, 16개 GPIO 제공 (버튼/출력 제어)

## 설정 파일

모든 핀 정의는 `main/include/config.h`에서 관리됩니다.

```c
// CAN Bus (회로도 기준)
#define CAN_TX_PIN              GPIO_NUM_16    // IO16
#define CAN_RX_PIN              GPIO_NUM_17    // IO17

// I2C (IMU 센서 + PCA9555)
#define I2C_SDA_PIN             GPIO_NUM_13    // pSDA
#define I2C_SCL_PIN             GPIO_NUM_2     // pSCL
#define IMU_INT_PIN             GPIO_NUM_15    // IMU_INT

// micro-ROS UART
#define ROS_TX_PIN              GPIO_NUM_23
#define ROS_RX_PIN              GPIO_NUM_22

// ST7789 LCD
#define LCD_MOSI_PIN            GPIO_NUM_26
#define LCD_MISO_PIN            GPIO_NUM_12
#define LCD_SCLK_PIN            GPIO_NUM_19
#define LCD_CS_PIN              GPIO_NUM_25
#define LCD_DC_PIN              GPIO_NUM_35
#define LCD_RST_PIN             GPIO_NUM_0

// LED
#define LED1_PIN                GPIO_NUM_27
#define LED2_PIN                GPIO_NUM_14

// PCA9555 I2C GPIO Expander (10 inputs + 6 outputs)
#define PCA9555_I2C_ADDR        0x20

// PCA9555 Input Pins (IOI_0 ~ IOI_9) - 10개 입력
#define PCA_IOI_0    0   // SELECT
#define PCA_IOI_1    1   // DOWN
#define PCA_IOI_2    2   // RIGHT
#define PCA_IOI_3    3   // LEFT
#define PCA_IOI_4    4   // UP
#define PCA_IOI_5    5   // POWER (pI_PWR_N)
#define PCA_IOI_6    6   // 확장 가능
#define PCA_IOI_7    7   // 확장 가능
#define PCA_IOI_8    8   // 확장 가능
#define PCA_IOI_9    9   // 확장 가능

// PCA9555 Output Pins (IOO_0 ~ IOO_5) - 6개 출력
#define PCA_IOO_0    10
#define PCA_IOO_1    11
#define PCA_IOO_2    12
#define PCA_IOO_3    13
#define PCA_IOO_4    14
#define PCA_IOO_5    15

// Button mappings
#define BTN_SELECT              PCA_IOI_0  // 0
#define BTN_DOWN                PCA_IOI_1  // 1
#define BTN_RIGHT               PCA_IOI_2  // 2
#define BTN_LEFT                PCA_IOI_3  // 3
#define BTN_UP                  PCA_IOI_4  // 4
#define BTN_POWER               PCA_IOI_5  // 5
```

## 업데이트 이력

- 2025-12-11: 초기 핀맵 작성
- 2025-12-11: 회로도 기반으로 핀맵 검증 및 수정
  * CAN 핀 변경: GPIO21/22 → GPIO27/26
  * I2C 핀 추가: GPIO21(SDA)/GPIO22(SCL)
  * PCA9555 I2C GPIO Expander 통합
  * 버튼을 PCA9555로 이동하여 GPIO 충돌 해결
  * 회로도 기반 LED, 전원 관리 기능 문서화
- 2025-12-11: PCA9555 포트 구성 수정
  * 입력 포트: IOI_0 ~ IOI_9 (10개 입력)
  * 출력 포트: IOO_0 ~ IOO_5 (6개 출력)
  * 확장 가능 입력 4개 추가 (IOI_6~9)
- 2025-12-11: I2C 및 LCD 핀 회로도 기반 재검증
  * I2C: GPIO13(SDA), GPIO2(SCL), GPIO15(IMU_INT)
  * LCD: GPIO26(MOSI), GPIO12(MISO), GPIO19(SCLK), GPIO25(CS), GPIO35(DC), GPIO0(RST)
  * LED: GPIO27(LED1), GPIO14(LED2)
