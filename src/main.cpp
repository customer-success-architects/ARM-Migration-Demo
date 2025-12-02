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
    
#ifdef __aarch64__
    printf("ARM64 CPU Feature Detector started\n");
    printf("Running on ARM architecture\n");
#else
    printf("x86/x64 CPU Feature Detector started\n");
    printf("Running on x86/x64 architecture\n");
#endif
    
    gui.run();
    
    return 0;
}
