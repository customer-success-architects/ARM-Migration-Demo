#include "gui.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    GUI gui;
    
    if (!gui.initialize()) {
        fprintf(stderr, "Failed to initialize GUI\n");
        return 1;
    }
    
#ifdef TARGET_ARCH_X86
    printf("CPU Feature Detector started (x86/x64 architecture)\n");
#elif defined(TARGET_ARCH_ARM)
    printf("CPU Feature Detector started (ARM architecture)\n");
#else
    printf("CPU Feature Detector started\n");
#endif
    
    gui.run();
    
    return 0;
}
