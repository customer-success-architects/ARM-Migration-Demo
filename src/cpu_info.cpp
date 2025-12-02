#include "cpu_info.h"
#include <cstring>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

CPUInfo::CPUInfo() {
    detect();
}

void CPUInfo::cpuid(uint32_t leaf, uint32_t subleaf, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx) {
#ifdef _MSC_VER
    int cpu_info[4];
    __cpuidex(cpu_info, leaf, subleaf);
    eax = cpu_info[0];
    ebx = cpu_info[1];
    ecx = cpu_info[2];
    edx = cpu_info[3];
#else
    __cpuid_count(leaf, subleaf, eax, ebx, ecx, edx);
#endif
}

void CPUInfo::detect() {
    uint32_t eax, ebx, ecx, edx;
    
    // Get maximum basic and extended leaves
    cpuid(0, 0, eax, ebx, ecx, edx);
    max_basic_leaf_ = eax;
    
    cpuid(0x80000000, 0, eax, ebx, ecx, edx);
    max_extended_leaf_ = eax;
    
    detectVendor();
    detectBrand();
    detectFeatures();
    detectCacheInfo();
    detectTopology();
    detectFrequency();
}

void CPUInfo::detectVendor() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, 0, eax, ebx, ecx, edx);
    
    char vendor[13] = {0};
    std::memcpy(vendor, &ebx, 4);
    std::memcpy(vendor + 4, &edx, 4);
    std::memcpy(vendor + 8, &ecx, 4);
    
    processor_info_.vendor = vendor;
}

void CPUInfo::detectBrand() {
    if (max_extended_leaf_ < 0x80000004) {
        processor_info_.brand = "Unknown";
        return;
    }
    
    char brand[49] = {0};
    uint32_t eax, ebx, ecx, edx;
    
    for (uint32_t i = 0; i < 3; i++) {
        cpuid(0x80000002 + i, 0, eax, ebx, ecx, edx);
        std::memcpy(brand + i * 16, &eax, 4);
        std::memcpy(brand + i * 16 + 4, &ebx, 4);
        std::memcpy(brand + i * 16 + 8, &ecx, 4);
        std::memcpy(brand + i * 16 + 12, &edx, 4);
    }
    
    // Trim leading spaces
    const char* start = brand;
    while (*start == ' ') start++;
    
    processor_info_.brand = start;
}

void CPUInfo::detectFeatures() {
    uint32_t eax, ebx, ecx, edx;
    
    // Leaf 1: Basic features and processor info
    if (max_basic_leaf_ >= 1) {
        cpuid(1, 0, eax, ebx, ecx, edx);
        
        // Extract family, model, stepping
        processor_info_.stepping = eax & 0xF;
        processor_info_.model = (eax >> 4) & 0xF;
        processor_info_.family = (eax >> 8) & 0xF;
        
        // Extended model and family
        if (processor_info_.family == 0xF) {
            processor_info_.family += (eax >> 20) & 0xFF;
        }
        if (processor_info_.family == 0x6 || processor_info_.family == 0xF) {
            processor_info_.model += ((eax >> 16) & 0xF) << 4;
        }
        
        // EDX features
        features_.x87_fpu = edx & (1 << 0);
        features_.tsc = edx & (1 << 4);
        features_.mmx = edx & (1 << 23);
        features_.sse = edx & (1 << 25);
        features_.sse2 = edx & (1 << 26);
        
        // ECX features
        features_.sse3 = ecx & (1 << 0);
        features_.pclmulqdq = ecx & (1 << 1);
        features_.ssse3 = ecx & (1 << 9);
        features_.fma = ecx & (1 << 12);
        features_.sse4_1 = ecx & (1 << 19);
        features_.sse4_2 = ecx & (1 << 20);
        features_.popcnt = ecx & (1 << 23);
        features_.aes = ecx & (1 << 25);
        features_.avx = ecx & (1 << 28);
        features_.rdrand = ecx & (1 << 30);
        
        // Virtualization
        features_.vmx = ecx & (1 << 5);  // Intel VT-x
    }
    
    // Leaf 7: Extended features
    if (max_basic_leaf_ >= 7) {
        cpuid(7, 0, eax, ebx, ecx, edx);
        
        // EBX features
        features_.sgx = ebx & (1 << 2);
        features_.bmi1 = ebx & (1 << 3);
        features_.avx2 = ebx & (1 << 5);
        features_.smep = ebx & (1 << 7);
        features_.bmi2 = ebx & (1 << 8);
        features_.avx512f = ebx & (1 << 16);
        features_.avx512dq = ebx & (1 << 17);
        features_.rdseed = ebx & (1 << 18);
        features_.avx512bw = ebx & (1 << 30);
        features_.avx512vl = ebx & (1 << 31);
        
        // ECX features
        features_.sha = ebx & (1 << 29);
        
        // EDX features
        features_.smap = edx & (1 << 20);
    }
    
    // Extended leaf 0x80000001: AMD-specific and NX bit
    if (max_extended_leaf_ >= 0x80000001) {
        cpuid(0x80000001, 0, eax, ebx, ecx, edx);
        
        features_.nx = edx & (1 << 20);  // NX bit
        features_.svm = ecx & (1 << 2);  // AMD-V
        features_.fma4 = ecx & (1 << 16); // AMD FMA4
    }
}

void CPUInfo::detectCacheInfo() {
    if (max_basic_leaf_ < 4) {
        return;
    }
    
    uint32_t eax, ebx, ecx, edx;
    
    // Iterate through cache levels using leaf 4
    for (uint32_t i = 0; i < 10; i++) {
        cpuid(4, i, eax, ebx, ecx, edx);
        
        uint32_t cache_type = eax & 0x1F;
        if (cache_type == 0) break; // No more caches
        
        uint32_t cache_level = (eax >> 5) & 0x7;
        uint32_t line_size = (ebx & 0xFFF) + 1;
        uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
        uint32_t sets = ecx + 1;
        
        uint32_t cache_size_bytes = ways * partitions * line_size * sets;
        uint32_t cache_size_kb = cache_size_bytes / 1024;
        
        if (cache_line_size == 0) {
            cache_info_.cache_line_size = line_size;
        }
        
        if (cache_level == 1) {
            if (cache_type == 1) { // Data cache
                cache_info_.l1_data_size = cache_size_kb;
            } else if (cache_type == 2) { // Instruction cache
                cache_info_.l1_instruction_size = cache_size_kb;
            }
        } else if (cache_level == 2) {
            cache_info_.l2_size = cache_size_kb;
        } else if (cache_level == 3) {
            cache_info_.l3_size = cache_size_kb;
        }
    }
}

void CPUInfo::detectTopology() {
    uint32_t eax, ebx, ecx, edx;
    
    // Try leaf 0xB for topology (modern Intel CPUs)
    if (max_basic_leaf_ >= 0xB) {
        cpuid(0xB, 0, eax, ebx, ecx, edx);
        if (ebx != 0) {
            // Thread level
            uint32_t thread_mask_width = eax & 0x1F;
            
            cpuid(0xB, 1, eax, ebx, ecx, edx);
            // Core level
            uint32_t core_mask_width = eax & 0x1F;
            
            cpuid(1, 0, eax, ebx, ecx, edx);
            uint32_t logical_per_package = (ebx >> 16) & 0xFF;
            
            processor_info_.logical_cores = logical_per_package;
            processor_info_.physical_cores = logical_per_package >> (thread_mask_width);
            
            if (processor_info_.physical_cores == 0) {
                processor_info_.physical_cores = processor_info_.logical_cores;
            }
            
            return;
        }
    }
    
    // Fallback: Use leaf 1
    if (max_basic_leaf_ >= 1) {
        cpuid(1, 0, eax, ebx, ecx, edx);
        processor_info_.logical_cores = (ebx >> 16) & 0xFF;
        
        // Assume no hyperthreading if we can't detect properly
        processor_info_.physical_cores = processor_info_.logical_cores;
        
        // Check for HT
        if (edx & (1 << 28)) {
            // Hyperthreading is supported, assume 2 threads per core
            processor_info_.physical_cores = processor_info_.logical_cores / 2;
        }
    }
}

void CPUInfo::detectFrequency() {
    if (max_basic_leaf_ < 0x16) {
        return;
    }
    
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x16, 0, eax, ebx, ecx, edx);
    
    processor_info_.base_frequency_mhz = eax & 0xFFFF;
    processor_info_.max_frequency_mhz = ebx & 0xFFFF;
}
