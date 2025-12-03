# CPU Feature Detector

A cross-platform desktop application that detects and displays CPU capabilities. Supports both x86/x64 (using CPUID instruction) and ARM/ARM64 architectures.

## Features

- **CPU Identification**: Vendor, brand, family, model, stepping
- **Instruction Set Detection**: 
  - x86: SSE, AVX, AVX2, AVX-512 support
  - ARM: NEON, SVE, SVE2 support
- **Cryptographic Features**: AES, SHA extensions
- **Cache Information**: L1/L2/L3 cache sizes and topology
- **Core Topology**: Physical cores and logical threads
- **Frequency Information**: Base and maximum CPU frequencies

## Architecture Support

- **x86/x86-64**: Full support with CPUID-based feature detection
- **ARM64 (AArch64)**: Full support with hwcap-based feature detection including NEON, SVE, cryptographic extensions
- **ARM (AArch32)**: Basic support

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
./CPUDetector
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

You can run the application in Docker on any architecture:

```bash
# Make sure Docker Desktop is installed and running
./run-docker.sh
```

### Alternative Docker Commands

```bash
# Build the image (auto-detects architecture)
docker build -t cpu-detector .

# Run the container
docker run --rm cpu-detector

# Or use docker-compose
docker-compose up --build
```

**Note**: The GUI requires X11 forwarding, which works on Linux but requires XQuartz setup on macOS. The container will display CPU information in the terminal.

## ARM-Specific Features Detected

When running on ARM64, the application detects:
- **SIMD**: NEON/Advanced SIMD, SVE, SVE2, Dot Product, BFloat16
- **Cryptography**: AES, SHA-1, SHA-2, SHA-3, SHA-512, CRC32, PMULL
- **Atomics**: Large System Extensions (LSE)
- **Other**: Half-precision FP, Int8 Matrix Multiply, JavaScript conversion

## License

MIT License
