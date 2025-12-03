#!/bin/bash

echo "Building and running CPU Feature Detector in Docker..."
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker is not installed"
    echo "Please install Docker Desktop from https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Detect architecture
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
    PLATFORM="linux/arm64"
    echo "Detected ARM64 architecture"
else
    PLATFORM="linux/amd64"
    echo "Detected x86_64 architecture"
fi

# Allow user to override platform
if [ "$1" = "--arm64" ]; then
    PLATFORM="linux/arm64"
    echo "Forcing ARM64 platform"
elif [ "$1" = "--amd64" ]; then
    PLATFORM="linux/amd64"
    echo "Forcing AMD64 platform"
fi

echo "Building Docker image for $PLATFORM platform..."
docker build --platform $PLATFORM -t cpu-detector .

if [ $? -ne 0 ]; then
    echo "ERROR: Docker build failed"
    exit 1
fi

echo ""
echo "Build successful! Running container..."
echo ""

# Run the container
docker run --platform $PLATFORM --rm cpu-detector

echo ""
echo "Note: On macOS, GUI display requires XQuartz and additional setup."
