# ESP32 리모컨 하드웨어 핀 맵핑

## 📋 개요

이 문서는 ESP32 WROOM 기반 리모컨의 모든 하드웨어 핀 연결을 설명합니다.

## 🔘 5버튼 키패드 (12512WS-08)

### 버튼 연결
```
12512WS-08         ESP32
─────────────────────────
IOI_0 (SELECT)  → GPIO 12
IOI_1 (DOWN)    → GPIO 13
IOI_2 (RIGHT)   → GPIO 14
IOI_3 (LEFT)    → GPIO 27
IOI_4 (UP)      → GPIO 26

공통 (GND)      → GND
```

### 버튼 기능
- **SELECT (중앙)**: 선택/확인
- **UP**: 위로 이동
- **DOWN**: 아래로 이동
- **LEFT**: 왼쪽 이동
- **RIGHT**: 오른쪽 이동

### 특수 조합
- **SELECT + LEFT + RIGHT (1초 이상)**: 설정 모드 진입 (CAN 통신)

## 📺 TFT LCD (ST7789)

### SPI 연결
```
SZH-EK096          ESP32
─────────────────────────
CS (Chip Select) → GPIO 5
DC (Data/Cmd)    → GPIO 4
RST (Reset)      → GPIO 15
MOSI (Data)      → GPIO 23
SCLK (Clock)     → GPIO 18
VCC              → 3.3V
GND              → GND
```

### 스펙
- 컨트롤러: ST7789
- 해상도: 320 x 240
- 인터페이스: SPI
- 백라이트: 항상 켜짐

## 🔌 CAN 통신 (ESP32 내장)

### CAN 트랜시버 (SN65HVD232)
```
SN65HVD232         ESP32
─────────────────────────
TX                → GPIO 21
RX                → GPIO 22
VCC               → 3.3V
GND               → GND

CANH              ↔ CAN_H (차량)
CANL              ↔ CAN_L (차량)
```

### 설정
- 보드레이트: 500 kbps
- 모드: 정상 모드 (NORMAL)
- 종단 저항: 120Ω (필요 시)

## 💡 상태 LED

```
LED                ESP32
─────────────────────────
양극 (+)         → GPIO 2
음극 (-) + 330Ω  → GND
```

## 📡 WiFi/ESP-NOW

ESP32 내장 WiFi 사용 (추가 핀 없음)

## 🔌 전원

```
전원 입력          ESP32
─────────────────────────
VCC (3.3V)       → 3.3V
GND              → GND
EN (Enable)      → 10kΩ → 3.3V (풀업)
```

## 📊 전체 핀 맵핑 테이블

| GPIO | 기능 | 방향 | 연결 대상 | 비고 |
|------|------|------|-----------|------|
| 2 | LED | 출력 | 상태 LED | 330Ω 저항 |
| 4 | LCD DC | 출력 | ST7789 DC | 명령/데이터 |
| 5 | LCD CS | 출력 | ST7789 CS | 칩 선택 |
| 12 | BTN_SELECT | 입력 | 12512WS-08 IOI_0 | 풀업 |
| 13 | BTN_DOWN | 입력 | 12512WS-08 IOI_1 | 풀업 |
| 14 | BTN_RIGHT | 입력 | 12512WS-08 IOI_2 | 풀업 |
| 15 | LCD RST | 출력 | ST7789 RST | 리셋 |
| 18 | LCD SCLK | 출력 | ST7789 SCLK | SPI 클럭 |
| 21 | CAN TX | 출력 | SN65HVD232 TX | CAN 전송 |
| 22 | CAN RX | 입력 | SN65HVD232 RX | CAN 수신 |
| 23 | LCD MOSI | 출력 | ST7789 MOSI | SPI 데이터 |
| 26 | BTN_UP | 입력 | 12512WS-08 IOI_4 | 풀업 |
| 27 | BTN_LEFT | 입력 | 12512WS-08 IOI_3 | 풀업 |

## ⚠️ 주의사항

### 부팅 모드 관련
- **GPIO 0**: 사용 안 함 (부팅 모드 선택)
- **GPIO 2**: LED 사용 가능 (부팅 시 플로팅 필요)
- **GPIO 15**: LCD RST 사용 가능 (부팅 시 LOW 필요)

### 사용 불가 핀
- **GPIO 6-11**: 플래시 메모리 연결
- **GPIO 34-39**: 입력 전용 (풀업/풀다운 없음)

### CAN 버스 설정
- 120Ω 종단 저항 필요 (CAN 버스 양 끝)
- CANH, CANL은 트위스트 페어 케이블 사용 권장
- GND 공통 연결 필수

## 🔧 테스트 코드

### 버튼 테스트
```cpp
void testButtons() {
    pinMode(12, INPUT_PULLUP);  // SELECT
    pinMode(13, INPUT_PULLUP);  // DOWN
    pinMode(14, INPUT_PULLUP);  // RIGHT
    pinMode(27, INPUT_PULLUP);  // LEFT
    pinMode(26, INPUT_PULLUP);  // UP
    
    Serial.println("버튼 테스트 시작");
    while(1) {
        if (digitalRead(12) == LOW) Serial.println("SELECT");
        if (digitalRead(13) == LOW) Serial.println("DOWN");
        if (digitalRead(14) == LOW) Serial.println("RIGHT");
        if (digitalRead(27) == LOW) Serial.println("LEFT");
        if (digitalRead(26) == LOW) Serial.println("UP");
        delay(100);
    }
}
```

### CAN 루프백 테스트
```cpp
#include <driver/twai.h>

void testCAN() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NO_ACK);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    twai_driver_install(&g_config, &t_config, &f_config);
    twai_start();
    
    // 메시지 전송
    twai_message_t message;
    message.identifier = 0x123;
    message.data_length_code = 8;
    message.flags = TWAI_MSG_FLAG_NONE;
    
    for (int i = 0; i < 8; i++) {
        message.data[i] = i;
    }
    
    if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
        Serial.println("CAN 전송 성공");
    }
}
```

## 📐 회로도

### 전원 회로
```
3.3V ──┬── ESP32 VCC
       ├── ST7789 VCC
       ├── SN65HVD232 VCC
       ├── 10kΩ ── ESP32 EN
       └── 100nF ── GND (디커플링)
```

### CAN 트랜시버
```
ESP32 TX (GPIO21) ────→ SN65HVD232 D
ESP32 RX (GPIO22) ←──── SN65HVD232 R

SN65HVD232 CANH ──┬── 120Ω ── CANL (종단 저항)
                  └── CAN_H (차량)
                  
SN65HVD232 CANL ──┴── CAN_L (차량)
```

## 🔗 참고 자료

- [ESP32 핀아웃](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [12512WS-08 데이터시트](키패드 이미지 참조)
- [ST7789 데이터시트](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)
- [SN65HVD232 데이터시트](https://www.ti.com/product/SN65HVD232)
- [ESP32 TWAI 가이드](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html)
