#!/bin/bash
# micro-ROS 빌드 스크립트 for WSL2
set -e

echo "======================================="
echo "micro-ROS WSL2 빌드 시작"
echo "======================================="
echo ""

# 프로젝트 디렉토리로 이동
cd /mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/components/micro_ros_espidf_component

echo "1단계: 이전 빌드 정리 중..."
# micro_ros_dev의 src는 유지 (CMakeLists.txt 수정 포함), build/install만 삭제
if [ -d "micro_ros_dev" ]; then
    rm -rf micro_ros_dev/build micro_ros_dev/install
    echo "micro_ros_dev: build/install 삭제됨"
fi
rm -rf libmicroros.a include esp32_toolchain.cmake

echo "2단계: ESP-IDF 환경 설정 중..."
export IDF_PATH="$HOME/esp/esp-idf"

if [ ! -d "$IDF_PATH" ]; then
    echo "오류: ESP-IDF를 찾을 수 없습니다: $IDF_PATH"
    exit 1
fi

# ESP-IDF export
echo "ESP-IDF 경로: $IDF_PATH"
. "$IDF_PATH/export.sh"

echo "2.5단계: Python 패키지 설치 중..."
# ESP-IDF Python 환경에 필요한 패키지 설치
pip3 install catkin_pkg lark-parser colcon-common-extensions

echo "3단계: 툴체인 경로 설정 중..."
# ESP-IDF의 툴체인 사용
TOOLCHAIN_DIR="$HOME/.espressif/tools/xtensa-esp-elf/esp-13.2.0_20230928/xtensa-esp-elf"
if [ ! -d "$TOOLCHAIN_DIR" ]; then
    echo "xtensa 툴체인을 찾을 수 없습니다: $TOOLCHAIN_DIR"
    exit 1
fi

export X_CC="${TOOLCHAIN_DIR}/bin/xtensa-esp32-elf-gcc"
export X_CXX="${TOOLCHAIN_DIR}/bin/xtensa-esp32-elf-g++"
export X_AR="${TOOLCHAIN_DIR}/bin/xtensa-esp32-elf-ar"
export X_STRIP="${TOOLCHAIN_DIR}/bin/xtensa-esp32-elf-strip"
export IDF_TARGET=esp32
export IDF_VERSION_MAJOR=5
export IDF_VERSION_MINOR=2

echo "툴체인: $TOOLCHAIN_DIR"
echo "컴파일러: $X_CC"

# 컴파일러 테스트
if [ ! -f "$X_CC" ]; then
    echo "컴파일러를 찾을 수 없습니다: $X_CC"
    exit 1
fi

echo "4단계: micro-ROS 빌드 시작 (15-20분 소요)..."

# rmw_test_fixture CMakeLists.txt 수정 (rmw 의존성 제거)
echo "4.1단계: rmw_test_fixture 패치 적용..."
python3 /mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/fix_rmw_test_fixture.py

# Makefile Hook: micro_ros_src 빌드 후 Micro-XRCE-DDS-Client 패치
echo "4.2단계: micro-ROS 빌드 시작..."
make -f libmicroros.mk || {
    echo "빌드 실패. Micro-XRCE-DDS-Client 패치 적용 후 재시도..."
    if [ -f "micro_ros_src/src/Micro-XRCE-DDS-Client/CMakeLists.txt" ]; then
        cd micro_ros_src/src/Micro-XRCE-DDS-Client
        git checkout CMakeLists.txt
        patch -p1 < /mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/components/micro_ros_espidf_component/fix_microxrcedds.patch
        cd ../../..
        rm -rf micro_ros_src/build/microxrcedds_client
        make -f libmicroros.mk
    fi
}

if [ -f "libmicroros.a" ]; then
    echo ""
    echo "======================================="
    echo "빌드 성공!"
    echo "======================================="
    echo ""
    ls -lh libmicroros.a
    ls -d include/
else
    echo ""
    echo "======================================="
    echo "빌드 실패!"
    echo "======================================="
    exit 1
fi
