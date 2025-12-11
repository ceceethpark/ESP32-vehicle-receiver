# Docker를 사용한 micro-ROS ESP32 빌드 스크립트 (Windows PowerShell용)

Write-Host "===================================" -ForegroundColor Cyan
Write-Host "Docker micro-ROS ESP32 빌드 시작" -ForegroundColor Cyan
Write-Host "===================================" -ForegroundColor Cyan

# Docker 이미지 이름
$IMAGE_NAME = "esp32-microros-builder"
$CONTAINER_NAME = "esp32-microros-build"

# 현재 디렉토리 (프로젝트 루트)
$PROJECT_DIR = (Get-Location).Path

# Docker가 설치되어 있는지 확인
Write-Host ""
Write-Host "[0/4] Docker 설치 확인 중..." -ForegroundColor Yellow
try {
    docker --version | Out-Null
    Write-Host "✓ Docker 설치 확인됨" -ForegroundColor Green
} catch {
    Write-Host "❌ Docker가 설치되어 있지 않습니다." -ForegroundColor Red
    Write-Host "Docker Desktop을 설치하세요: https://www.docker.com/products/docker-desktop" -ForegroundColor Yellow
    exit 1
}

# Docker 이미지 빌드
Write-Host ""
Write-Host "[1/4] Docker 이미지 빌드 중..." -ForegroundColor Yellow
docker build -t $IMAGE_NAME .
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Docker 이미지 빌드 실패" -ForegroundColor Red
    exit 1
}

# 기존 컨테이너가 있으면 제거
Write-Host ""
Write-Host "[2/4] 기존 컨테이너 정리 중..." -ForegroundColor Yellow
docker rm -f $CONTAINER_NAME 2>$null

# Docker 컨테이너에서 빌드 실행
Write-Host ""
Write-Host "[3/4] PlatformIO 빌드 실행 중..." -ForegroundColor Yellow
Write-Host "경고: 첫 빌드는 micro-ROS 의존성 다운로드로 인해 15-20분 소요됩니다." -ForegroundColor Yellow

# Windows 경로를 Linux 경로로 변환 (Docker Desktop용)
$LINUX_PATH = "/workspace"

docker run --rm `
    --name $CONTAINER_NAME `
    -v "${PROJECT_DIR}:${LINUX_PATH}" `
    -w $LINUX_PATH `
    $IMAGE_NAME `
    bash -c "pio run"

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "===================================" -ForegroundColor Red
    Write-Host "❌ 빌드 실패" -ForegroundColor Red
    Write-Host "===================================" -ForegroundColor Red
    exit 1
}

# 빌드 결과 확인
Write-Host ""
Write-Host "[4/4] 빌드 결과 확인..." -ForegroundColor Yellow
$FIRMWARE_PATH = ".pio\build\esp32dev\firmware.bin"

if (Test-Path $FIRMWARE_PATH) {
    Write-Host ""
    Write-Host "===================================" -ForegroundColor Green
    Write-Host "✅ 빌드 성공!" -ForegroundColor Green
    Write-Host "===================================" -ForegroundColor Green
    Write-Host "펌웨어 위치: $FIRMWARE_PATH" -ForegroundColor Cyan
    
    $firmware = Get-Item $FIRMWARE_PATH
    Write-Host "파일 크기: $($firmware.Length / 1KB) KB" -ForegroundColor Cyan
    Write-Host "생성 시간: $($firmware.LastWriteTime)" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "===================================" -ForegroundColor Red
    Write-Host "❌ 펌웨어 파일을 찾을 수 없습니다" -ForegroundColor Red
    Write-Host "===================================" -ForegroundColor Red
    exit 1
}
