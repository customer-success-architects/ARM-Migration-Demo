#include "gui.h"
#include <cstdio>

// Architecture detection macros
#if defined(TARGET_ARCH_X86) || (!defined(TARGET_ARCH_ARM64) && !defined(TARGET_ARCH_ARM) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)))
#define CPU_ARCH_X86 1
#else
#define CPU_ARCH_ARM 1
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    GUI gui;
    
    if (!gui.initialize()) {
        fprintf(stderr, "Failed to initialize GUI\n");
        return 1;
    }
    
    printf("CPU Feature Detector started\n");
#ifdef CPU_ARCH_ARM
    printf("Running on ARM architecture\n");
#else
    printf("Running on x86/x64 architecture\n");
#endif
    
    gui.run();
    
    return 0;
}
