# ESP32 리모컨 프로젝트 - 업데이트 노트

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
