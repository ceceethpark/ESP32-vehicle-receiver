#!/bin/bash
# Docker 컨테이너 내부에서 실행되는 빌드 스크립트

set -e

echo "====================================="
echo "프로젝트를 컨테이너 내부로 복사 중..."
echo "====================================="

# Windows 파일시스템의 CRLF 문제를 피하기 위해 컨테이너 내부로 복사
# .pio 디렉토리는 제외 (빌드 캐시는 새로 생성)
rsync -a --exclude='.pio' --exclude='.git' /workspace/ /tmp/build/
cd /tmp/build

echo ""
echo "====================================="
echo "줄바꿈 변환 중..."
echo "====================================="

# 셸 스크립트 파일들의 줄바꿈을 LF로 변환
find . -name "*.sh" -type f -exec dos2unix {} \; 2>/dev/null || true

echo ""
echo "====================================="
echo "PlatformIO 빌드 시작..."
echo "====================================="

# 빌드 실행
pio run

echo ""
echo "====================================="
echo "빌드 결과를 원본 경로로 복사 중..."
echo "====================================="

# 빌드 결과를 다시 Windows 파일시스템으로 복사
if [ -f ".pio/build/esp32dev/firmware.bin" ]; then
    mkdir -p /workspace/.pio/build/esp32dev/
    cp -r .pio/build/esp32dev/* /workspace/.pio/build/esp32dev/
    echo "✅ 빌드 성공!"
    ls -lh /workspace/.pio/build/esp32dev/firmware.bin
else
    echo "❌ 빌드 실패"
    exit 1
fi
