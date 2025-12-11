#!/bin/bash
# Docker를 사용한 micro-ROS ESP32 빌드 스크립트

set -e

echo "==================================="
echo "Docker micro-ROS ESP32 빌드 시작"
echo "==================================="

# Docker 이미지 이름
IMAGE_NAME="esp32-microros-builder"
CONTAINER_NAME="esp32-microros-build"

# 현재 디렉토리 (프로젝트 루트)
PROJECT_DIR=$(pwd)

# Docker 이미지 빌드
echo ""
echo "[1/4] Docker 이미지 빌드 중..."
docker build -t $IMAGE_NAME .

# 기존 컨테이너가 있으면 제거
echo ""
echo "[2/4] 기존 컨테이너 정리 중..."
docker rm -f $CONTAINER_NAME 2>/dev/null || true

# Docker 컨테이너에서 빌드 실행
echo ""
echo "[3/4] PlatformIO 빌드 실행 중..."
echo "경고: 첫 빌드는 micro-ROS 의존성 다운로드로 인해 15-20분 소요됩니다."
docker run --rm \
    --name $CONTAINER_NAME \
    -v "$PROJECT_DIR:/workspace" \
    -w /workspace \
    $IMAGE_NAME \
    bash -c "pio run"

# 빌드 결과 확인
echo ""
echo "[4/4] 빌드 결과 확인..."
if [ -f ".pio/build/esp32dev/firmware.bin" ]; then
    echo ""
    echo "==================================="
    echo "✅ 빌드 성공!"
    echo "==================================="
    echo "펌웨어 위치: .pio/build/esp32dev/firmware.bin"
    ls -lh .pio/build/esp32dev/firmware.bin
else
    echo ""
    echo "==================================="
    echo "❌ 빌드 실패"
    echo "==================================="
    exit 1
fi
