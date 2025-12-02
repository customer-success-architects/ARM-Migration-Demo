#include "gui.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <cstdio>

GUI::GUI() : cpu_info_(std::make_unique<CPUInfo>()) {}

GUI::~GUI() {
    shutdown();
}

bool GUI::initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow("CPU Feature Detector", 
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                1280, 720, window_flags);
    if (!window_) {
        printf("Error creating window: %s\n", SDL_GetError());
        return false;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void GUI::run() {
    bool done = false;
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.12f, 1.00f);

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && 
                event.window.windowID == SDL_GetWindowID(window_))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        render();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window_);
    }
}

void GUI::shutdown() {
    if (gl_context_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(gl_context_);
        gl_context_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    SDL_Quit();
}

void GUI::render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::Begin("CPU Feature Detector", nullptr, window_flags);
    
#ifdef __aarch64__
    ImGui::Text("ARM64 CPU Information");
#else
    ImGui::Text("x86/x64 CPU Information");
#endif
    ImGui::Separator();
    
    if (ImGui::BeginTabBar("CPUTabs")) {
        if (ImGui::BeginTabItem("Processor Info")) {
            renderProcessorInfo();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Features")) {
            renderFeatures();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Cache & Topology")) {
            renderCacheInfo();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void GUI::renderProcessorInfo() {
    const auto& info = cpu_info_->getProcessorInfo();
    
    ImGui::Spacing();
    ImGui::Text("Vendor:        %s", info.vendor.c_str());
    ImGui::Text("Brand:         %s", info.brand.c_str());
    ImGui::Separator();
    
#ifdef __aarch64__
    if (!info.implementer.empty()) {
        ImGui::Text("Implementer:   %s", info.implementer.c_str());
    }
    if (!info.architecture.empty()) {
        ImGui::Text("Architecture:  %s", info.architecture.c_str());
    }
    if (!info.variant.empty()) {
        ImGui::Text("Variant:       %s", info.variant.c_str());
    }
    if (!info.part.empty()) {
        ImGui::Text("Part:          %s", info.part.c_str());
    }
    if (!info.revision.empty()) {
        ImGui::Text("Revision:      %s", info.revision.c_str());
    }
#else
    ImGui::Text("Family:        %u", info.family);
    ImGui::Text("Model:         %u", info.model);
    ImGui::Text("Stepping:      %u", info.stepping);
#endif
    ImGui::Separator();
    
    ImGui::Text("Physical Cores: %u", info.physical_cores);
    ImGui::Text("Logical Cores:  %u", info.logical_cores);
    ImGui::Separator();
    
    if (info.base_frequency_mhz > 0) {
        ImGui::Text("Base Frequency:  %u MHz", info.base_frequency_mhz);
        ImGui::Text("Max Frequency:   %u MHz", info.max_frequency_mhz);
    } else {
        ImGui::Text("Frequency information not available");
    }
}

void GUI::renderFeatures() {
    const auto& features = cpu_info_->getFeatures();
    
    ImGui::Spacing();
    
#ifdef __aarch64__
    // ARM SIMD Instructions
    if (ImGui::CollapsingHeader("ARM SIMD Instructions", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        ImGui::BeginTable("ARM_SIMD", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("FP", (bool*)&features.fp);
        ImGui::TableNextColumn(); ImGui::Checkbox("ASIMD (NEON)", (bool*)&features.asimd);
        ImGui::TableNextColumn(); ImGui::Checkbox("FP16", (bool*)&features.fp16);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("BF16", (bool*)&features.bf16);
        ImGui::TableNextColumn(); ImGui::Checkbox("Dot Product", (bool*)&features.dotprod);
        ImGui::TableNextColumn(); ImGui::Checkbox("I8MM", (bool*)&features.i8mm);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("RDM", (bool*)&features.rdm);
        ImGui::TableNextColumn(); ImGui::Checkbox("FCMA", (bool*)&features.fcma);
        ImGui::TableNextColumn(); ImGui::Checkbox("JSCVT", (bool*)&features.jscvt);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("FRINT", (bool*)&features.frint);
        ImGui::EndTable();
        
        ImGui::Text("Scalable Vector Extensions:");
        ImGui::BeginTable("SVE", 2);
        ImGui::TableNextColumn(); ImGui::Checkbox("SVE", (bool*)&features.sve);
        ImGui::TableNextColumn(); ImGui::Checkbox("SVE2", (bool*)&features.sve2);
        ImGui::TableNextColumn(); ImGui::Checkbox("SVE AES", (bool*)&features.sve_aes);
        ImGui::TableNextColumn(); ImGui::Checkbox("SVE PMULL", (bool*)&features.sve_pmull);
        ImGui::EndTable();
        
        ImGui::Unindent();
    }
    
    // Cryptographic Features
    if (ImGui::CollapsingHeader("Cryptographic Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Crypto", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("AES", (bool*)&features.aes);
        ImGui::TableNextColumn(); ImGui::Checkbox("PMULL", (bool*)&features.pmull);
        ImGui::TableNextColumn(); ImGui::Checkbox("SHA1", (bool*)&features.sha1);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("SHA2", (bool*)&features.sha2);
        ImGui::TableNextColumn(); ImGui::Checkbox("SHA3", (bool*)&features.sha3);
        ImGui::TableNextColumn(); ImGui::Checkbox("SHA512", (bool*)&features.sha512);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("SM3", (bool*)&features.sm3);
        ImGui::TableNextColumn(); ImGui::Checkbox("SM4", (bool*)&features.sm4);
        ImGui::TableNextColumn(); ImGui::Checkbox("CRC32", (bool*)&features.crc32);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // Atomic & Memory Features
    if (ImGui::CollapsingHeader("Atomic & Memory Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Atomic", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("LSE", (bool*)&features.lse);
        ImGui::TableNextColumn(); ImGui::Checkbox("LSE2", (bool*)&features.lse2);
        ImGui::TableNextColumn(); ImGui::Checkbox("DCPOP", (bool*)&features.dcpop);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // Security Features
    if (ImGui::CollapsingHeader("Security Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Security", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("Pointer Auth", (bool*)&features.pauth);
        ImGui::TableNextColumn(); ImGui::Checkbox("BTI", (bool*)&features.bti);
        ImGui::TableNextColumn(); ImGui::Checkbox("MTE", (bool*)&features.mte);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
#else
    // x86 SIMD Instructions
    if (ImGui::CollapsingHeader("SIMD Instructions", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        ImGui::BeginTable("SIMD", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("x87 FPU", (bool*)&features.x87_fpu);
        ImGui::TableNextColumn(); ImGui::Checkbox("MMX", (bool*)&features.mmx);
        ImGui::TableNextColumn(); ImGui::Checkbox("SSE", (bool*)&features.sse);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("SSE2", (bool*)&features.sse2);
        ImGui::TableNextColumn(); ImGui::Checkbox("SSE3", (bool*)&features.sse3);
        ImGui::TableNextColumn(); ImGui::Checkbox("SSSE3", (bool*)&features.ssse3);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("SSE4.1", (bool*)&features.sse4_1);
        ImGui::TableNextColumn(); ImGui::Checkbox("SSE4.2", (bool*)&features.sse4_2);
        ImGui::TableNextColumn(); ImGui::Checkbox("AVX", (bool*)&features.avx);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("AVX2", (bool*)&features.avx2);
        ImGui::TableNextColumn(); ImGui::Checkbox("FMA", (bool*)&features.fma);
        ImGui::TableNextColumn(); ImGui::Checkbox("FMA4", (bool*)&features.fma4);
        ImGui::EndTable();
        
        ImGui::Text("AVX-512 Extensions:");
        ImGui::BeginTable("AVX512", 2);
        ImGui::TableNextColumn(); ImGui::Checkbox("AVX-512 F", (bool*)&features.avx512f);
        ImGui::TableNextColumn(); ImGui::Checkbox("AVX-512 DQ", (bool*)&features.avx512dq);
        ImGui::TableNextColumn(); ImGui::Checkbox("AVX-512 BW", (bool*)&features.avx512bw);
        ImGui::TableNextColumn(); ImGui::Checkbox("AVX-512 VL", (bool*)&features.avx512vl);
        ImGui::EndTable();
        
        ImGui::Unindent();
    }
    
    // Cryptographic Features
    if (ImGui::CollapsingHeader("Cryptographic Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Crypto", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("AES-NI", (bool*)&features.aes);
        ImGui::TableNextColumn(); ImGui::Checkbox("SHA", (bool*)&features.sha);
        ImGui::TableNextColumn(); ImGui::Checkbox("PCLMULQDQ", (bool*)&features.pclmulqdq);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // Virtualization
    if (ImGui::CollapsingHeader("Virtualization & Security", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("VirtSec", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("Intel VT-x", (bool*)&features.vmx);
        ImGui::TableNextColumn(); ImGui::Checkbox("AMD-V", (bool*)&features.svm);
        ImGui::TableNextColumn(); ImGui::Checkbox("NX Bit", (bool*)&features.nx);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("SMEP", (bool*)&features.smep);
        ImGui::TableNextColumn(); ImGui::Checkbox("SMAP", (bool*)&features.smap);
        ImGui::TableNextColumn(); ImGui::Checkbox("SGX", (bool*)&features.sgx);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // Other Features
    if (ImGui::CollapsingHeader("Other Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Other", 3);
        ImGui::TableNextColumn(); ImGui::Checkbox("TSC", (bool*)&features.tsc);
        ImGui::TableNextColumn(); ImGui::Checkbox("RDRAND", (bool*)&features.rdrand);
        ImGui::TableNextColumn(); ImGui::Checkbox("RDSEED", (bool*)&features.rdseed);
        
        ImGui::TableNextColumn(); ImGui::Checkbox("POPCNT", (bool*)&features.popcnt);
        ImGui::TableNextColumn(); ImGui::Checkbox("BMI1", (bool*)&features.bmi1);
        ImGui::TableNextColumn(); ImGui::Checkbox("BMI2", (bool*)&features.bmi2);
        ImGui::EndTable();
        ImGui::Unindent();
    }
#endif
}

void GUI::renderCacheInfo() {
    const auto& cache = cpu_info_->getCacheInfo();
    const auto& info = cpu_info_->getProcessorInfo();
    
    ImGui::Spacing();
    
    if (ImGui::CollapsingHeader("Cache Hierarchy", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        if (cache.l1_data_size > 0) {
            ImGui::Text("L1 Data Cache:        %u KB", cache.l1_data_size);
        }
        if (cache.l1_instruction_size > 0) {
            ImGui::Text("L1 Instruction Cache: %u KB", cache.l1_instruction_size);
        }
        if (cache.l2_size > 0) {
            ImGui::Text("L2 Cache:             %u KB", cache.l2_size);
        }
        if (cache.l3_size > 0) {
            ImGui::Text("L3 Cache:             %u KB (%.2f MB)", cache.l3_size, cache.l3_size / 1024.0f);
        }
        if (cache.cache_line_size > 0) {
            ImGui::Text("Cache Line Size:      %u bytes", cache.cache_line_size);
        }
        
        if (cache.l1_data_size == 0 && cache.l2_size == 0 && cache.l3_size == 0) {
            ImGui::Text("Cache information not available");
        }
        
        ImGui::Unindent();
    }
    
    if (ImGui::CollapsingHeader("Core Topology", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        ImGui::Text("Physical Cores: %u", info.physical_cores);
        ImGui::Text("Logical Cores:  %u", info.logical_cores);
        
        if (info.logical_cores > info.physical_cores) {
            ImGui::Text("Hyperthreading: Enabled (%u threads per core)", 
                       info.logical_cores / info.physical_cores);
        } else {
            ImGui::Text("Hyperthreading: Not detected");
        }
        
        ImGui::Unindent();
    }
}
