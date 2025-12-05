# CPU Feature Detector

A cross-platform desktop application that detects and displays CPU capabilities. Supports both x86/x64 (using CPUID instruction) and ARM64 (using HWCAP and sysfs) architectures.

## Features

### Common Features (All Architectures)
- **CPU Identification**: Vendor, brand/model identification
- **Cache Information**: L1/L2/L3 cache sizes and cache line size
- **Core Topology**: Physical cores and logical threads
- **Frequency Information**: Base and maximum CPU frequencies

### x86/x64 Specific Features
- **SIMD Instructions**: SSE, SSE2-4, AVX, AVX2, AVX-512 support
- **Cryptographic Features**: AES-NI, SHA extensions, PCLMULQDQ
- **Virtualization**: Intel VT-x, AMD-V support
- **Security Features**: NX bit, SMEP, SMAP, SGX

### ARM64 Specific Features
- **SIMD Instructions**: NEON (ASIMD), SVE, SVE2 support
- **Cryptographic Features**: AES, SHA1/2/3/512, CRC32, PMULL
- **Security Features**: BTI, MTE, Pointer Authentication (PAC)
- **Floating Point**: FP, FP16, BF16, I8MM (matrix multiply)

## Architecture Support

This application supports both **x86/x86-64** and **ARM64 (AArch64)** architectures with native CPU feature detection for each platform.

## Building

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler (GCC, Clang, or MSVC)
- SDL2
- OpenGL

### Quick Setup

```bash
# Run the setup script (handles dependencies and ImGui)
./setup.sh

# Build
mkdir build && cd build
cmake ..
make
./CPUDetector
```

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
./CPUDetector
```

### Linux (x86_64 or ARM64)

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
./CPUDetector
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

Docker builds automatically detect the native architecture:

```bash
# Run the docker script (auto-detects architecture)
./run-docker.sh
```

### Alternative Docker Commands

```bash
# Build for native architecture
docker build -t cpu-detector .

# Run the container
docker run --rm cpu-detector

# Or use docker-compose (supports multi-arch)
docker-compose up --build

# Build for specific architecture (cross-platform)
docker build --platform linux/arm64 -t cpu-detector .
docker build --platform linux/amd64 -t cpu-detector .
```

**Note**: The GUI requires X11 forwarding, which works on Linux but requires XQuartz setup on macOS. The container will display CPU information in the terminal.

## License

MIT License
