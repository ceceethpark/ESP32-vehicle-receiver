# ESP32 리모컨 프로젝트 - 업데이트 노트

## 2024-12-12 업데이트 (최신)

### 🏗️ 아키텍처 단순화 및 통합 초기화

#### 1. PCA9555 I2C 드라이버 통합
**변경 전**: 
- 독립된 `pca9555_driver` 컴포넌트 (500+ 줄)
- 2단계 초기화 (드라이버 → ButtonControl)
- main.cpp에서 `Pca9555Driver` 객체 관리

**변경 후**:
- PCA9555 I2C 코드를 `ButtonControl`에 직접 통합 (~80 줄)
- 1단계 초기화: `button_control.initializeI2C()`
- main.cpp 전역 객체 9개 → 8개로 축소

```cpp
// Before: 2-step initialization
static Pca9555Driver pca9555(PCA9555_I2C_ADDR);
pca9555.begin(I2C_NUM_0, GPIO_21, GPIO_22, 400000);
button_control.initializeI2C(&pca9555, pca_buttons, 6, callback, nullptr);

// After: 1-step initialization
button_control.initializeI2C(
    I2C_NUM_0, GPIO_21, GPIO_22, 0x20,
    pca_buttons, 6, callback, nullptr
);
```

**이점**:
- 컴포넌트 개수 감소 (11 → 10)
- 의존성 단순화 (ButtonControl only requires `driver`)
- 사용자 친화적 API (한 번의 호출로 완료)
- 코드 캡슐화 (I2C 세부사항 ButtonControl 내부로 은닉)

#### 2. WifiControl 컴포넌트 추가
- WiFi 초기화 로직을 main.cpp에서 분리
- ESP-NOW 필수 의존성 관리 개선
- `wifi_control.initialize(WIFI_MODE_STA)` 한 번의 호출로 완료

#### 3. TaskManager 컴포넌트 추가
- FreeRTOS Task 관리 통합
- UI Task와 ROS Task 내장 함수 제공
- Task 통계 및 모니터링 기능

```cpp
task_manager.createUiTask(&lcd_control, LCD_UPDATE_INTERVAL_MS, 
                          LCD_UI_TASK_STACK_SIZE, LCD_UI_TASK_PRIORITY);
task_manager.createRosTask(&ros_bridge, &lcd_control, 
                           ROS_TASK_STACK_SIZE, ROS_TASK_PRIORITY);
```

#### 4. 통합 initialize() 메서드 추가
모든 컴포넌트에 콜백 및 자동 Task 시작을 포함한 `initialize()` 메서드 추가:

- `RosBridge::initialize(node_name, topic_name)` - Publisher 생성 포함
- `LcdControl::initialize(initial_mode, connection_status)` - 초기 화면 표시
- `LedControl::initialize()` - 성공 LED 표시
- `CanControl::initialize(bitrate, status_cb, lcd_cb, ...)` - RX Task 시작 포함
- `EspNowControl::initialize(channel, cmd_callback, ...)` - RX Task 시작 포함
- `ButtonControl::initializeI2C(...)` - PCA9555 + Scan Task 시작

#### 5. 듀얼 버튼 시스템 완성
**로컬 버튼 (PCA9555 I2C)**:
- 포트: IOI_0~IOI_5 (6개)
- ROS 토픽: 100~105번
- Active LOW with internal pullup
- ButtonCallback으로 이벤트 처리

**원격 버튼 (ESP-NOW)**:
- 명령: 1~6번 (전진/후진/정지/리프트/캐스터 등)
- ROS 토픽: 1~6번
- EspNowControl 내부에서 CAN 명령 자동 전송

#### 6. main.cpp 구조 개선
**3단계 초기화 패턴**:
```cpp
// Phase 1: 기본 인프라
wifi_control.initialize();
led_control.initialize();
lcd_control.initialize("BOOTING", false);

// Phase 2: 통신 컴포넌트 (의존성 순서 고려)
ros_bridge.initialize("esp32_micro_hub", "espnow_button");
button_control.initializeI2C(...);  // PCA9555 통합
can_control.initialize(500000, status_cb, lcd_cb, ...);

// Phase 3: ESP-NOW (의존성 주입)
espnow_control.begin(1);
espnow_control.setRosBridge(&ros_bridge);
espnow_control.setCanControl(&can_control);
espnow_control.startRxTask(...);

// Task 시작
task_manager.createUiTask(...);
task_manager.createRosTask(...);
```

#### 7. 코드 메트릭스
- **main.cpp**: 340줄 → 230줄 (32% 감소)
- **전역 객체**: 9개 → 8개
- **컴포넌트**: 11개 → 10개
- **ButtonControl**: I2C 통신 코드 ~80줄 추가 (총 ~400줄)
- **삭제된 파일**: 
  * `components/pca9555_driver/` (전체 디렉토리)
  * `pca9555_driver/Pca9555Driver.h/cpp` (~500줄)
  * `pca9555_driver/CMakeLists.txt`

#### 8. 콜백 아키텍처 개선
**CAN 듀얼 콜백**:
- `StatusCallback`: ROS 발행
- `LcdUpdateCallback`: LCD 실시간 업데이트

**ESP-NOW 내부 처리**:
- ROS 발행 자동화
- CAN 명령 자동 전송
- ButtonCommandCallback (레거시)
- CommandProcessCallback (추가 처리)

#### 9. 에러 처리 통일
- **Critical**: `ESP_ERROR_CHECK()` 사용 (NVS, WiFi)
- **Non-critical**: `ESP_LOGW()` + continue (LCD, ROS, CAN)
- 시스템 안정성 향상 (일부 컴포넌트 실패해도 계속 동작)

---

## 2024-12-11 업데이트

### 🚗 CAN Control 시스템 완전 재작성 (STM32 tja1050 기반)

#### 1. CanControl 클래스 완전 재구현
- **기반**: STM32 tja1050 클래스 (git commit bca4c952)
- **드라이버**: ESP32 TWAI (Controller Area Network)
- **프로토콜**: 기존 차량 제어 시스템과 100% 호환

##### CAN 메시지 ID 정의
```cpp
// TX (ESP32 → Vehicle)
#define CAN_TX_GET_CONFIG  0x0700  // 설정 조회
#define CAN_TX_PUT_CMD     0x0701  // 명령 전송
#define CAN_TX_SAVE_CMD    0x0708  // 설정 저장

// RX (Vehicle → ESP32)
#define CAN_RX_DATA_ID     0x05B0  // 데이터 버퍼 0~5 (0x5B0~0x5B5)
#define CAN_RX_RESPONSE_ID 0x05B8  // 응답
```

##### 6-Buffer 수신 전략
```cpp
uint8_t can_rx_buf_[6][8];  // 6개 버퍼 × 8바이트
uint8_t can_buf_idx_;       // 비트마스크 (0x3F = 모두 수신)
uint8_t can_alive_timeout_; // 100ms 타임아웃

// Buffer 0: volt_main, volt_dcdc
// Buffer 1: current_avg, consumption
// Buffer 2: motor_temp, fet_temp
// Buffer 3: soc, error_code
// Buffer 4-5: 예약
```

##### TWAI 필터 설정
```cpp
// 0x5B0~0x5BF 범위만 수용
acceptance_code = (0x05B0 << 21);
acceptance_mask = ~((0x7F0) << 21);
```

#### 2. 데이터 구조체
```cpp
struct VehicleControlData {
    uint8_t speed;        // 모터 속도 (0~255)
    uint8_t direction;    // 방향 (0: 정지, 1: 전진, 2: 후진)
    uint8_t lift_state;   // 리프트 상태
    uint8_t caster_state; // 캐스터 상태
};

struct VehicleStatusData {
    int16_t volt_main;     // 메인 배터리 전압 (0.1V 단위)
    int16_t volt_dcdc;     // DCDC 전압 (0.1V 단위)
    int16_t current_avg;   // 평균 전류 (0.1A 단위)
    int16_t consumption;   // 소비 전력 (W)
    int16_t motor_temp;    // 모터 온도 (0.1°C)
    int16_t fet_temp;      // FET 온도 (0.1°C)
    uint8_t soc;          // 배터리 잔량 (%)
    uint8_t error_code;   // 에러 코드
};
```

#### 3. 명령 메서드
```cpp
bool sendMotorCommand(uint8_t speed, uint8_t direction);
bool sendLiftCommand(uint8_t state);
bool sendCasterCommand(uint8_t state);
bool sendGetConfig();
bool sendSaveConfig();
```

#### 4. 콜백 시스템
```cpp
// 응답 수신 (ID 0x5B8)
using ResponseCallback = std::function<void(const uint8_t* data, size_t len)>;

// 차량 상태 수신 완료 (6개 버퍼 모두 도착)
using StatusCallback = std::function<void(const VehicleStatusData& status)>;
```

#### 5. 타임아웃 메커니즘
```cpp
// RX Task (10ms 주기)
void rxTaskWrapper() {
    while (rx_task_running_) {
        if (can_alive_timeout_ > 0) {
            can_alive_timeout_--;
            if (can_alive_timeout_ == 0) {
                ESP_LOGW(TAG, "CAN timeout");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

#### 6. ButtonControl Enum 충돌 해결
```cpp
// config.h 매크로와 충돌 방지
enum ButtonId {
    BUTTON_SELECT = 0,      // (구 BTN_FORWARD)
    BUTTON_DOWN,            // (구 BTN_BACKWARD)
    BUTTON_RIGHT,           // (구 BTN_RIGHT)
    BUTTON_LEFT_DIR,        // (구 BTN_LEFT)
    BUTTON_UP,              // 신규
    BUTTON_POWER,           // 신규
    BUTTON_EMERGENCY,       // 신규
    BUTTON_RUN,             // 신규
    BUTTON_COUNT
};
```

#### 7. main.cpp 콜백 업데이트
```cpp
// 기존: VehicleStatus { speed, direction, battery_level, ... }
// 변경: VehicleStatusData { volt_main, volt_dcdc, soc, current_avg, ... }

void onCanStatus(const CanControl::VehicleStatusData& status) {
    ESP_LOGI(TAG, "VMain:%d DCDC:%d Curr:%d SOC:%d MotorT:%d FetT:%d",
             status.volt_main, status.volt_dcdc, status.current_avg,
             status.soc, status.motor_temp, status.fet_temp);
}
```

#### 8. 기술 사양
- **CAN Bitrate**: 500 Kbps
- **핀**: GPIO16 (TX), GPIO17 (RX)
- **필터**: 0x5B0~0x5BF 수용
- **Task Stack**: TX 2048, RX 4096 bytes
- **Task Priority**: TX=5, RX=5

#### 9. 변경된 파일
- ✅ `components/can_control/CanControl.h` - 완전 재작성
- ✅ `components/can_control/CanControl.cpp` - 완전 재작성
- ✅ `components/button_control/ButtonControl.h` - Enum 수정
- ✅ `components/button_control/ButtonControl.cpp` - 업데이트
- ✅ `main/main.cpp` - 콜백 시그니처 변경

#### 10. 빌드 상태
- **CanControl**: ✅ 컴파일 성공
- **ButtonControl**: ✅ 컴파일 성공
- **main.cpp**: ✅ 컴파일 성공
- **전체 빌드**: 🔄 진행 중

#### 11. 다음 단계
1. 전체 프로젝트 빌드 완료 확인
2. 실제 차량과 CAN 통신 테스트
3. 6-buffer 수신 동작 검증
4. Timeout 메커니즘 테스트
5. PCA9555 I2C 드라이버 구현

---

## 2025-12-11 업데이트

### ✨ micro-ROS 통합 완료

#### 1. WSL2에서 libmicroros.a 빌드 성공
- **환경**: WSL2 Ubuntu-22.04 + ESP-IDF v5.2
- **빌드 시간**: 58분 12초
- **패키지**: 75개 완료 (micro_ros_dev 60개 + micro_ros_src 75개)
- **결과**: libmicroros.a (18MB) + include/ (헤더 파일)

#### 2. 주요 문제 해결
- **VERSION/SOVERSION CMake 오류**: Micro-XRCE-DDS-Client CMakeLists.txt 321-324줄 삭제
- **POSIX transport 오류**: UDP/SERIAL 비활성화, CUSTOM_TRANSPORT 활성화
- **자동 재빌드 방지**: CMakeLists.txt에 libmicroros.a 존재 확인 로직 추가

#### 3. ESP32 프로젝트 통합
- **CUSTOM_TRANSPORT 구현**: UART1 (GPIO16/17) @ 921600 bps
  - `uart_transport_open/close/write/read` 4개 함수
- **micro-ROS 초기화**: node, publisher 생성
- **ESP-NOW → ROS2 발행**: `/espnow_button` 토픽 (std_msgs/Int32)
- **Agent 연결 대기**: `rmw_uros_ping_agent()` 루프

#### 4. 최종 빌드 결과
- **RAM**: 43,504 bytes (13.3% / 327,680 bytes) - micro-ROS 전: 10.0%
- **Flash**: 804,477 bytes (76.7% / 1,048,576 bytes) - micro-ROS 전: 72.3%
- **증가량**: RAM +10,768 bytes, Flash +45,976 bytes
- **상태**: ✅ 빌드 성공, 메모리 충분

#### 5. 문서 추가
- `docs/micro-ros-build-issues.md`: 빌드 문제 해결 상세 기록
- `docs/microros-integration-complete.md`: 통합 완료 및 테스트 가이드

#### 6. 다음 단계
- ESP32 펌웨어 플래시
- micro-ROS Agent 연결 테스트 (라즈베리파이 5 또는 PC)
- ESP-NOW 리모컨 데이터 → ROS2 토픽 확인
- LiDAR, Encoder, LCD, CAN 추가

---

## 2025-12-10 업데이트

### ✨ Serial 출력 표준화 및 UI 재설계

#### 1. Serial 출력 완전 표준화
- **모든 Serial.print/println → printf 변환** (총 88개)
- **CR+LF 줄바꿈** 통일 (`\r\n`)
- **TXD0 포트 출력** (GPIO1)
- **Float 값 정상 출력** 지원

**변환된 파일:**
- RemoteESPNow.cpp (8개)
- RemoteLCD.cpp (1개)
- RemoteButton.cpp (7개)
- RemoteLED.cpp (1개)
- main.cpp (15개)
- YbCar.cpp (3개)
- YbCarDoctor.cpp (31개)
- RemoteCANCom.cpp (22개)

#### 2. UI 레이아웃 대폭 재설계
- **타이틀**: "YCB AI 전동차"
- **온도/전류 재배치**: 방향 표시 바로 아래 (220-255px)
  - 모터/FET 온도 좌우 배치
  - 전류 표시 별도 줄
- **배터리 게이지 이동**: 272px, 220x18 크기
- **연결 상태**: LED 원형 인디케이터 추가 (300px)
- **RSSI**: "dBm" 단위 표시

#### 3. SVG UI 디자인 파일 추가
- **파일**: `docs/ui-design-240x320.svg`
- **특징**: 
  - Figma, Adobe XD, Illustrator 호환
  - 레이어 구조화
  - 색상 팔레트 정의
  - 240x320 세로형 레이아웃

### 📊 빌드 결과
```
RAM:   13.6% (44,472 bytes)
Flash: 61.2% (801,665 bytes)
Status: SUCCESS
```

---

## 2025-12-09 업데이트

### ✨ 주요 변경사항

#### 1. 클래스 기반 구조로 전환
- LCD 기능을 `RemoteLCD` 클래스로 분리
- 키보드 기능을 `RemoteKeyboard` 클래스로 분리
- 직관적인 폴더 구조 및 네이밍

#### 2. LCD 디스플레이 추가
- **모델**: SZH-EK096 (ST7789, 320x240)
- **통신**: SPI 4선식
- **기능**:
  - 메인 화면 UI
  - 버튼 상태 표시
  - 연결 상태 표시
  - 배터리 레벨 표시
  - 진행바, 텍스트, 버튼 그리기

#### 3. 고급 키보드 기능
- 디바운싱 (50ms)
- 롱프레스 감지 (1초)
- 더블클릭 지원 (300ms)
- 이벤트 큐 시스템
- 설정 가능한 타이밍

### 📁 새로운 파일 구조

```
esp32_core_remocon/
├── include/class/
│   ├── lcd/RemoteLCD.h
│   └── keyboard/RemoteKeyboard.h
├── src/class/
│   ├── lcd/RemoteLCD.cpp
│   └── keyboard/RemoteKeyboard.cpp
└── docs/
    └── class-structure.md
```

### 🔧 설정 변경

#### platformio.ini
```ini
lib_deps = 
    adafruit/Adafruit GFX Library @ ^1.11.9
    adafruit/Adafruit ST7735 and ST7789 Library @ ^1.10.3
    adafruit/Adafruit BusIO @ ^1.15.0
```

### 🎯 사용법

#### LCD 사용
```cpp
RemoteLCD lcd;

lcd.begin();
lcd.drawMainScreen();
lcd.showButtonStatus(1, true);
lcd.showConnectionStatus(true);
lcd.showBatteryLevel(85);
```

#### 키보드 사용
```cpp
RemoteKeyboard keyboard;

keyboard.begin();
keyboard.scan();

while (keyboard.hasEvent()) {
    ButtonEventInfo event = keyboard.getEvent();
    // 이벤트 처리
}
```

### 🐛 버그 수정
- 버튼 디바운싱 개선
- 메모리 누수 방지
- 안정적인 ESP-NOW 통신

### 📚 추가 문서
- `docs/class-structure.md` - 클래스 구조 상세 설명
- 코드 주석 개선
- 예제 코드 추가

### 🔜 향후 계획
- [ ] 배터리 전압 ADC 측정
- [ ] 설정 메뉴 시스템
- [ ] Wi-Fi 설정 UI
- [ ] 저전력 모드 (Deep Sleep)
- [ ] OTA 업데이트 지원
