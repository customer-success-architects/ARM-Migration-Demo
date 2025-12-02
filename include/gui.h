#pragma once

#include "cpu_info.h"
#include <memory>

struct SDL_Window;
typedef void *SDL_GLContext;

class GUI {
public:
    GUI();
    ~GUI();
    
    bool initialize();
    void run();
    void shutdown();

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    std::unique_ptr<CPUInfo> cpu_info_;
    
    void render();
    void renderProcessorInfo();
    void renderFeatures();
    void renderCacheInfo();
};
