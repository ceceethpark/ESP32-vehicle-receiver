# micro-ROS 빌드 문제 해결 기록

**날짜**: 2025-12-11  
**프로젝트**: ESP32 Micro Hub - micro-ROS 통합  
**환경**: WSL2 Ubuntu-22.04, ESP-IDF v5.2, micro-ROS kilted branch

---

## 빌드 단계 구조

### 1단계: micro_ros_dev (빌드 도구)
- **패키지 수**: 약 60개
- **내용**: ament_cmake, ament_package, rosidl_generator 등
- **목적**: 2단계에서 사용할 빌드 시스템 준비
- **비유**: 집을 짓기 전 공구 준비

### 2단계: micro_ros_src (라이브러리)
- **패키지 수**: 75개 (C++ 패키지 15개 제외)
- **내용**: rcl, rmw, microxrcedds_client 등
- **목적**: ESP32용 libmicroros.a 생성
- **비유**: 준비된 공구로 실제 집 건설

---

## 문제 1: VERSION/SOVERSION CMake 오류

### 증상
```
CMake Error at CMakeLists.txt:320 (set_target_properties):
  set_target_properties called with incorrect arguments.
  VERSION not allowed unless CMP0048 is set to NEW
```

### 원인
`Micro-XRCE-DDS-Client/CMakeLists.txt` 321-324줄:
```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION
        ${PROJECT_VERSION}              # ← 정의되지 않은 변수!
    SOVERSION
        ${PROJECT_VERSION_MAJOR}        # ← 정의되지 않은 변수!
    C_STANDARD
        ${UCLIENT_C_STANDARD}
    ...
)
```

- `${PROJECT_VERSION}` 변수가 정의되지 않아 빈 값으로 전달됨
- CMake는 `VERSION` 키워드 뒤에 값이 없으면 오류 발생

### 해결 방법
**321-324줄 완전 삭제**:
```bash
cd /mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/components/micro_ros_espidf_component/micro_ros_src/src/Micro-XRCE-DDS-Client
sed -i '321,324d' CMakeLists.txt
```

**수정 후 구조** (319-331줄):
```cmake
set_common_compile_options(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD
        ${UCLIENT_C_STANDARD}
    C_STANDARD_REQUIRED
        YES
    POSITION_INDEPENDENT_CODE
        ${UCLIENT_PIC}
    )
```

### 자동화
`libmicroros.mk` 86줄 뒤에 추가:
```makefile
sed -i '321,324d' src/Micro-XRCE-DDS-Client/CMakeLists.txt || true;
```
- `rm -rf micro_ros_src`로 소스 재클론 시 자동 패치 적용

---

## 문제 2: POSIX 소켓 API 오류

### 증상
```
fatal error: arpa/inet.h: No such file or directory
fatal error: sys/socket.h: No such file or directory
fatal error: termios.h: No such file or directory
```

### 원인
- **SERIAL 프로파일**: Linux termios API 사용 (ESP32 FreeRTOS에 없음)
- **UDP 프로파일**: POSIX 소켓 API 사용 (arpa/inet.h, sys/socket.h)
- ESP32는 FreeRTOS 기반으로 Linux/POSIX API 없음

### 해결 방법
**colcon.meta 수정** (`components/micro_ros_espidf_component/colcon.meta`):

#### microxrcedds_client 설정:
```json
"microxrcedds_client": {
    "cmake-args": [
        "-DUCLIENT_PROFILE_DISCOVERY=OFF",
        "-DUCLIENT_PROFILE_UDP=OFF",              // ON → OFF
        "-DUCLIENT_PROFILE_TCP=OFF",
        "-DUCLIENT_PROFILE_SERIAL=OFF",           // ON → OFF
        "-DUCLIENT_PROFILE_CUSTOM_TRANSPORT=ON"   // 활성화
    ]
}
```

#### rmw_microxrcedds 설정:
```json
"rmw_microxrcedds": {
    "cmake-args": [
        "-DRMW_UXRCE_TRANSPORT=custom",           // udp → custom
        ...
    ]
}
```

### CUSTOM_TRANSPORT 구현 (ESP32 main.c)
```c
#include <uxr/client/transport.h>
#include "driver/uart.h"

// 3개 함수 구현 필요 (약 30줄)
bool uart_open(uxrCustomTransport* transport) {
    uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 17, 16, -1, -1); // TX=17, RX=16
    uart_driver_install(UART_NUM_1, 2048, 2048, 0, NULL, 0);
    return true;
}

size_t uart_write(uxrCustomTransport* transport, const uint8_t* buf, 
                  size_t len, uint8_t* err) {
    int written = uart_write_bytes(UART_NUM_1, (const char*)buf, len);
    return written > 0 ? written : 0;
}

size_t uart_read(uxrCustomTransport* transport, uint8_t* buf, size_t len, 
                 int timeout, uint8_t* err) {
    int read = uart_read_bytes(UART_NUM_1, buf, len, 
                               pdMS_TO_TICKS(timeout));
    return read > 0 ? read : 0;
}

// micro-ROS 초기화 시
rmw_uros_set_custom_transport(
    true,  // framing
    NULL,  // args
    uart_open,
    NULL,  // close
    uart_write,
    uart_read
);
```

---

## 문제 3: 빌드 스크립트 자동화

### 문제점
- `libmicroros.mk` 50줄: `rm -rf micro_ros_src` → 수동 패치 삭제됨
- 매번 소스 재클론 시 수동으로 패치 재적용 필요

### 해결 방법
**libmicroros.mk 수정** (86줄 뒤):
```makefile
git clone -b $(MICROROS_BRANCH) $(MICROROS_REPO) src/micro_ros_espidf_component; \
cd src/micro_ros_espidf_component; \
git submodule update --init --recursive --depth 1; \
# 자동 패치 추가
sed -i '321,324d' src/Micro-XRCE-DDS-Client/CMakeLists.txt || true; \
```

---

## 빌드 명령어

### WSL2에서 실행
```bash
# 환경 변수 설정
COMPONENT_DIR="/mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/components/micro_ros_espidf_component"

# micro_ros_src로 이동
cd $COMPONENT_DIR/micro_ros_src

# colcon 빌드 (C++ 패키지 제외, 75개 패키지)
colcon build \
    --merge-install \
    --packages-ignore-regex '.*_cpp' \
    --metas $COMPONENT_DIR/colcon.meta \
    --cmake-args \
        -DCMAKE_TOOLCHAIN_FILE=$COMPONENT_DIR/esp32_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF
```

### 결과 확인
```bash
# 빌드 완료 메시지
Summary: 75 packages finished

# 라이브러리 확인
ls -lh install/lib/libmicroros.a  # 예상 크기: 2-4MB

# 패키지 카운트
ls install/lib/*.a | wc -l  # 약 60+ 개의 .a 파일
```

---

## 메모리 사용량 계산

### ESP32-WROOM-32 (320KB RAM)
```
기본 사용량:
- FreeRTOS 커널: 20-30KB
- ESP-NOW: 15-20KB
- CAN 드라이버: 8-10KB
- 기타 드라이버: 10-15KB
= 약 53-75KB

micro-ROS 추가:
- rcl/rclc 런타임: 40-50KB
- DDS 버퍼: 30-40KB
- UART 버퍼: 4KB
= 약 74-94KB

센서 추가 (계획):
- LiDAR: 20-25KB
- Encoder: 3-5KB
- LCD 없이: 0KB (프레임버퍼 제외)
= 약 23-30KB

총 사용량: 150-199KB
여유 공간: 121-170KB (38-53%)
```

**결론**: ESP32-WROOM-32로 충분, 업그레이드 불필요

---

## 참고 사항

### SERIAL vs CUSTOM_TRANSPORT
| 항목 | SERIAL 프로파일 | CUSTOM_TRANSPORT |
|------|----------------|------------------|
| **OS 요구사항** | Linux/POSIX | Any (FreeRTOS 포함) |
| **API** | termios (Linux 전용) | 사용자 정의 |
| **구현 복잡도** | 자동 (0줄) | 3개 함수 (약 30줄) |
| **ESP32 호환** | ❌ 불가능 | ✅ 가능 |

### micro-ROS Agent (PC 측)
```bash
# ROS2 Humble 설치 필요
sudo apt install ros-humble-micro-ros-agent

# Agent 실행
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 921600

# 토픽 확인
ros2 topic list
ros2 topic echo /espnow/data
```

### 빌드 시간
- **1단계** (micro_ros_dev): 약 5-10분
- **2단계** (micro_ros_src): 약 10-15분
- **총 소요 시간**: 15-25분

### 재빌드 불필요
- `libmicroros.a`는 재사용 가능
- ESP32 코드 수정 시 WSL2 재빌드 불필요
- Windows PlatformIO에서 직접 빌드/플래시

---

## 현재 진행 상황 (2025-12-11)

✅ **완료**:
- ESP-IDF 5.5.0 마이그레이션
- ESP-NOW 콜백 업데이트 (wifi_tx_info_t)
- WSL2 환경 구축 (ESP-IDF v5.2)
- micro_ros_dev 빌드 (60개 패키지)
- VERSION/SOVERSION 패치 적용
- POSIX transport 비활성화
- CUSTOM_TRANSPORT 활성화
- micro_ros_src 빌드: 75/75 패키지 완료 (58분 소요)
- libmicroros.a 생성 (18MB)
- micro-ROS 코드 통합 (main.c)
- ESP32 빌드 성공

**최종 빌드 결과**:
- RAM: 13.3% (43,504 / 327,680 bytes)
- Flash: 76.7% (804,477 / 1,048,576 bytes)

⏳ **다음 단계**:
1. ESP32에 펌웨어 플래시
2. micro-ROS Agent 연결 테스트
3. ESP-NOW 리모컨 데이터 ROS2 토픽 발행 확인
4. LiDAR, Encoder, LCD, CAN 추가
