#include "gui.h"
#include "arch_detect.h"
#include <cstdio>

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
