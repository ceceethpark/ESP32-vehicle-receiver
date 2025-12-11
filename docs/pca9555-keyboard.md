# PCA9555 I2C 키보드 설정 가이드

## 📋 개요

ESP32 리모컨은 PCA9555 I2C GPIO 확장 칩을 사용하여 12개의 버튼 입력을 처리합니다.

## 🔌 하드웨어 연결

### PCA9555 핀 연결
```
ESP32          PCA9555
GPIO 21 (SDA) → SDA
GPIO 22 (SCL) → SCL
3.3V          → VCC
GND           → GND
              → A0 (GND)  // 주소 설정
              → A1 (GND)  // 주소 설정
              → A2 (GND)  // 주소 설정
```

### I2C 주소
- **기본 주소**: 0x20 (A0, A1, A2 모두 GND)
- 다른 주소가 필요한 경우 A0~A2 핀으로 설정 가능

### 버튼 연결 (12개)
```
PCA9555 포트    버튼 ID    연결
IOI_0          0         버튼 1
IOI_1          1         버튼 2
IOI_2          2         버튼 3
IOI_3          3         버튼 4
IOI_4          4         버튼 5
IOI_5          5         버튼 6
IOI_6          6         버튼 7
IOI_7          7         버튼 8
IOI_8          8         버튼 9
IOI_9          9         버튼 10
IOI_10         10        버튼 11
IOI_11         11        버튼 12
```

각 버튼은 PCA9555 IOI 핀과 GND 사이에 연결 (내부 풀업 사용)

## 📡 통신 정보

### PCA9555 레지스터
- **INPUT_PORT_0 (0x00)**: IOI_0 ~ IOI_7 입력 상태
- **INPUT_PORT_1 (0x01)**: IOI_8 ~ IOI_15 입력 상태
- **CONFIG_PORT_0 (0x06)**: Port 0 방향 설정 (1=입력)
- **CONFIG_PORT_1 (0x07)**: Port 1 방향 설정 (1=입력)

### 통신 설정
- **I2C 주파수**: 400kHz (Fast Mode)
- **버스 전압**: 3.3V
- **풀업 저항**: 4.7kΩ (SDA, SCL)

## 💻 소프트웨어 설정

### RemoteButton 클래스 사용

```cpp
#include "class/button/RemoteButton.h"

RemoteButton buttons;

void setup() {
    // 키보드 초기화
    if (!buttons.begin()) {
        Serial.println("PCA9555 초기화 실패!");
        return;
    }
    Serial.println("12개 키보드 준비완료");
}

void loop() {
    // 버튼 스캔
    buttons.scan();
    
    // 이벤트 처리
    buttons.processEvents();  // 자동 처리
    
    // 또는 수동 처리
    while (buttons.hasEvent()) {
        ButtonEventInfo event = buttons.getEvent();
        
        Serial.print("버튼 ");
        Serial.print(event.buttonId);
        Serial.print(" - ");
        
        switch (event.event) {
            case BUTTON_PRESSED:
                Serial.println("눌림");
                break;
            case BUTTON_RELEASED:
                Serial.println("릴리스");
                break;
            case BUTTON_LONG_PRESS:
                Serial.println("롱프레스");
                break;
        }
    }
}
```

### 버튼 ID 상수
```cpp
buttons.BUTTON_1   // 0  (IOI_0)
buttons.BUTTON_2   // 1  (IOI_1)
buttons.BUTTON_3   // 2  (IOI_2)
buttons.BUTTON_4   // 3  (IOI_3)
buttons.BUTTON_5   // 4  (IOI_4)
buttons.BUTTON_6   // 5  (IOI_5)
buttons.BUTTON_7   // 6  (IOI_6)
buttons.BUTTON_8   // 7  (IOI_7)
buttons.BUTTON_9   // 8  (IOI_8)
buttons.BUTTON_10  // 9  (IOI_9)
buttons.BUTTON_11  // 10 (IOI_10)
buttons.BUTTON_12  // 11 (IOI_11)
```

## 📺 LCD 디스플레이

12개 버튼은 LCD에 3x4 그리드로 표시됩니다.

```
┌──────────────────────────────┐
│     ESP32 리모컨             │
├──────────────────────────────┤
│[K0] [K1] [K2] [K3]           │
│[K4] [K5] [K6] [K7]           │
│[K8] [K9] [K10] [K11]         │
├──────────────────────────────┤
│배터리: 85%    연결: ●        │
│PCA9555 12키 리모컨           │
└──────────────────────────────┘
```

## ⚙️ 설정 변경

### I2C 주소 변경
`RemoteButton.h`에서:
```cpp
static const uint8_t PCA9555_ADDRESS = 0x20;  // 원하는 주소로 변경
```

### I2C 핀 변경
`RemoteButton.h`에서:
```cpp
static const uint8_t I2C_SDA = 21;  // 다른 핀으로 변경
static const uint8_t I2C_SCL = 22;  // 다른 핀으로 변경
```

### 디바운스 시간 변경
```cpp
buttons.setDebounceTime(50);      // 50ms (기본값)
buttons.setLongPressTime(1000);   // 1초 (기본값)
```

## 🐛 문제 해결

### I2C 통신 오류
```
PCA9555 초기화 실패 - I2C 통신 오류
```
**해결 방법**:
1. I2C 연결 확인 (SDA, SCL)
2. 풀업 저항 확인 (4.7kΩ)
3. PCA9555 전원 확인 (3.3V)
4. I2C 주소 확인 (A0, A1, A2 설정)

### 버튼이 반응하지 않음
1. 버튼 연결 확인 (IOI 핀 - GND)
2. 시리얼 모니터에서 입력 확인
3. I2C 스캐너로 PCA9555 주소 확인

### I2C 스캐너 코드
```cpp
void scanI2C() {
    Serial.println("I2C 스캔 시작...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("I2C 장치 발견: 0x");
            Serial.println(addr, HEX);
        }
    }
    Serial.println("스캔 완료");
}
```

## 📚 PCA9555 데이터시트
- 제조사: Texas Instruments / NXP
- 16비트 I2C GPIO 확장기
- I2C Fast Mode 지원 (400kHz)
- 입력 풀업 지원

## 🔗 추가 정보
- [PCA9555 데이터시트](https://www.ti.com/product/PCA9555)
- [I2C 프로토콜](https://www.i2c-bus.org/)
- [ESP32 I2C 가이드](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)

## ✅ 테스트 체크리스트

- [ ] PCA9555가 I2C 주소 0x20에서 감지됨
- [ ] 각 버튼이 시리얼 모니터에 표시됨
- [ ] LCD에서 버튼 상태가 시각적으로 확인됨
- [ ] 롱프레스가 1초 후 감지됨
- [ ] 디바운싱이 정상 작동함 (깨끗한 입력)
