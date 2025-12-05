#!/bin/bash

echo "Building and running CPU Feature Detector in Docker..."
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker is not installed"
    echo "Please install Docker Desktop from https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Detect current architecture
ARCH=$(uname -m)
if [[ "$ARCH" == "x86_64" || "$ARCH" == "amd64" ]]; then
    PLATFORM="linux/amd64"
    echo "Building for x86_64 architecture..."
elif [[ "$ARCH" == "aarch64" || "$ARCH" == "arm64" ]]; then
    PLATFORM="linux/arm64"
    echo "Building for ARM64 architecture..."
else
    echo "Detected architecture: $ARCH"
    echo "Building for native architecture..."
    PLATFORM=""
fi

# Build the Docker image
echo "Building Docker image..."
if [ -n "$PLATFORM" ]; then
    docker build --platform "$PLATFORM" -t cpu-detector .
else
    docker build -t cpu-detector .
fi

if [ $? -ne 0 ]; then
    echo "ERROR: Docker build failed"
    exit 1
fi

echo ""
echo "Build successful! Running container..."
echo ""

# Run the container
# Note: GUI won't work on macOS without additional X11 setup
# This will run and show CPU info in terminal output
if [ -n "$PLATFORM" ]; then
    docker run --platform "$PLATFORM" --rm cpu-detector
else
    docker run --rm cpu-detector
fi

echo ""
echo "Note: On macOS, GUI display requires XQuartz and additional setup."
