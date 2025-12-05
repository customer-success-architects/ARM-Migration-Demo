# Use ARM64 Ubuntu base image
FROM --platform=linux/arm64 ubuntu:22.04

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libsdl2-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    xvfb \
    x11-apps \
    mesa-utils \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Clone Dear ImGui
RUN mkdir -p external && \
    cd external && \
    git clone --depth 1 https://github.com/ocornut/imgui.git

# Build the application
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make

# Set display for X11 forwarding (if needed)
ENV DISPLAY=:99

# Default command
CMD ["./build/ARMCPUDetector"]
