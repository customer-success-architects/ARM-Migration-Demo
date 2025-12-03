#pragma once

// Common architecture detection macros
// These should be defined by CMake via target_compile_definitions, but we also
// provide fallback detection based on compiler-defined macros

#if defined(TARGET_ARCH_X86)
    // Already defined by CMake
    #define CPU_ARCH_X86 1
#elif defined(TARGET_ARCH_ARM64)
    // Already defined by CMake
    #define CPU_ARCH_ARM 1
#elif defined(TARGET_ARCH_ARM)
    // Already defined by CMake
    #define CPU_ARCH_ARM 1
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    // Fallback: detect x86 from compiler
    #define CPU_ARCH_X86 1
#elif defined(__aarch64__) || defined(__arm__)
    // Fallback: detect ARM from compiler
    #define CPU_ARCH_ARM 1
#else
    // Unknown architecture - default to generic
    #define CPU_ARCH_UNKNOWN 1
#endif
