# CPU Feature Detector

A desktop application that detects and displays CPU capabilities. Supports both x86/x64 (using CPUID instruction) and ARM64/AArch64 architectures.

## Features

- **CPU Identification**: Vendor, brand, family, model, stepping (x86) / implementer, variant, part (ARM)
- **Instruction Set Detection**: 
  - x86: SSE, AVX, AVX2, AVX-512 support
  - ARM: NEON/ASIMD, SVE, SVE2, FP16, BF16, Dot Product
- **Cryptographic Features**: AES, SHA extensions
- **Cache Information**: L1/L2/L3 cache sizes and topology
- **Core Topology**: Physical cores and logical threads
- **Frequency Information**: Base and maximum CPU frequencies

## Architecture Support

This application supports both architectures:
- **x86/x86-64**: Uses CPUID instruction for feature detection
- **ARM64/AArch64**: Uses Linux HWCAP and sysfs for feature detection

## Building

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler (GCC, Clang, or MSVC)
- SDL2
- OpenGL

### macOS (Intel or Apple Silicon)

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
./ARMCPUDetector  # or ./x86CPUDetector on Intel
```

### Linux (x86 or ARM)

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

You can build and run the application in Docker for ARM64:

```bash
# Make sure Docker Desktop is installed and running
./run-docker.sh
```

### Alternative Docker Commands

```bash
# Build the image for ARM64
docker build --platform linux/arm64 -t arm-cpu-detector .

# Run the container
docker run --platform linux/arm64 --rm arm-cpu-detector

# Or use docker-compose
docker-compose up --build
```

**Note**: The GUI requires X11 forwarding, which works on Linux but requires XQuartz setup on macOS. The container will display CPU information in the terminal.

## License

MIT License
