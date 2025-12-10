# ESP32 Micro Hub - 차량 수신기 (ESP-NOW + CAN + LCD)

ESP32 WROOM 모듈을 사용한 ESP-NOW 및 CAN 통신 기반 운반차 수신기 프로젝트입니다.

## 📋 프로젝트 개요

이 프로젝트는 **운반차 수신기**로서 다음 기능을 수행합니다:
- **입력**: 리모컨 OR 로컬 버튼으로 명령 수신
- **제어**: CAN 통신으로 운반차 드라이버(STM32) 제어
- **표시**: LCD에 차량 상태 표시 (배터리, 온도, 속도 등)
- **통신**: ESP-NOW로 리모컨에 상태 정보 전송
- **클래스 기반 모듈 구조**: 간결하고 확장 가능한 설계

## 🏗️ 시스템 구조

```
┌──────────────┐         ESP-NOW          ┌─────────────────┐
│   리모컨     │ ◄─────────────────────► │  ESP32 수신기   │
│  (송신기)    │  명령/상태              │  (이 프로젝트)  │
└──────────────┘                         └────────┬────────┘
                                                  │ CAN
                                                  ↓
                                         ┌─────────────────┐
                                         │  운반차 드라이버 │
                                         │  (STM32 보드)   │
                                         └─────────────────┘
                                         
                ┌─────────────────┐
                │  로컬 버튼      │ → ESP32 수신기
                │  (10개 + ADC)   │   (리모컨 대체용)
                └─────────────────┘
                
                ┌─────────────────┐
                │  LCD 디스플레이 │ ← ESP32 수신기
                │  (차량 상태)    │   (상태 표시)
                └─────────────────┘
```

## ✨ 주요 기능

### 🚗 차량 제어 (이중 제어 시스템)

#### 1️⃣ 리모컨 제어 (우선)
- **ESP-NOW 무선 명령 수신**
  - SELECT: 비상 정지
  - UP: 전진 (50% 속도)
  - DOWN: 후진 (30% 속도)
  - LEFT: 좌회전
  - RIGHT: 우회전
- 리모컨 신호 있을 때 자동 우선 모드

#### 2️⃣ 로컬 버튼 제어 (대체)
- **리모컨 없을 때 자동 전환** (3초 타임아웃)
- **토글 스위치**:
  - 전진/후진 토글 (pIO_AN/AP)
  - 리프트 상/하 (pIO_CN/CP)
  - 캐스트 상/하 (pIO_DN/DP)
- **버튼**:
  - 좌/우회전 (pIO_BN/BP)
  - 비상정지 (pIO_1P)
  - 운전 스위치 (pIO_1N - foot switch)
- **ADC 입력**:
  - 스로틀 제어 (pAN0: 0-4095)
  - 속도 제한 (pAN1: 0-4095)
- **모터 제어**: YbCar 클래스를 통한 모터 구동
- **상태 LED**: 명령 수신 시각 피드백

### 📺 LCD 디스플레이
- **TFT LCD (240x320, ST7789)**: 차량 상태 실시간 표시
  - 배터리 잔량 (%)
  - 속도 (km/h)
  - 방향 (전진/후진/정지)
  - 모터 온도 (°C)
  - FET 온도 (°C)
  - 모터 전류 (mA)
  - 제어 모드 (리모컨/로컬버튼)
  - 0.5초 주기 업데이트

### 📡 통신 시스템
- **ESP-NOW (2.4GHz WiFi)**:
  - **수신**: 리모컨 명령 (5개 버튼 제어)
  - **송신**: 차량 상태 데이터 (1초 주기)
    - 속도, 방향, 배터리 레벨
    - 모터 온도, FET 온도, 모터 전류
  - WiFi 라우터 불필요
  - 저지연 통신 (< 10ms)
  - 양방향 통신
- **CAN 통신 (500kbps)**:
  - YCB 드라이브 모터 컨트롤러 제어
  - 모터 파라미터 조정
  - 센서 데이터 수집

### 📊 센서 데이터
- **배터리 모니터링**: 전압 및 잔량 측정
- **온도 센서**: 모터 및 FET 온도
- **전류 센서**: 모터 전류 측정
- **속도 측정**: 실시간 속도 계산

### 🚗 차량 설정 관리 (CAN)
1. **배터리/전력 설정**
   - 배터리 전압 (48V)
   - 전류 제한 (200A)
   - 저전압 임계값 (23V)
2. **온도 보호**
   - 모터 온도 제한 (90°C)
   - FET 온도 제한 (85°C)
3. **모터 제어**
   - 모터1/2 극성 설정
   - 전진/후진 속도 비율 (100%/80%)
   - 가속/감속 램프 (20/20)
4. **스로틀 설정**
   - 스로틀 오프셋 (300)
   - 변곡점 (900)
5. **브레이크 설정**
   - 브레이크 지연 (100ms)
   - 브레이크 레이트 (10)

## 🔧 하드웨어 구성

### 차량 수신기
- **MCU**: ESP32 WROOM (Dual-core, WiFi, BLE)
- **전원**: 3.3V (또는 차량 전원 레귤레이터)
- **LCD**: TFT 240x320 (ST7789, SPI)
  - CS: GPIO 5
  - DC: GPIO 4
  - RST: GPIO 15
  - MOSI: GPIO 23
  - SCLK: GPIO 18
- **CAN 트랜시버 (SN65HVD232)**: ESP32 내장 TWAI
  - TX: GPIO 21
  - RX: GPIO 22
  - 보드레이트: 500 kbps
  - 운반차 드라이버(STM32) 연결
- **상태 LED**: GPIO 2
- **로컬 제어 버튼** (10개):
  - 전진/후진: GPIO 32, 33
  - 좌/우회전: GPIO 25, 26
  - 리프트 상/하: GPIO 27, 14
  - 캐스트 상/하: GPIO 12, 13
  - 비상정지: GPIO 15
  - 운전(Foot Switch): GPIO 4
- **ADC 입력** (2개):
  - 스로틀(pAN0): GPIO 34 (ADC1_CH6)
  - 속도제한(pAN1): GPIO 35 (ADC1_CH7)
- **센서 인터페이스**:
  - 배터리 전압: CAN 또는 ADC
  - 온도 센서: I2C 또는 CAN
  - 전류 센서: CAN
- **UART**: RX, TX (프로그래밍 및 디버깅)

### 핀아웃 상세
자세한 하드웨어 연결은 [hardware-pinout.md](docs/hardware-pinout.md) 참고

## 📦 필요 사항

### 소프트웨어
- [PlatformIO](https://platformio.org/) (권장) 또는 Arduino IDE
- ESP32 보드 지원 패키지

### 하드웨어
- ESP32 WROOM 모듈 x 1 (차량 수신기)
- SN65HVD232 CAN 트랜시버
- LED x 1 (상태 표시용)
- 저항 (LED용: 330Ω)
- 120Ω 종단 저항 (CAN 버스)
- USB-UART 변환기 (프로그래밍용)
- YCB 드라이브 (모터 컨트롤러)

## 🚀 시작하기

### 1. 프로젝트 빌드

```bash
# PlatformIO 사용 시
pio run

# 또는 VS Code에서 PlatformIO 확장 사용
```

### 2. 차량 수신기 업로드

```bash
# PlatformIO 빌드 및 업로드
pio run --target upload

# 시리얼 모니터 열기 (MAC 주소 확인)
pio device monitor -b 115200
```

### 3. MAC 주소 확인

시리얼 모니터에서 차량 수신기의 MAC 주소 확인:

```
=== ESP32 차량 수신기 시작 ===
ESP-NOW 초기화 중...
MAC 주소: XX:XX:XX:XX:XX:XX
수신기 MAC 주소: XX:XX:XX:XX:XX:XX
이 주소를 리모컨에 설정하세요!
```

### 4. 리모컨 설정

리모컨 프로젝트의 `src/main.cpp`에서 이 MAC 주소 입력:

```cpp
// 차량 수신기 MAC 주소 (위에서 확인한 주소로 변경)
uint8_t receiverAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

### 5. 테스트

1. 차량 수신기 전원 켜기
2. 리모컨 전원 켜기
3. 리모컨 버튼 테스트:
   - **UP**: 전진 (50%)
   - **DOWN**: 후진 (30%)
   - **LEFT**: 좌회전
   - **RIGHT**: 우회전
   - **SELECT**: 비상 정지

### 6. 시리얼 모니터 출력 예제

```
데이터 수신: 12 bytes
버튼 ID: 1, 상태: 1
→ 전진: 50%
전진 (50%)

데이터 수신: 12 bytes
버튼 ID: 1, 상태: 0
■ 정지
차량 정지
```

### 5. CAN 트랜시버 연결

SN65HVD232 CAN 트랜시버를 다음과 같이 연결:

```
ESP32 GPIO 21 → SN65HVD232 TX
ESP32 GPIO 22 ← SN65HVD232 RX
ESP32 3.3V    → SN65HVD232 VCC
ESP32 GND     → SN65HVD232 GND

SN65HVD232 CANH ↔ 차량 CAN_H (120Ω 종단 저항)
SN65HVD232 CANL ↔ 차량 CAN_L
```

## 📡 동작 원리

### 통신 시스템

#### ESP-NOW 통신 (리모컨 ↔ 차량)
- **프로토콜**: ESP-NOW (Espressif 무선 프로토콜)
- **용도**: 실시간 차량 제어 및 상태 전송
- **특징**:
  - WiFi 라우터 불필요
  - 저지연 통신 (< 10ms)
  - 양방향 통신
    - 수신: 리모컨 버튼 명령
    - 송신: 차량 상태 (1초 주기)
  - 최대 250 바이트 데이터
  - 통신 거리: 약 200m (장애물 없을 시)

#### CAN 통신 (차량 ↔ YCB 드라이브)
- **프로토콜**: CAN 2.0A (Standard 11-bit ID)
- **하드웨어**: ESP32 내장 TWAI 컨트롤러
- **용도**: 모터 컨트롤러 제어 및 센서 데이터 수집
- **특징**:
  - 500kbps 보드레이트
  - 표준 11비트 식별자
  - 산업 표준 프로토콜
  - 노이즈 내성 강함
  - 통신 거리: 최대 40m @ 1Mbps

### 데이터 구조

#### ESP-NOW 메시지

**리모컨 → 차량 (버튼 명령)**
```cpp
struct struct_message {
  uint8_t buttonId;      // 버튼 ID (0-4: SELECT/UP/DOWN/LEFT/RIGHT)
  uint8_t buttonState;   // 상태 (0:릴리스, 1:눌림)
  uint32_t timestamp;    // 타임스탬프
};
```

**차량 → 리모컨 (상태 데이터)**
```cpp
struct vehicle_message {
  uint8_t speed;         // 속도 (km/h)
  uint8_t direction;     // 방향 (0:정지, 1:전진, 2:후진)
  uint8_t batteryLevel;  // 배터리 레벨 (0-100%)
  int16_t motorTemp;     // 모터 온도 (°C)
  uint16_t motorCurrent; // 모터 전류 (mA)
  int16_t fetTemp;       // FET 온도 (°C)
  uint32_t timestamp;    // 타임스탬프
};
```

### 제어 흐름

```
리모컨 버튼 입력
      ↓
 ESP-NOW 전송
      ↓
차량 수신기 수신
      ↓
onDataReceived()
      ↓
handleButtonCommand()
      ↓
YbCar 모터 제어
      ↓
CAN 명령 → YCB 드라이브
      ↓
센서 데이터 수집
      ↓
ESP-NOW 송신 (1초 주기)
      ↓
리모컨에 상태 표시
```

#### CAN 메시지 ID
```cpp
// 송신 (TX)
0x700: CAN_TX_GET_CONFIG      // Config 읽기 요청 (retrieve=1)
0x701: CAN_TX_PUT_CMD         // Config 쓰기 명령
0x708: CAN_TX_SAVE_CMD        // Config 저장 명령 (saving=1)

// 수신 (RX)
0x5B0-0x5B7: CAN_RX_DATA_ID_BASE  // Config 데이터 (8개 메시지, 각 8바이트 = 64바이트)
0x5B8: CAN_RX_RESPONSE_ID         // VCU 응답
```

## 🎮 사용법

### 일반 제어 모드 (ESP-NOW)
1. **전원 켜기**
   - LCD에 초기화 메시지 표시
   - LED 깜빡임: 준비 완료
2. **버튼 조작**
   - SELECT: 선택/확인
   - UP/DOWN/LEFT/RIGHT: 방향 제어
   - 버튼 누르면 ESP-NOW로 즉시 전송
3. **차량 상태 확인**
   - LCD에 실시간 표시: 속도, 배터리, 온도, RSSI
   - LED: 통신 상태 표시

### 설정 모드 진입 (CAN 통신)
1. **특수 조합 누르기**
   - SELECT + LEFT + RIGHT 동시에 1초 이상
   - LCD에 "설정 모드 진입..." 표시
2. **CAN 통신 시작**
   - MODE_ENTER (0x110) 메시지 전송
   - 차량이 ACK (0x112) 응답 대기
3. **설정 메뉴 탐색**
   - UP/DOWN: 항목 이동
   - LEFT/RIGHT: 값 조정
   - SELECT: 확인/저장
4. **설정 모드 종료**
   - SELECT 길게 누르기 (3초)
   - MODE_EXIT (0x111) 전송

### 차량 파라미터 설정 (16개 항목)
1. 배터리 전압 설정 (V)
2. 제한 전류 (A)
3. 제한 모터 온도 (°C)
4. 제한 FET 온도 (°C)
5. 저전압 임계값 (V)
6. Barity Im
7. 모터1 극성
8. 모터2 극성
9. 스로틀 오프셋
10. 스로틀 변곡점
11. 전진 비율 (%)
12. 후진 비율 (%)
13. 가속 램프
14. 감속 램프
15. 브레이크 지연 (ms)
16. 브레이크 레이트

## 🔨 커스터마이징

### 버튼 핀 변경
`src/class/button/RemoteButton.h` 수정:
```cpp
static const uint8_t PIN_BTN_SELECT = 12;  // 원하는 GPIO로 변경
static const uint8_t PIN_BTN_DOWN = 13;
// ...
```

### CAN 보드레이트 변경
`src/class/cancom/RemoteCANCom.cpp` 수정:
```cpp
// 500kbps → 250kbps로 변경 예시
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
```

### LCD 핀 변경
`src/class/lcd/RemoteLCD.h` 수정:
```cpp
static const uint8_t TFT_CS = 5;   // 원하는 GPIO
static const uint8_t TFT_DC = 4;
static const uint8_t TFT_RST = 15;
```

### 차량 설정 파라미터 추가
1. `src/class/ybcarDoctor/YbCarDoctor.h`에 변수 추가
2. CAN 메시지 처리 함수 업데이트
3. LCD 표시 함수에 UI 추가

### 전력 최적화
- Deep Sleep 사용 시:
  - GPIO 웨이크업 설정 (버튼 인터럽트)
  - ESP-NOW 재초기화 필요
- Light Sleep:
  - WiFi 모뎀 슬립 (esp_wifi_set_ps(WIFI_PS_MIN_MODEM))

## 🐛 문제 해결

### 5버튼 입력 안 됨
- **증상**: 버튼 눌러도 반응 없음
- **확인 사항**:
  - GPIO 연결 확인 (SELECT=12, DOWN=13, RIGHT=14, LEFT=27, UP=26)
  - 공통 GND 연결 확인
  - INPUT_PULLUP 설정 확인 (내부 풀업 사용)
  - 버튼 스위치 정상 동작 확인 (멀티미터로 단락 테스트)

### ESP-NOW 초기화 실패
- **증상**: "ESP-NOW 초기화 실패" 메시지
- **해결**:
  - WiFi 모드 확인 (WIFI_MODE_STA)
  - ESP32 보드 리셋
  - 다른 WiFi 라이브러리와 충돌 확인
  - 펌웨어 재업로드

### CAN 통신 안 됨
- **증상**: 설정 모드 진입 안 됨
- **확인 사항**:
  - SN65HVD232 트랜시버 연결 (TX=21, RX=22)
  - CAN_H, CAN_L 배선 확인 (트위스트 페어)
  - 120Ω 종단 저항 설치 (CAN 버스 양 끝)
  - 차량측 CAN 컨트롤러 동작 확인
  - CAN 보드레이트 일치 (500kbps)
  - 시리얼 모니터에서 CAN 메시지 로그 확인

### LCD 표시 안 됨
- **증상**: LCD 화면 깜깜함 또는 노이즈
- **확인 사항**:
  - SPI 핀 연결 (CS=5, DC=4, RST=15, MOSI=23, SCLK=18)
  - 3.3V 전원 공급 확인
  - 백라이트 연결 확인
  - RST 핀 동작 확인
  - Adafruit 라이브러리 버전 확인

### 차량 데이터 수신 안 됨
- **증상**: LCD에 차량 상태 표시 안 됨, RSSI: 0
- **확인 사항**:
  - 차량 MAC 주소 올바르게 설정
  - 차량측 ESP-NOW 송신 코드 동작 확인
  - 통신 거리 (최대 200m)
  - 장애물 확인
  - RSSI 값 모니터링 (시리얼 출력)

### 메모리 부족
- **증상**: 빌드 실패 또는 런타임 크래시
- **해결**:
  - PSRAM 활성화 (platformio.ini)
  - LCD 버퍼 크기 최적화
  - 사용하지 않는 클래스 제거
  - 로그 레벨 낮추기 (esp_log_level_set)

### 버튼이 반응하지 않음
- PCA9555 I2C 통신 확인
- 버튼 연결 확인 (IOI 핀 - GND)
- 시리얼 모니터로 버튼 입력 확인

## 📚 참고 자료

### 통신 프로토콜
- [ESP-NOW 공식 문서](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [ESP32 TWAI (CAN) 드라이버](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html)
- [CAN 사양 및 지식](https://www.can-cia.org/can-knowledge/)

### 하드웨어
- [ESP32 핀아웃 참조](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ST7789 TFT LCD 데이터시트](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)
- [SN65HVD232 CAN 트랜시버](https://www.ti.com/product/SN65HVD232)
- [12512WS-08 5버튼 키패드](docs/12512ws-08_5button_spec.md)

### 개발 도구
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [PlatformIO ESP32 가이드](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- [Adafruit GFX 라이브러리](https://learn.adafruit.com/adafruit-gfx-graphics-library)

### 프로젝트 문서
- [하드웨어 핀 맵핑](docs/hardware-pinout.md)
- [클래스 구조 설명](docs/class-structure.md)
- [5버튼 키패드 사양](docs/12512ws-08_5button_spec.md)

## 📁 프로젝트 구조

```
esp32_core_remocon/
├── src/
│   ├── main.cpp                          # 메인 프로그램
│   └── class/
│       ├── lcd/                          # LCD 디스플레이 클래스
│       │   ├── RemoteLCD.h
│       │   └── RemoteLCD.cpp
│       ├── button/                       # 5버튼 입력 클래스
│       │   ├── RemoteButton.h
│       │   └── RemoteButton.cpp
│       ├── led/                          # 상태 LED 클래스
│       │   ├── RemoteLED.h
│       │   └── RemoteLED.cpp
│       ├── espnow/                       # ESP-NOW 통신 클래스
│       │   ├── RemoteESPNow.h
│       │   └── RemoteESPNow.cpp
│       ├── cancom/                       # CAN 통신 클래스 (ESP32 TWAI)
│       │   ├── RemoteCANCom.h
│       │   └── RemoteCANCom.cpp
│       ├── ybcar/                        # 차량 데이터 관리 클래스
│       │   ├── YbCar.h
│       │   └── YbCar.cpp
│       └── ybcarDoctor/                  # 차량 설정 관리 클래스
│           ├── YbCarDoctor.h
│           └── YbCarDoctor.cpp
├── docs/
│   ├── hardware-pinout.md                # 하드웨어 핀 맵핑
│   ├── 12512ws-08_5button_spec.md        # 5버튼 키패드 사양
│   └── class-structure.md                # 클래스 구조 설명
├── platformio.ini                        # PlatformIO 설정
└── README.md                             # 프로젝트 설명
```

### 클래스 구조

#### 1. RemoteLCD (LCD 디스플레이)
- TFT LCD 초기화 및 제어
- 차량 상태 표시 (속도, 배터리, 온도, RSSI)
- 버튼 상태 표시
- 설정 메뉴 UI

#### 2. RemoteButton (5버튼 입력)
- GPIO 직접 입력 (INPUT_PULLUP)
- 디바운싱, 롱프레스, 더블클릭
- 특수 조합 감지 (SELECT+LEFT+RIGHT → 설정 모드)
- 이벤트 핸들러

#### 3. RemoteLED (상태 LED)
- LED 제어 (켜기/끄기/깜빡임)
- 성공/에러 패턴 표시

#### 4. RemoteESPNow (ESP-NOW 통신)
- 양방향 ESP-NOW 통신
- 버튼 명령 전송
- 차량 데이터 수신
- RSSI 모니터링

#### 5. RemoteCANCom (CAN 통신)
- ESP32 내장 TWAI 드라이버 사용
- 설정 모드 진입/종료
- 설정값 요청/응답/업데이트
- CAN 메시지 처리

#### 6. YbCar (차량 데이터)
- 차량 상태 데이터 관리
- ESP-NOW로 수신한 데이터 저장
- LCD 디스플레이 업데이트

#### 7. YbCarDoctor (차량 설정)
- 16개 파라미터 관리
- CAN 통신으로 설정 송수신
- Preferences를 사용한 EEPROM 저장
- 설정 메뉴 UI 제어

## 📄 라이선스

MIT License

## 👤 작성자

프로젝트 작성일: 2025-12-09

## 📝 버전 히스토리

### v2.1.0 (2025-12-10) - UI Redesign & Korean Font
- **UI 개선**
  - 5버튼 십자 레이아웃으로 재설계
  - 16x16 한글 조합형 폰트 통합 (~11KB)
  - UTF-8 혼합 텍스트 렌더링 (한글+영문)
  - 차량 상태 표시 영역 재배치
  - 📺 **[UI 디자인 문서](docs/ui-design.md)** | **[다이어그램](docs/ui-diagram.md)**
- **폰트 시스템**
  - `draw16String()`: UTF-8 문자열 렌더링
  - `draw16Korean()`: 초성(20) + 중성(22) + 종성(28) 조합
  - `draw16English()`: 8x16 ASCII 렌더링
  - 스케일링 및 투명 배경 지원

### v2.0.0 (2025-12-09) - Major Refactor
- **하드웨어 변경**
  - 12버튼 PCA9555 I2C → 5버튼 직접 GPIO (12512WS-08)
  - 외부 MCP2515 CAN → ESP32 내장 TWAI CAN
- **CAN 통신 추가**
  - RemoteCANCom 클래스 구현
  - 16개 차량 파라미터 설정 기능
  - 설정 모드 진입 조합 (SELECT+LEFT+RIGHT, 1초)
- **이중 통신 아키텍처**
  - ESP-NOW: 실시간 차량 제어
  - CAN: 차량 설정 관리
- **핀 맵핑 변경**
  - GPIO 21/22: I2C → CAN TX/RX
  - GPIO 12/13/14/26/27: 5버튼 직접 입력

### v1.0.0 (2025-12-08) - Initial Release
- 12버튼 PCA9555 I2C 리모컨
- ESP-NOW 통신
- TFT LCD 디스플레이 (ST7789)
- 클래스 기반 모듈 구조
- YbCar 차량 데이터 관리
- YbCarDoctor 차량 설정 관리

## 🔮 향후 개선 사항

- [ ] 배터리 전압 ADC 모니터링 (리모컨 자체 배터리)
- [ ] Deep Sleep 모드 구현 (버튼 웨이크업)
- [ ] CAN 설정 UI 완성 (네비게�