# micro-ROS Docker Build Script for Windows
# This script builds micro-ROS library using Docker

Write-Host "Starting micro-ROS Docker build..." -ForegroundColor Green

# Check if Docker is running
try {
    docker ps | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Docker is not running. Please start Docker Desktop." -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "Docker is not available. Please install Docker Desktop." -ForegroundColor Red
    exit 1
}

# Get project directory
$PROJECT_DIR = (Get-Location).Path
Write-Host "Project directory: $PROJECT_DIR" -ForegroundColor Cyan

# Pull micro-ROS Docker image
Write-Host "`nPulling micro-ROS Docker image..." -ForegroundColor Yellow
docker pull microros/esp-idf-microros:humble

# Run micro-ROS build in Docker
Write-Host "`nBuilding micro-ROS library in Docker..." -ForegroundColor Yellow
docker run --rm `
    -v "${PROJECT_DIR}:/project" `
    -w /project `
    microros/esp-idf-microros:humble `
    bash -c "cd components/micro_ros_espidf_component && . /opt/esp/idf/export.sh && make -f libmicroros.mk clean && make -f libmicroros.mk"

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nmicro-ROS build completed successfully!" -ForegroundColor Green
} else {
    Write-Host "`nmicro-ROS build failed!" -ForegroundColor Red
    exit 1
}
