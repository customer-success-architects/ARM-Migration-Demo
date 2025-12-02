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
    
    printf("x86 CPU Feature Detector started\n");
    printf("This application is x86/x64 architecture only\n");
    
    gui.run();
    
    return 0;
}
