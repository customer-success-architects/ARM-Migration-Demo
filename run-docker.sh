#!/bin/bash

echo "Building and running ARM CPU Feature Detector in Docker..."
echo "This will run an ARM64 container natively on ARM systems"
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker is not installed"
    echo "Please install Docker Desktop from https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Build the Docker image for ARM64 platform
echo "Building Docker image for linux/arm64 platform..."
docker build --platform linux/arm64 -t arm-cpu-detector .

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
docker run --platform linux/arm64 --rm arm-cpu-detector

echo ""
echo "Note: On macOS, GUI display requires XQuartz and additional setup."
echo "The application ran natively on ARM64 architecture."
