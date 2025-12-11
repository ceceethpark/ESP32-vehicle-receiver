# ESP-IDF micro-ROS 프로젝트 설정

## 현재 상황
- Arduino 프레임워크에서 micro-ROS 빌드 실패
- ESP-IDF 프레임워크로 전환 필요
- micro-ROS는 ESP-IDF를 공식 지원

## 전환 계획

### 1. 환경 설정
- ESP-IDF v4.4+ 설치 필요
- micro-ROS ESP32 컴포넌트 사용

### 2. 프로젝트 구조 변경
```
esp32_micro_hub/
├── CMakeLists.txt              # ESP-IDF 메인 빌드 파일
├── sdkconfig                   # ESP-IDF 설정
├── main/
│   ├── CMakeLists.txt
│   ├── main.cpp (또는 main.c)
│   ├── component.mk
│   └── ...
└── components/
    ├── micro_ros_esp32/        # micro-ROS 컴포넌트
    ├── esp_now_handler/
    ├── lcd_driver/
    └── can_driver/
```

### 3. 코드 마이그레이션 필요 항목
- Arduino.h → ESP-IDF 헤더들
- Serial → UART 드라이버
- WiFi → esp_wifi
- ESP-NOW → esp_now
- FreeRTOS → ESP-IDF FreeRTOS (유사하지만 약간 다름)
- SPI (LCD) → ESP-IDF SPI 드라이버

### 4. micro-ROS 통합
- ESP-IDF 컴포넌트로 micro-ROS 추가
- colcon 빌드 시스템 사용

## 대안: 하이브리드 접근

### 옵션 A: ESP-IDF + Arduino as component
- ESP-IDF 프레임워크 사용
- Arduino를 ESP-IDF 컴포넌트로 추가
- 기존 Arduino 코드 대부분 유지 가능
- micro-ROS는 ESP-IDF 네이티브로 사용

### 옵션 B: 완전 ESP-IDF 네이티브
- 모든 코드를 ESP-IDF API로 재작성
- 최적의 성능과 안정성
- 작업량 많음

## 권장 방법
**옵션 A (하이브리드)를 권장합니다:**
- 빠른 전환 가능
- 기존 코드 재사용
- micro-ROS 안정적 사용
- 점진적 최적화 가능

## 필요한 도구
1. ESP-IDF 설치: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
2. micro-ROS for ESP32: https://github.com/micro-ROS/micro_ros_espidf_component

## 다음 단계
1. ESP-IDF 환경 설정 확인
2. 새 프로젝트 구조 생성
3. Arduino as ESP-IDF component 추가
4. micro-ROS 컴포넌트 추가
5. 기존 코드 통합
