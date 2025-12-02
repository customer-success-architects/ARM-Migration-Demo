# x86 CPU Feature Detector

A desktop application that detects and displays x86/x64 CPU capabilities using the CPUID instruction.

## Features

- **CPU Identification**: Vendor, brand, family, model, stepping
- **Instruction Set Detection**: SSE, AVX, AVX2, AVX-512 support
- **Cryptographic Features**: AES-NI, SHA extensions
- **Cache Information**: L1/L2/L3 cache sizes and topology
- **Core Topology**: Physical cores and logical threads
- **Frequency Information**: Base and maximum CPU frequencies

## Architecture Support

**x86/x86-64 ONLY** - This application is intentionally built for x86 architecture only and will not compile or run on ARM processors.

## Building

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler (GCC, Clang, or MSVC)
- SDL2
- OpenGL

### macOS

```bash
# Install dependencies
brew install cmake sdl2

# Clone ImGui (if not included)
mkdir -p external
cd external
git clone https://github.com/ocornut/imgui.git
cd ..

# Build
mkdir build && cd build
cmake ..
make
./x86CPUDetector
```

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install cmake libsdl2-dev libgl1-mesa-dev

# Clone ImGui
mkdir -p external
cd external
git clone https://github.com/ocornut/imgui.git
cd ..

# Build
mkdir build && cd build
cmake ..
make
./x86CPUDetector
```

### Windows

```bash
# Install dependencies via vcpkg
vcpkg install sdl2:x64-windows

# Clone ImGui
mkdir external
cd external
git clone https://github.com/ocornut/imgui.git
cd ..

# Build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Running with Docker (Recommended for ARM Macs)

If you're on Apple Silicon (ARM) or want to run in an isolated environment:

```bash
# Make sure Docker Desktop is installed and running
./run-docker.sh
```

This will build and run the x86 application in a Docker container using x86 emulation, even on ARM Macs.

### Alternative Docker Commands

```bash
# Build the image
docker build --platform linux/amd64 -t x86-cpu-detector .

# Run the container
docker run --platform linux/amd64 --rm x86-cpu-detector

# Or use docker-compose
docker-compose up --build
```

**Note**: The GUI requires X11 forwarding, which works on Linux but requires XQuartz setup on macOS. The container will display CPU information in the terminal.

## License

MIT License
