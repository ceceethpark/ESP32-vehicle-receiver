# Complete micro-ROS library build script for Windows
# This script builds micro-ROS inside Docker and packages the library

Write-Host "=======================================" -ForegroundColor Cyan
Write-Host "micro-ROS Library Builder for ESP32" -ForegroundColor Cyan
Write-Host "=======================================" -ForegroundColor Cyan
Write-Host ""

# Check Docker
if (!(Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Host "Error: Docker is not installed!" -ForegroundColor Red
    exit 1
}

Write-Host "Step 1: Cleaning previous builds..." -ForegroundColor Yellow
docker run --rm -v "${PWD}:/project" `
    -w /project/components/micro_ros_espidf_component `
    espressif/idf:release-v5.2 `
    bash -c "rm -rf micro_ros_dev micro_ros_src libmicroros.a include esp32_toolchain.cmake"

Write-Host "Step 2: Building micro-ROS (this will take 15-20 minutes)..." -ForegroundColor Yellow
Write-Host "Please be patient..." -ForegroundColor Yellow
Write-Host ""

$buildScript = @'
#!/bin/bash
set -e

# Source ESP-IDF environment
. /opt/esp/idf/export.sh > /dev/null 2>&1

# Install Python dependencies
pip3 install catkin_pkg lark-parser colcon-common-extensions > /dev/null 2>&1

# Set toolchain variables
export TOOLCHAIN_PREFIX=/opt/esp/tools/xtensa-esp-elf/esp-13.2.0_20250707/xtensa-esp-elf
export X_CC=${TOOLCHAIN_PREFIX}/bin/xtensa-esp32-elf-gcc
export X_CXX=${TOOLCHAIN_PREFIX}/bin/xtensa-esp32-elf-g++
export X_AR=${TOOLCHAIN_PREFIX}/bin/xtensa-esp32-elf-ar
export X_STRIP=${TOOLCHAIN_PREFIX}/bin/xtensa-esp32-elf-strip
export IDF_TARGET=esp32
export IDF_PATH=/opt/esp/idf
export IDF_VERSION_MAJOR=5
export IDF_VERSION_MINOR=2

# Build
cd /project/components/micro_ros_espidf_component
make -f libmicroros.mk

echo ""
echo "Build completed successfully!"
ls -lh libmicroros.a
'@

$buildScript | docker run --rm -i -v "${PWD}:/project" `
    -w /project/components/micro_ros_espidf_component `
    espressif/idf:release-v5.2 `
    bash

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "=======================================" -ForegroundColor Green
    Write-Host "SUCCESS! micro-ROS library built!" -ForegroundColor Green
    Write-Host "=======================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Library location: components\micro_ros_espidf_component\libmicroros.a" -ForegroundColor Cyan
    Write-Host "Headers location: components\micro_ros_espidf_component\include\" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "=======================================" -ForegroundColor Red
    Write-Host "BUILD FAILED!" -ForegroundColor Red
    Write-Host "=======================================" -ForegroundColor Red
    exit 1
}
