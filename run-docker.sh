#!/bin/bash

echo "Building and running x86 CPU Feature Detector in Docker..."
echo "This will run x86 container even on ARM Mac (via emulation)"
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker is not installed"
    echo "Please install Docker Desktop from https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Build the Docker image for x86/amd64 platform
echo "Building Docker image for linux/amd64 platform..."
docker build --platform linux/amd64 -t x86-cpu-detector .

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
docker run --platform linux/amd64 --rm x86-cpu-detector

echo ""
echo "Note: On macOS, GUI display requires XQuartz and additional setup."
echo "The application ran in x86 emulation mode via Docker."
