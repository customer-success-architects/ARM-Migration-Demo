#!/bin/bash

echo "Building and running CPU Feature Detector in Docker..."
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker is not installed"
    echo "Please install Docker Desktop from https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Build the Docker image (auto-detects architecture)
echo "Building Docker image for native platform..."
docker build -t cpu-detector .

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
docker run --rm cpu-detector

echo ""
echo "Note: On macOS, GUI display requires XQuartz and additional setup."
