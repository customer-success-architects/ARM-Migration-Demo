# CPU Feature Detector

A cross-platform desktop application that detects and displays CPU capabilities. Supports both **x86/x64** and **ARM64** architectures.

## Features

- **CPU Identification**: Vendor, brand, family, model, stepping
- **Instruction Set Detection**: 
  - x86: SSE, AVX, AVX2, AVX-512 support
  - ARM: NEON, SVE, SVE2 support
- **Cryptographic Features**: AES, SHA extensions (architecture-specific)
- **Cache Information**: L1/L2/L3 cache sizes and topology
- **Core Topology**: Physical cores and logical threads
- **Frequency Information**: Base and maximum CPU frequencies

## Architecture Support

This application supports both architectures:
- **x86/x86-64**: Uses CPUID instruction for feature detection
- **ARM64/AArch64**: Uses Linux hwcap and sysfs for feature detection

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

### Linux (x86 or ARM64)

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

For containerized deployment on ARM64:

```bash
# Build and run for ARM64
docker build --platform linux/arm64 -t cpu-detector .
docker run --platform linux/arm64 --rm cpu-detector

# Or use docker-compose
docker-compose up --build
```

**Note**: The GUI requires X11 forwarding, which works on Linux but requires XQuartz setup on macOS. The container will display CPU information in the terminal.

## Architecture-Specific Features

### x86/x64 Features Detected
- SIMD: MMX, SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, AVX, AVX2, AVX-512
- Crypto: AES-NI, SHA, PCLMULQDQ
- Virtualization: VT-x, AMD-V
- Security: NX, SMEP, SMAP, SGX

### ARM64 Features Detected
- SIMD: NEON (ASIMD), SVE, SVE2, Dot Product, Int8 Matrix Multiply
- Floating Point: FP, FP16, BF16, FCMA, FRINT
- Crypto: AES, SHA-1, SHA-2, SHA-3, SHA-512, PMULL
- Memory: Atomics (LSE), CRC32
- Security: BTI, MTE, SB, SSBS

## License

MIT License
