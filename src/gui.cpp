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
    
    const auto& info = cpu_info_->getProcessorInfo();
    ImGui::Text("%s CPU Information", info.architecture.c_str());
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
    ImGui::Text("Architecture:  %s", info.architecture.c_str());
    ImGui::Text("Vendor:        %s", info.vendor.c_str());
    ImGui::Text("Brand:         %s", info.brand.c_str());
    ImGui::Separator();
    
#ifdef TARGET_ARCH_ARM
    ImGui::Text("Implementer:   0x%02X", info.implementer);
    ImGui::Text("Variant:       0x%X", info.variant);
    ImGui::Text("Part:          0x%03X", info.part);
    ImGui::Text("Revision:      %u", info.revision);
#else
    ImGui::Text("Family:        %u", info.family);
    ImGui::Text("Model:         %u", info.model);
    ImGui::Text("Stepping:      %u", info.stepping);
#endif
    ImGui::Separator();
    
    ImGui::Text("Physical Cores: %u", info.physical_cores);
    ImGui::Text("Logical Cores:  %u", info.logical_cores);
    ImGui::Separator();
    
    if (info.base_frequency_mhz > 0 || info.max_frequency_mhz > 0) {
        if (info.base_frequency_mhz > 0)
            ImGui::Text("Base Frequency:  %u MHz", info.base_frequency_mhz);
        if (info.max_frequency_mhz > 0)
            ImGui::Text("Max Frequency:   %u MHz", info.max_frequency_mhz);
    } else {
        ImGui::Text("Frequency information not available");
    }
}

// Helper function to display a feature checkbox safely (read-only display)
static void FeatureCheckbox(const char* label, bool value) {
    bool temp = value;
    ImGui::BeginDisabled();
    ImGui::Checkbox(label, &temp);
    ImGui::EndDisabled();
}

void GUI::renderFeatures() {
    const auto& features = cpu_info_->getFeatures();
    
    ImGui::Spacing();
    
#ifdef TARGET_ARCH_X86
    // x86 SIMD Instructions
    if (ImGui::CollapsingHeader("SIMD Instructions", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        ImGui::BeginTable("SIMD", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("x87 FPU", features.x87_fpu);
        ImGui::TableNextColumn(); FeatureCheckbox("MMX", features.mmx);
        ImGui::TableNextColumn(); FeatureCheckbox("SSE", features.sse);
        
        ImGui::TableNextColumn(); FeatureCheckbox("SSE2", features.sse2);
        ImGui::TableNextColumn(); FeatureCheckbox("SSE3", features.sse3);
        ImGui::TableNextColumn(); FeatureCheckbox("SSSE3", features.ssse3);
        
        ImGui::TableNextColumn(); FeatureCheckbox("SSE4.1", features.sse4_1);
        ImGui::TableNextColumn(); FeatureCheckbox("SSE4.2", features.sse4_2);
        ImGui::TableNextColumn(); FeatureCheckbox("AVX", features.avx);
        
        ImGui::TableNextColumn(); FeatureCheckbox("AVX2", features.avx2);
        ImGui::TableNextColumn(); FeatureCheckbox("FMA", features.fma);
        ImGui::TableNextColumn(); FeatureCheckbox("FMA4", features.fma4);
        ImGui::EndTable();
        
        ImGui::Text("AVX-512 Extensions:");
        ImGui::BeginTable("AVX512", 2);
        ImGui::TableNextColumn(); FeatureCheckbox("AVX-512 F", features.avx512f);
        ImGui::TableNextColumn(); FeatureCheckbox("AVX-512 DQ", features.avx512dq);
        ImGui::TableNextColumn(); FeatureCheckbox("AVX-512 BW", features.avx512bw);
        ImGui::TableNextColumn(); FeatureCheckbox("AVX-512 VL", features.avx512vl);
        ImGui::EndTable();
        
        ImGui::Unindent();
    }
    
    // x86 Cryptographic Features
    if (ImGui::CollapsingHeader("Cryptographic Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Crypto", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("AES-NI", features.aes);
        ImGui::TableNextColumn(); FeatureCheckbox("SHA", features.sha);
        ImGui::TableNextColumn(); FeatureCheckbox("PCLMULQDQ", features.pclmulqdq);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // x86 Virtualization
    if (ImGui::CollapsingHeader("Virtualization & Security", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("VirtSec", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("Intel VT-x", features.vmx);
        ImGui::TableNextColumn(); FeatureCheckbox("AMD-V", features.svm);
        ImGui::TableNextColumn(); FeatureCheckbox("NX Bit", features.nx);
        
        ImGui::TableNextColumn(); FeatureCheckbox("SMEP", features.smep);
        ImGui::TableNextColumn(); FeatureCheckbox("SMAP", features.smap);
        ImGui::TableNextColumn(); FeatureCheckbox("SGX", features.sgx);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // x86 Other Features
    if (ImGui::CollapsingHeader("Other Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Other", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("TSC", features.tsc);
        ImGui::TableNextColumn(); FeatureCheckbox("RDRAND", features.rdrand);
        ImGui::TableNextColumn(); FeatureCheckbox("RDSEED", features.rdseed);
        
        ImGui::TableNextColumn(); FeatureCheckbox("POPCNT", features.popcnt);
        ImGui::TableNextColumn(); FeatureCheckbox("BMI1", features.bmi1);
        ImGui::TableNextColumn(); FeatureCheckbox("BMI2", features.bmi2);
        ImGui::EndTable();
        ImGui::Unindent();
    }

#elif defined(TARGET_ARCH_ARM)
    // ARM SIMD Instructions
    if (ImGui::CollapsingHeader("SIMD Instructions (NEON/SVE)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        ImGui::BeginTable("SIMD", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("NEON (ASIMD)", features.neon);
        ImGui::TableNextColumn(); FeatureCheckbox("NEON FP16", features.neon_fp16);
        ImGui::TableNextColumn(); FeatureCheckbox("NEON DotProd", features.neon_dotprod);
        
        ImGui::TableNextColumn(); FeatureCheckbox("SVE", features.sve);
        ImGui::TableNextColumn(); FeatureCheckbox("SVE2", features.sve2);
        ImGui::TableNextColumn(); FeatureCheckbox("I8MM", features.i8mm);
        ImGui::EndTable();
        
        ImGui::Unindent();
    }
    
    // ARM Floating Point
    if (ImGui::CollapsingHeader("Floating Point", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("FP", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("FP", features.fp);
        ImGui::TableNextColumn(); FeatureCheckbox("FP16", features.fp16);
        ImGui::TableNextColumn(); FeatureCheckbox("BF16", features.bf16);
        ImGui::TableNextColumn(); FeatureCheckbox("FRINT", features.frint);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // ARM Cryptographic Features
    if (ImGui::CollapsingHeader("Cryptographic Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Crypto", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("AES", features.aes);
        ImGui::TableNextColumn(); FeatureCheckbox("SHA1", features.sha1);
        ImGui::TableNextColumn(); FeatureCheckbox("SHA2", features.sha2);
        
        ImGui::TableNextColumn(); FeatureCheckbox("SHA3", features.sha3);
        ImGui::TableNextColumn(); FeatureCheckbox("SHA512", features.sha512);
        ImGui::TableNextColumn(); FeatureCheckbox("CRC32", features.crc32);
        
        ImGui::TableNextColumn(); FeatureCheckbox("PMULL", features.pmull);
        ImGui::TableNextColumn(); FeatureCheckbox("RNG", features.rng);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // ARM Security Features
    if (ImGui::CollapsingHeader("Security Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Security", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("BTI", features.bti);
        ImGui::TableNextColumn(); FeatureCheckbox("MTE", features.mte);
        ImGui::TableNextColumn(); FeatureCheckbox("PAC-A", features.paca);
        
        ImGui::TableNextColumn(); FeatureCheckbox("PAC-G", features.pacg);
        ImGui::TableNextColumn(); FeatureCheckbox("SSBS", features.ssbs);
        ImGui::TableNextColumn(); FeatureCheckbox("SB", features.sb);
        ImGui::EndTable();
        ImGui::Unindent();
    }
    
    // ARM Other Features
    if (ImGui::CollapsingHeader("Other Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::BeginTable("Other", 3);
        ImGui::TableNextColumn(); FeatureCheckbox("Atomics", features.atomics);
        ImGui::TableNextColumn(); FeatureCheckbox("DCPOP", features.dcpop);
        ImGui::TableNextColumn(); FeatureCheckbox("DCPODP", features.dcpodp);
        
        ImGui::TableNextColumn(); FeatureCheckbox("FLAGM", features.flagm);
        ImGui::EndTable();
        ImGui::Unindent();
    }

#else
    // Generic fallback
    ImGui::Text("Feature detection not available for this architecture");
    ImGui::BeginTable("Generic", 2);
    ImGui::TableNextColumn(); FeatureCheckbox("SIMD", features.simd);
    ImGui::TableNextColumn(); FeatureCheckbox("Crypto", features.crypto);
    ImGui::EndTable();
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
#ifdef TARGET_ARCH_X86
            ImGui::Text("Hyperthreading: Enabled (%u threads per core)", 
                       info.logical_cores / info.physical_cores);
#elif defined(TARGET_ARCH_ARM)
            ImGui::Text("SMT: Enabled (%u threads per core)", 
                       info.logical_cores / info.physical_cores);
#else
            ImGui::Text("SMT: Enabled (%u threads per core)", 
                       info.logical_cores / info.physical_cores);
#endif
        } else {
#ifdef TARGET_ARCH_X86
            ImGui::Text("Hyperthreading: Not detected");
#else
            ImGui::Text("SMT: Not detected");
#endif
        }
        
        ImGui::Unindent();
    }
}
