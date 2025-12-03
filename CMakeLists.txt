cmake_minimum_required(VERSION 3.15)
project(CPUDetector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(x86_64)")
    set(TARGET_ARCH "x86")
    message(STATUS "Building for x86/x64 architecture: ${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64)|(ARM64)|(arm64)|(armv8)")
    set(TARGET_ARCH "arm64")
    message(STATUS "Building for ARM64 architecture: ${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(arm)")
    set(TARGET_ARCH "arm")
    message(STATUS "Building for ARM architecture: ${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(STATUS "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    set(TARGET_ARCH "unknown")
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

# Add architecture-specific compile definitions
if(TARGET_ARCH STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE TARGET_ARCH_X86)
elseif(TARGET_ARCH STREQUAL "arm64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE TARGET_ARCH_ARM64)
elseif(TARGET_ARCH STREQUAL "arm")
    target_compile_definitions(${PROJECT_NAME} PRIVATE TARGET_ARCH_ARM)
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
    # MSVC: Enable AVX2 and intrinsics (x86 only)
    if(TARGET_ARCH STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang: Architecture-specific options
    if(TARGET_ARCH STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE -march=native -Wall -Wextra)
    elseif(TARGET_ARCH STREQUAL "arm64")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
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
