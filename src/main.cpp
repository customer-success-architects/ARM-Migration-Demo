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
    
#ifdef ARM_BUILD
    printf("ARM CPU Feature Detector started\n");
    printf("This application is built for ARM64/AArch64 architecture\n");
#else
    printf("x86 CPU Feature Detector started\n");
    printf("This application is x86/x64 architecture only\n");
#endif
    
    gui.run();
    
    return 0;
}
