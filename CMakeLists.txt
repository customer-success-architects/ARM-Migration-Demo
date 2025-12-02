cmake_minimum_required(VERSION 3.15)
project(x86CPUDetector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure x86/x64 architecture only
if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(x86_64)")
    message(FATAL_ERROR "This project requires x86/x86-64 architecture. Current: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

message(STATUS "Building for x86/x64 architecture: ${CMAKE_SYSTEM_PROCESSOR}")

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
    # MSVC: Enable AVX2 and intrinsics
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang: Enable x86 features
    target_compile_options(${PROJECT_NAME} PRIVATE -march=native -Wall -Wextra)
    
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
