#!/bin/bash

echo "Setting up x86 CPU Feature Detector..."

# Check if running on x86/x64 architecture
ARCH=$(uname -m)
if [[ "$ARCH" != "x86_64" && "$ARCH" != "i386" && "$ARCH" != "i686" ]]; then
    echo "ERROR: This project requires x86/x64 architecture"
    echo "Current architecture: $ARCH"
    exit 1
fi

echo "Architecture check passed: $ARCH"

# Clone ImGui if not present
if [ ! -d "external/imgui" ]; then
    echo "Cloning Dear ImGui..."
    mkdir -p external
    cd external
    git clone --depth 1 https://github.com/ocornut/imgui.git
    cd ..
else
    echo "Dear ImGui already present"
fi

# Install dependencies based on OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "macOS detected - checking dependencies..."
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Please install from https://brew.sh"
        exit 1
    fi
    
    echo "Installing dependencies via Homebrew..."
    brew install cmake sdl2
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Linux detected - checking dependencies..."
    
    if command -v apt-get &> /dev/null; then
        echo "Installing dependencies via apt-get..."
        sudo apt-get update
        sudo apt-get install -y cmake libsdl2-dev libgl1-mesa-dev
    elif command -v yum &> /dev/null; then
        echo "Installing dependencies via yum..."
        sudo yum install -y cmake SDL2-devel mesa-libGL-devel
    else
        echo "Please install cmake, SDL2, and OpenGL development libraries manually"
        exit 1
    fi
else
    echo "Unsupported OS: $OSTYPE"
    echo "Please install cmake, SDL2, and OpenGL manually"
fi

echo ""
echo "Setup complete! To build:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"
echo "  ./x86CPUDetector"
