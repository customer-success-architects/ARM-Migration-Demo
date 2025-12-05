cmake_minimum_required(VERSION 3.15)
project(CPUDetector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Support both ARM and x86 architectures
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64)|(arm64)|(ARM64)")
    set(ARM_BUILD ON)
    message(STATUS "Building for ARM64 architecture: ${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(x86_64)")
    set(ARM_BUILD OFF)
    message(STATUS "Building for x86/x64 architecture: ${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(STATUS "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}, assuming ARM")
    set(ARM_BUILD ON)
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/cpu_info.cpp
    src/gui.cpp
)

# ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Add compile definition for ARM builds
if(ARM_BUILD)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ARM_BUILD=1)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    ${SDL2_LIBRARIES}
)

# Platform-specific settings
if(MSVC)
    # MSVC: Enable platform-specific features
    if(ARM_BUILD)
        # ARM on Windows (not common but possible)
        target_compile_options(${PROJECT_NAME} PRIVATE)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang: Enable platform-specific features
    if(ARM_BUILD)
        # ARM64: Enable NEON SIMD (enabled by default on ARM64)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    else()
        # x86: Enable native optimizations
        target_compile_options(${PROJECT_NAME} PRIVATE -march=native -Wall -Wextra)
    endif()
    
    # Link dl for Linux
    if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE dl)
    endif()
endif()

# macOS specific
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
endif()

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
