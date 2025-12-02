# CPU Feature Detector

A cross-platform desktop application that detects and displays CPU capabilities for both ARM64 and x86/x64 architectures.

## Features

### ARM64 Architecture
- **CPU Identification**: Vendor, brand, implementer, architecture version
- **SIMD Instructions**: NEON (ASIMD), FP16, BF16, Dot Product, SVE, SVE2
- **Cryptographic Features**: AES, SHA1/2/3/512, SM3/4, CRC32
- **Atomic Operations**: LSE, LSE2, DCPOP
- **Security Features**: Pointer Authentication, BTI, MTE
- **Cache Information**: L1/L2/L3 cache sizes and topology
- **Core Topology**: Physical and logical core detection

### x86/x64 Architecture
- **CPU Identification**: Vendor, brand, family, model, stepping
- **Instruction Set Detection**: SSE, AVX, AVX2, AVX-512 support
- **Cryptographic Features**: AES-NI, SHA extensions
- **Cache Information**: L1/L2/L3 cache sizes and topology
- **Core Topology**: Physical cores and logical threads
- **Frequency Information**: Base and maximum CPU frequencies

## Architecture Support

This application now supports **both ARM64 and x86/x64 architectures**. It will automatically detect the CPU architecture at compile time and runtime, displaying the appropriate features for each platform.

- **ARM64**: Apple Silicon (M1/M2/M3), AWS Graviton, Ampere Altra, and other ARM processors
- **x86/x64**: Intel and AMD processors

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
./ARMCPUDetector
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
./ARMCPUDetector
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

## Running with Docker

The application can run in Docker on both ARM64 and x86/x64 hosts:

```bash
# Make sure Docker Desktop is installed and running
./run-docker.sh
```

This will build and run the application in a Docker container, automatically detecting your system's architecture.

### Alternative Docker Commands

```bash
# Build the image (automatically detects architecture)
docker build -t cpu-detector .

# Run the container
docker run --rm cpu-detector

# Or use docker-compose
docker-compose up --build
```

### Multi-Architecture Support

The Docker image now supports both ARM64 and x86/x64 architectures. When you build the image, it will automatically use the appropriate base image for your system:

- On Apple Silicon Macs (M1/M2/M3): Builds for ARM64
- On Intel/AMD systems: Builds for x86/x64

**Note**: The GUI requires X11 forwarding, which works on Linux but requires XQuartz setup on macOS. The container will display CPU information in the terminal.

## License

MIT License
