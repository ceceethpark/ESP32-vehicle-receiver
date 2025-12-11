# micro-ROS ESP32 빌드를 위한 Docker 이미지
FROM ubuntu:22.04

# 비대화형 설치를 위한 환경 변수
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Seoul

# 기본 패키지 및 빌드 도구 설치
RUN apt-get update && apt-get install -y \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    python3-venv \
    build-essential \
    cmake \
    ninja-build \
    ccache \
    libusb-1.0-0-dev \
    udev \
    dos2unix \
    rsync \
    && rm -rf /var/lib/apt/lists/*

# PlatformIO 설치
RUN pip3 install --upgrade pip && \
    pip3 install platformio

# 작업 디렉토리 설정
WORKDIR /workspace

# PlatformIO 초기 설정 (캐시 생성)
RUN pio system info

# micro-ROS가 필요로 하는 Python 가상환경 생성
RUN python3 -m venv /root/.platformio/penv

# 가상환경에 필요한 패키지 설치
RUN /root/.platformio/penv/bin/pip install --upgrade pip && \
    /root/.platformio/penv/bin/pip install catkin-pkg lark-parser empy importlib-resources pyyaml

# 빌드 스크립트 복사를 위한 준비
VOLUME ["/workspace"]

# 기본 명령어
CMD ["bash"]
