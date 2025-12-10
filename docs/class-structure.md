# 클래스 구조 설명

## 📂 프로젝트 구조

```
esp32_core_remocon/
├── include/
│   └── class/
│       ├── lcd/
│       │   └── RemoteLCD.h          # LCD 클래스 헤더
│       └── button/
│           └── RemoteButton.h     # 키보드 클래스 헤더
├── src/
│   ├── class/
│   │   ├── lcd/
│   │   │   └── RemoteLCD.cpp        # LCD 클래스 구현
│   │   ├── button/
│   │   │   └── RemoteButton.cpp   # 키보드 클래스 구현
│   │   ├── led/
│   │   │   └── RemoteLED.cpp      # LED 클래스
│   │   ├── espnow/
│   │   │   └── RemoteESPNow.cpp   # ESP-NOW 클래스
│   │   ├── ybcar/
│   │   │   └── YbCar.cpp          # 차량 데이터 클래스
│   │   └── ybcarDoctor/
│   │       └── YbCarDoctor.cpp    # 차량 설정 클래스
│   └── main.cpp                     # 메인 프로그램
├── examples/
│   └── receiver.cpp                 # 수신기 예제
├── docs/
│   ├── pinout.md                    # 핀 배치
│   ├── quick-start.md               # 빠른 시작
│   ├── class-structure.md           # 클래스 구조 (이 파일)
│   └── pca9555-keyboard.md          # PCA9555 가이드
├── platformio.ini                   # PlatformIO 설정
└── README.md                        # 프로젝트 문서
```

## 📺 RemoteLCD 클래스

### 위치
- 헤더: `src/class/lcd/RemoteLCD.h`
- 구현: `src/class/lcd/RemoteLCD.cpp`

### 주요 기능
```cpp
class RemoteLCD {
public:
    // 초기화
    bool begin();
    
    // 화면 제어
    void clear();
    void displayOn();
    void displayOff();
    
    // 텍스트 출력
    void printText(const char* text, uint16_t x, uint16_t y, uint16_t color);
    void printTextCentered(const char* text, uint16_t y, uint16_t color);
    void setTextSize(uint8_t size);
    
    // 상태 표시
    void showButtonStatus(uint8_t buttonId, bool pressed);
    void showConnectionStatus(bool connected);
    void showBatteryLevel(uint8_t percentage);
    
    // 차량 정보 표시
    void showVehicleSpeed(int speed);
    void showVehicleDirection(int direction);
    void showVehicleBattery(int level);
    void showMotorTemp(int temp);
    void showMotorCurrent(int current);
    void showFetTemp(int temp);
    void showRSSI(int rssi);
    
    // UI 그리기
    void drawMainScreen();
    void drawButton(...);
    void drawProgressBar(...);
};
```

### 사용 예제
```cpp
RemoteLCD lcd;

void setup() {
    lcd.begin();
    lcd.drawMainScreen();
    lcd.showConnectionStatus(true);
    lcd.showBatteryLevel(100);
}

void loop() {
    lcd.showButtonStatus(1, true);  // 버튼 1 눌림 표시
    lcd.showVehicleSpeed(50);        // 속도 50 표시
}
```

### LCD 스펙
- **모델**: SZH-EK096 (ST7789 컨트롤러)
- **통신**: SPI (4 I/O)
- **해상도**: 320 x 240
- **전원**: DC 3.3V
- **핀 연결**:
  - CS: GPIO 5
  - DC: GPIO 4
  - RST: GPIO 15
  - MOSI: GPIO 23
  - SCLK: GPIO 18

## 🎮 RemoteButton 클래스

### 위치
- 헤더: `src/class/button/RemoteButton.h`
- 구현: `src/class/button/RemoteButton.cpp`

### 주요 기능
```cpp
class RemoteButton {
public:
    // 초기화
    bool begin();
    
    // 버튼 스캔
    void scan();
    
    // 버튼 상태 확인
    bool isButtonPressed(uint8_t buttonId);
    bool wasButtonJustPressed(uint8_t buttonId);
    bool wasButtonJustReleased(uint8_t buttonId);
    
    // 이벤트 처리
    bool hasEvent();
    ButtonEventInfo getEvent();
    void processEvents();
    
    // 설정
    void setDebounceTime(unsigned long ms);
    void setLongPressTime(unsigned long ms);
    void setDoubleClickTime(unsigned long ms);
    
    // 핸들러 설정
    void setHandlers(RemoteLCD* lcd, RemoteESPNow* espNow);
};
```

### 이벤트 타입
```cpp
enum ButtonEvent {
    BUTTON_NONE = 0,
    BUTTON_PRESSED,       // 버튼 눌림
    BUTTON_RELEASED,      // 버튼 릴리스
    BUTTON_LONG_PRESS,    // 롱프레스 (1초)
    BUTTON_DOUBLE_CLICK   // 더블클릭 (300ms 이내)
};
```

### 사용 예제
```cpp
RemoteButton buttons;

void setup() {
    buttons.begin();
    buttons.setLongPressTime(1000);  // 1초 롱프레스
}

void loop() {
    buttons.scan();
    buttons.processEvents();  // 자동으로 이벤트 처리
}
```

### 버튼 핀 매핑 (PCA9555 I2C)
- I2C SDA: GPIO 21
- I2C SCL: GPIO 22
- PCA9555 주소: 0x20
- 버튼 0~11: IOI_0 ~ IOI_11 (12개)

## 💡 RemoteLED 클래스

### 주요 기능
```cpp
class RemoteLED {
public:
    void begin();
    void on();
    void off();
    void toggle();
    void blink(unsigned long onTime, unsigned long offTime);
    void blinkTimes(uint8_t count, unsigned long duration);
    void showSuccess();
    void showError();
    void update();
};
```

## 📡 RemoteESPNow 클래스

### 주요 기능
```cpp
class RemoteESPNow {
public:
    bool begin();
    void setReceiver(uint8_t* macAddress);
    void sendButtonPress(uint8_t buttonId);
    void sendButtonRelease(uint8_t buttonId);
    void sendButtonState(uint8_t buttonId, bool pressed);
    void setSendCallback(void (*callback)(bool));
    void setUpdateCallback(void (*callback)());
    void setReceiveCallback(void (*callback)(const uint8_t*, int));
    int getRSSI();
    void update();
};
```

## 🚗 YbCar 클래스

### 주요 기능
```cpp
class YbCar {
public:
    void begin(RemoteLCD* lcd, RemoteESPNow* espNow);
    void updateVehicleData(int speed, int direction, int battery, 
                          int motorTemp, int motorCurrent, int fetTemp);
    void updateDisplay();
    int getSpeed();
    int getDirection();
    int getBatteryLevel();
    int getMotorTemp();
    int getMotorCurrent();
    int getFetTemp();
    bool isConnected();
};
```

## ⚙️ YbCarDoctor 클래스

### 주요 기능
```cpp
class YbCarDoctor {
public:
    void begin(RemoteLCD* lcd, RemoteESPNow* espNow);
    void requestSettings();
    bool updateSettings(const VehicleSettings& settings);
    void handleSettingsMessage(const uint8_t* data, int len);
    void displaySettings();
    void displaySettingsMenu();
    
    // 16개 설정 항목 setter 함수들
    void setBatteryVoltage(int voltage);
    void setLimitCurrent(int current);
    void setLimitMotorTemp(int temp);
    // ... 등
};
```

### 차량 설정 구조
```cpp
struct VehicleSettings {
    int batteryVoltage;    // 48V (4800)
    int limitCurrent;      // 200A (20000)
    int limitMotorTemp;    // 90°C
    int limitFetTemp;      // 85°C
    int lowBattery;        // 23V (2300)
    int barityIm;          // 0=무한궤도, 1=바퀴
    int motor1Polarity;    // 0=정회전, 1=역회전
    int motor2Polarity;    // 0=정회전, 1=역회전
    int throttleOffset;    // 300
    int throttleInflec;    // 900
    int forward;           // 100%
    int backward;          // 80%
    int accel;             // 20
    int decel;             // 20
    int brakeDelay;        // 100
    int brakeRate;         // 10
};
```

## 🔧 main.cpp 통합

### 전체 흐름
```cpp
#include "class/lcd/RemoteLCD.h"
#include "class/button/RemoteButton.h"
#include "class/led/RemoteLED.h"
#include "class/espnow/RemoteESPNow.h"
#include "class/ybcar/YbCar.h"
#include "class/ybcarDoctor/YbCarDoctor.h"

RemoteLCD lcd;
RemoteButton buttons;
RemoteLED led;
RemoteESPNow espNow;
YbCar ybcar;
YbCarDoctor doctor;

void setup() {
    // LCD 초기화
    lcd.begin();
    lcd.drawMainScreen();
    
    // 버튼 초기화
    buttons.begin();
    buttons.setHandlers(&lcd, &espNow);
    
    // LED 초기화
    led.begin();
    led.blink(100, 100);
    
    // ESP-NOW 초기화
    espNow.begin();
    espNow.setReceiver(receiverAddress);
    espNow.setSendCallback(onSendComplete);
    espNow.setUpdateCallback(onStatusUpdate);
    espNow.setReceiveCallback(onDataReceived);
    
    // 차량 데이터 초기화
    ybcar.begin(&lcd, &espNow);
    
    // 차량 설정 초기화
    doctor.begin(&lcd, &espNow);
}

void loop() {
    buttons.scan();
    led.update();
    buttons.processEvents();
    espNow.update();
}
```

## 📐 설계 철학

### 1. 캡슐화
- LCD 관련 모든 기능은 `RemoteLCD` 클래스에
- 버튼 관련 모든 기능은 `RemoteButton` 클래스에
- main.cpp는 통합 로직만 담당

### 2. 직관적인 네이밍
- `RemoteLCD` - 리모컨의 LCD
- `RemoteButton` - 리모컨의 버튼
- 메서드 이름이 명확하게 (`showButtonStatus`, `drawMainScreen`)

### 3. 확장성
- 새로운 기능 추가가 쉬움
- 클래스 단위로 테스트 가능
- 다른 프로젝트에 재사용 가능

### 4. 폴더 구조
```
class/
├── lcd/           # LCD 관련 모든 파일
│   ├── RemoteLCD.h
│   └── RemoteLCD.cpp
├── button/        # 버튼 관련 모든 파일
│   ├── RemoteButton.h
│   └── RemoteButton.cpp
├── led/           # LED 관련
├── espnow/        # ESP-NOW 관련
├── ybcar/         # 차량 데이터 관련
└── ybcarDoctor/   # 차량 설정 관련
```

## 🔮 확장 가능한 기능

### LCD 클래스 확장
```cpp
// 메뉴 시스템 추가
void RemoteLCD::showMenu(MenuItem* items, uint8_t count);

// 그래프 그리기
void RemoteLCD::drawGraph(int* data, uint8_t count);

// 아이콘 표시
void RemoteLCD::drawIcon(Icon icon, uint16_t x, uint16_t y);
```

### 버튼 클래스 확장
```cpp
// 버튼 조합 지원
bool RemoteButton::areButtonsPressed(uint8_t btn1, uint8_t btn2);

// 제스처 인식
GestureType RemoteButton::detectGesture();

// 커스텀 이벤트
void RemoteButton::registerCustomEvent(CustomEventHandler handler);
```

## 📝 코딩 규칙

1. **클래스 이름**: PascalCase (예: `RemoteLCD`)
2. **메서드 이름**: camelCase (예: `showButtonStatus`)
3. **상수**: UPPER_CASE (예: `BUTTON_COUNT`)
4. **멤버 변수**: camelCase with prefix (예: `currentTextSize`)
5. **파일 이름**: 클래스 이름과 동일 (예: `RemoteLCD.h`, `RemoteLCD.cpp`)

## 📊 빌드 결과

- **RAM 사용량**: 13.6% (44,728/327,680 bytes)
- **Flash 사용량**: 60.8% (796,361/1,310,720 bytes)
- **빌드 시간**: ~7-30초
