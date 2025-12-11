# ESP32 WROOM 핀 설정 참고

## 리모컨 핀 매핑

### LCD 디스플레이 (SPI)
- **CS**: GPIO 5
- **DC**: GPIO 4
- **RST**: GPIO 15
- **MOSI**: GPIO 23
- **SCLK**: GPIO 18

### 키보드 (I2C - PCA9555)
- **SDA**: GPIO 21
- **SCL**: GPIO 22
- **주소**: 0x20
- **버튼**: IOI_0 ~ IOI_11 (12개)

### LED
- **상태 LED**: GPIO 2

### 통신
- **TX**: GPIO 1 (UART TX)
- **RX**: GPIO 3 (UART RX)

### 전원
- **VCC**: 3.3V
- **GND**: Ground
- **EN**: Enable (풀업 필요)

## 📐 전체 핀아웃 다이어그램

```
ESP32 WROOM 모듈
┌─────────────────────────────┐
│ EN                      GPIO23├──→ LCD MOSI
│ GPIO36                  GPIO22├──→ I2C SCL
│ GPIO39                  TXD0  │
│ GPIO34                  RXD0  │
│ GPIO35                  GPIO21├──→ I2C SDA
│ GPIO32                  GPIO19│
│ GPIO33                  GPIO18├──→ LCD SCLK
│ GPIO25                  GPIO5 ├──→ LCD CS
│ GPIO26                  GPIO17│
│ GPIO27                  GPIO16│
│ GPIO14                  GPIO4 ├──→ LCD DC
│ GPIO12                  GPIO0 │
│ GPIO13                  GPIO2 ├──→ LED
│ SD2                     GPIO15├──→ LCD RST
│ SD3                     SD1   │
│ CMD                     CLK   │
│ 3V3       GND    GND    GND   │
└─────────────────────────────┘
```

## 버튼 연결 방법 (PCA9555)

### PCA9555 연결
```
PCA9555 IOI_0  ──┬── 버튼 1 ── GND
PCA9555 IOI_1  ──┬── 버튼 2 ── GND
PCA9555 IOI_2  ──┬── 버튼 3 ── GND
...
PCA9555 IOI_11 ──┬── 버튼 12 ── GND
```

내부 풀업이 활성화되어 있으므로 외부 저항 불필요

## ⚠️ 주의사항

### 사용 금지 핀
- **GPIO 6-11**: 플래시 메모리 연결 (사용 불가)
- **GPIO 0**: 부팅 모드 선택 (LOW=프로그래밍 모드)
- **GPIO 2**: 부팅 시 플로팅 필요 (LED는 OK)
- **GPIO 15**: 부팅 시 LOW 필요 (LCD RST는 OK)

### 입력 전용 핀
- **GPIO 34-39**: 입력 전용 (풀업/풀다운 없음)

### 권장 사항
- **풀업 저항**: I2C 라인에 4.7kΩ (SDA, SCL)
- **전원 디커플링**: 100nF 세라믹 커패시터 (ESP32 VCC-GND 사이)
- **LCD 전원**: 안정적인 3.3V 공급

## 🔧 연결 검증

### I2C 스캔 코드
```cpp
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);  // SDA, SCL
    
    Serial.println("I2C 스캔 시작...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C 장치 발견: 0x%02X\n", addr);
        }
    }
}

void loop() {}
```

### SPI 테스트 코드
```cpp
#include <SPI.h>
#include <Adafruit_ST7789.h>

#define TFT_CS    5
#define TFT_DC    4
#define TFT_RST   15
#define TFT_MOSI  23
#define TFT_SCLK  18

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
    tft.init(240, 320);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("LCD Test OK!");
}

void loop() {}
```

## 📊 전력 소비

| 구성요소 | 전류 | 참고 |
|---------|------|------|
| ESP32 WROOM | ~80mA | WiFi OFF, CPU 활성 |
| ST7789 LCD | ~30mA | 백라이트 포함 |
| PCA9555 | ~1mA | 대기 상태 |
| LED | ~20mA | GPIO 2 (330Ω 저항) |
| **총합** | **~130mA** | 일반 동작 시 |

배터리: 1000mAh 기준 약 7-8시간 사용 가능

## 🔌 권장 회로도

```
ESP32 3.3V ──┬── PCA9555 VCC
             ├── LCD VCC
             └── 100nF ── GND

ESP32 GND ───┬── PCA9555 GND
             ├── LCD GND
             ├── PCA9555 A0/A1/A2
             └── 버튼 공통

GPIO 21 (SDA) ───┬── 4.7kΩ ── 3.3V
                 └── PCA9555 SDA

GPIO 22 (SCL) ───┬── 4.7kΩ ── 3.3V
                 └── PCA9555 SCL
```

## 📝 핀 사용 요약

| GPIO | 기능 | 방향 | 참고 |
|------|------|------|------|
| 2 | LED | 출력 | 상태 표시 |
| 4 | LCD DC | 출력 | 명령/데이터 선택 |
| 5 | LCD CS | 출력 | 칩 선택 |
| 15 | LCD RST | 출력 | 리셋 |
| 18 | LCD SCLK | 출력 | SPI 클럭 |
| 21 | I2C SDA | I/O | 4.7kΩ 풀업 |
| 22 | I2C SCL | I/O | 4.7kΩ 풀업 |
| 23 | LCD MOSI | 출력 | SPI 데이터 |

**사용 가능한 GPIO**: 12, 13, 14, 16, 17, 19, 25, 26, 27, 32, 33

## 🔗 참고 자료
- [ESP32 핀아웃 레퍼런스](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ST7789 데이터시트](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)
- [PCA9555 데이터시트](https://www.ti.com/product/PCA9555)
