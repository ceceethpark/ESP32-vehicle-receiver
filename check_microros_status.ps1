# micro-ROS 빌드 상태 모니터링 스크립트
Write-Host "=======================================" -ForegroundColor Cyan
Write-Host "micro-ROS 빌드 상태 확인" -ForegroundColor Cyan
Write-Host "=======================================" -ForegroundColor Cyan
Write-Host ""

# 1. WSL colcon 프로세스 확인
Write-Host "1. WSL 빌드 프로세스 확인..." -ForegroundColor Yellow
$colconProcess = wsl ps aux | Select-String "colcon build"
if ($colconProcess) {
    Write-Host "   ✓ colcon 빌드 진행 중" -ForegroundColor Green
    Write-Host "   프로세스: $colconProcess" -ForegroundColor Gray
} else {
    Write-Host "   ✗ colcon 프로세스 없음 (완료되었거나 실행되지 않음)" -ForegroundColor Red
}

Write-Host ""

# 2. micro_ros_dev 빌드 상태
Write-Host "2. micro_ros_dev 디렉토리 확인..." -ForegroundColor Yellow
$devInstall = Test-Path "components\micro_ros_espidf_component\micro_ros_dev\install"
if ($devInstall) {
    Write-Host "   ✓ micro_ros_dev/install 존재" -ForegroundColor Green
} else {
    Write-Host "   ✗ micro_ros_dev/install 없음" -ForegroundColor Red
}

Write-Host ""

# 3. micro_ros_src 빌드 상태
Write-Host "3. micro_ros_src 디렉토리 확인..." -ForegroundColor Yellow
$srcInstall = Test-Path "components\micro_ros_espidf_component\micro_ros_src\install"
if ($srcInstall) {
    Write-Host "   ✓ micro_ros_src/install 존재" -ForegroundColor Green
    $libCount = (Get-ChildItem "components\micro_ros_espidf_component\micro_ros_src\install\lib" -Filter *.a -ErrorAction SilentlyContinue).Count
    Write-Host "   라이브러리 파일 수: $libCount" -ForegroundColor Gray
} else {
    Write-Host "   ✗ micro_ros_src/install 없음 (아직 빌드 안됨)" -ForegroundColor Yellow
}

Write-Host ""

# 4. 최종 라이브러리 확인
Write-Host "4. 최종 libmicroros.a 확인..." -ForegroundColor Yellow
$libMicroros = Test-Path "components\micro_ros_espidf_component\libmicroros.a"
if ($libMicroros) {
    $libSize = (Get-Item "components\micro_ros_espidf_component\libmicroros.a").Length
    $libSizeMB = [math]::Round($libSize / 1MB, 2)
    Write-Host "   ✓ libmicroros.a 존재 ($libSizeMB MB)" -ForegroundColor Green
    Write-Host ""
    Write-Host "   ========================================" -ForegroundColor Green
    Write-Host "   빌드 완료! micro-ROS 사용 준비됨" -ForegroundColor Green
    Write-Host "   ========================================" -ForegroundColor Green
} else {
    Write-Host "   ✗ libmicroros.a 없음 (빌드 진행 중 또는 실패)" -ForegroundColor Yellow
}

Write-Host ""

# 5. include 디렉토리 확인
Write-Host "5. include 디렉토리 확인..." -ForegroundColor Yellow
$includeDir = Test-Path "components\micro_ros_espidf_component\include"
if ($includeDir) {
    $headerCount = (Get-ChildItem "components\micro_ros_espidf_component\include" -Recurse -Filter *.h -ErrorAction SilentlyContinue).Count
    Write-Host "   ✓ include 디렉토리 존재 (헤더 파일: $headerCount 개)" -ForegroundColor Green
} else {
    Write-Host "   ✗ include 디렉토리 없음" -ForegroundColor Red
}

Write-Host ""
Write-Host "=======================================" -ForegroundColor Cyan

# 빌드 완료 여부 판단
if ($libMicroros -and $includeDir) {
    Write-Host "상태: 빌드 완료 ✓" -ForegroundColor Green
    Write-Host ""
    Write-Host "다음 단계:" -ForegroundColor Yellow
    Write-Host "  1. pio run  # WSL2에서 ESP32 프로젝트 빌드" -ForegroundColor White
    Write-Host "  2. pio run -t upload  # ESP32에 펌웨어 업로드" -ForegroundColor White
} elseif ($colconProcess) {
    Write-Host "상태: 빌드 진행 중... (5-10분 소요)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "이 스크립트를 다시 실행하여 상태를 확인하세요:" -ForegroundColor Cyan
    Write-Host "  .\check_microros_status.ps1" -ForegroundColor White
} else {
    Write-Host "상태: 빌드 대기 중 또는 실패" -ForegroundColor Red
    Write-Host ""
    Write-Host "빌드를 시작하려면:" -ForegroundColor Yellow
    Write-Host "  wsl bash /mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/build_microros_wsl2.sh" -ForegroundColor White
}

Write-Host "=======================================" -ForegroundColor Cyan
