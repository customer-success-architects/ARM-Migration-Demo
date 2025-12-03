#include "cpu_info.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef ARM_BUILD
// ARM-specific includes
#include <sys/auxv.h>
#ifndef HWCAP_NEON
#define HWCAP_NEON      (1 << 12)
#endif
#ifndef HWCAP_AES
#define HWCAP_AES       (1 << 3)
#endif
#ifndef HWCAP_PMULL
#define HWCAP_PMULL     (1 << 4)
#endif
#ifndef HWCAP_SHA1
#define HWCAP_SHA1      (1 << 5)
#endif
#ifndef HWCAP_SHA2
#define HWCAP_SHA2      (1 << 6)
#endif
#ifndef HWCAP_CRC32
#define HWCAP_CRC32     (1 << 7)
#endif
#ifndef HWCAP_ATOMICS
#define HWCAP_ATOMICS   (1 << 8)
#endif
#ifndef HWCAP_FPHP
#define HWCAP_FPHP      (1 << 9)
#endif
#ifndef HWCAP_ASIMDHP
#define HWCAP_ASIMDHP   (1 << 10)
#endif
#ifndef HWCAP_ASIMD
#define HWCAP_ASIMD     (1 << 1)
#endif
#ifndef HWCAP_FP
#define HWCAP_FP        (1 << 0)
#endif
#ifndef HWCAP_JSCVT
#define HWCAP_JSCVT     (1 << 13)
#endif
#ifndef HWCAP_FCMA
#define HWCAP_FCMA      (1 << 14)
#endif
#ifndef HWCAP_LRCPC
#define HWCAP_LRCPC     (1 << 15)
#endif
#ifndef HWCAP_DCPOP
#define HWCAP_DCPOP     (1 << 16)
#endif
#ifndef HWCAP_SHA3
#define HWCAP_SHA3      (1 << 17)
#endif
#ifndef HWCAP_SM3
#define HWCAP_SM3       (1 << 18)
#endif
#ifndef HWCAP_SM4
#define HWCAP_SM4       (1 << 19)
#endif
#ifndef HWCAP_ASIMDDP
#define HWCAP_ASIMDDP   (1 << 20)
#endif
#ifndef HWCAP_SHA512
#define HWCAP_SHA512    (1 << 21)
#endif
#ifndef HWCAP_SVE
#define HWCAP_SVE       (1 << 22)
#endif
#ifndef HWCAP_ASIMDFHM
#define HWCAP_ASIMDFHM  (1 << 23)
#endif
#ifndef HWCAP_DIT
#define HWCAP_DIT       (1 << 24)
#endif
#ifndef HWCAP_USCAT
#define HWCAP_USCAT     (1 << 25)
#endif
#ifndef HWCAP_ILRCPC
#define HWCAP_ILRCPC    (1 << 26)
#endif
#ifndef HWCAP_FLAGM
#define HWCAP_FLAGM     (1 << 27)
#endif
#ifndef HWCAP_SSBS
#define HWCAP_SSBS      (1 << 28)
#endif
#ifndef HWCAP_SB
#define HWCAP_SB        (1 << 29)
#endif
#ifndef HWCAP_PACA
#define HWCAP_PACA      (1 << 30)
#endif
#ifndef HWCAP_PACG
#define HWCAP_PACG      (1UL << 31)
#endif

// HWCAP2 flags
#ifndef HWCAP2_SVE2
#define HWCAP2_SVE2     (1 << 1)
#endif
#ifndef HWCAP2_SVEAES
#define HWCAP2_SVEAES   (1 << 2)
#endif
#ifndef HWCAP2_SVEPMULL
#define HWCAP2_SVEPMULL (1 << 3)
#endif
#ifndef HWCAP2_SVEBITPERM
#define HWCAP2_SVEBITPERM (1 << 4)
#endif
#ifndef HWCAP2_SVESHA3
#define HWCAP2_SVESHA3  (1 << 5)
#endif
#ifndef HWCAP2_SVESM4
#define HWCAP2_SVESM4   (1 << 6)
#endif
#ifndef HWCAP2_FLAGM2
#define HWCAP2_FLAGM2   (1 << 7)
#endif
#ifndef HWCAP2_FRINT
#define HWCAP2_FRINT    (1 << 8)
#endif
#ifndef HWCAP2_I8MM
#define HWCAP2_I8MM     (1 << 13)
#endif
#ifndef HWCAP2_BF16
#define HWCAP2_BF16     (1 << 14)
#endif
#ifndef HWCAP2_BTI
#define HWCAP2_BTI      (1 << 17)
#endif
#ifndef HWCAP2_MTE
#define HWCAP2_MTE      (1 << 18)
#endif

#else
// x86-specific includes
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

CPUInfo::CPUInfo() {
    detect();
}

#ifdef ARM_BUILD

void CPUInfo::detect() {
    processor_info_.architecture = "ARM64";
    detectARMInfo();
    detectARMFeatures();
    detectARMCacheInfo();
    detectARMTopology();
    detectARMFrequency();
}

void CPUInfo::detectARMInfo() {
    // Read /proc/cpuinfo to get processor information
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) {
        processor_info_.vendor = "Unknown";
        processor_info_.brand = "ARM Processor";
        return;
    }
    
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("CPU implementer") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                // Trim whitespace
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                int implementer = std::stoi(value, nullptr, 0);
                switch (implementer) {
                    case 0x41: processor_info_.vendor = "ARM"; break;
                    case 0x42: processor_info_.vendor = "Broadcom"; break;
                    case 0x43: processor_info_.vendor = "Cavium"; break;
                    case 0x44: processor_info_.vendor = "DEC"; break;
                    case 0x46: processor_info_.vendor = "Fujitsu"; break;
                    case 0x48: processor_info_.vendor = "HiSilicon"; break;
                    case 0x4D: processor_info_.vendor = "Motorola"; break;
                    case 0x4E: processor_info_.vendor = "NVIDIA"; break;
                    case 0x50: processor_info_.vendor = "APM"; break;
                    case 0x51: processor_info_.vendor = "Qualcomm"; break;
                    case 0x53: processor_info_.vendor = "Samsung"; break;
                    case 0x56: processor_info_.vendor = "Marvell"; break;
                    case 0x61: processor_info_.vendor = "Apple"; break;
                    case 0x66: processor_info_.vendor = "Faraday"; break;
                    case 0x69: processor_info_.vendor = "Intel"; break;
                    case 0x6D: processor_info_.vendor = "Microsoft"; break;
                    case 0x70: processor_info_.vendor = "Phytium"; break;
                    case 0xC0: processor_info_.vendor = "Ampere"; break;
                    default: processor_info_.vendor = "Unknown"; break;
                }
            }
        }
        else if (line.find("CPU part") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                processor_info_.model = std::stoi(value, nullptr, 0);
            }
        }
        else if (line.find("CPU revision") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                processor_info_.stepping = std::stoi(value);
            }
        }
        else if (line.find("model name") != std::string::npos || line.find("Processor") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos && processor_info_.brand.empty()) {
                std::string value = line.substr(pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                processor_info_.brand = value;
            }
        }
    }
    
    if (processor_info_.brand.empty()) {
        processor_info_.brand = processor_info_.vendor + " ARM64 Processor";
    }
    
    // ARM doesn't have the same family concept as x86, but we can use ARMv8 as the family
    processor_info_.family = 8;  // ARMv8
}

void CPUInfo::detectARMFeatures() {
    unsigned long hwcap = getauxval(AT_HWCAP);
    unsigned long hwcap2 = getauxval(AT_HWCAP2);
    
    // ARM64 SIMD
    features_.neon = hwcap & HWCAP_ASIMD;
    features_.sve = hwcap & HWCAP_SVE;
    features_.sve2 = hwcap2 & HWCAP2_SVE2;
    
    // Cryptographic
    features_.aes = hwcap & HWCAP_AES;
    features_.sha1 = hwcap & HWCAP_SHA1;
    features_.sha2 = hwcap & HWCAP_SHA2;
    features_.sha3 = hwcap & HWCAP_SHA3;
    features_.sha512 = hwcap & HWCAP_SHA512;
    features_.pmull = hwcap & HWCAP_PMULL;
    
    // Floating Point
    features_.fp = hwcap & HWCAP_FP;
    features_.fp16 = hwcap & HWCAP_FPHP;
    features_.bf16 = hwcap2 & HWCAP2_BF16;
    
    // Atomics and Memory
    features_.atomics = hwcap & HWCAP_ATOMICS;
    features_.crc32 = hwcap & HWCAP_CRC32;
    features_.lse = hwcap & HWCAP_ATOMICS;  // LSE is same as atomics
    
    // Other Features
    features_.dotprod = hwcap & HWCAP_ASIMDDP;
    features_.i8mm = hwcap2 & HWCAP2_I8MM;
    features_.jscvt = hwcap & HWCAP_JSCVT;
    features_.fcma = hwcap & HWCAP_FCMA;
    features_.frint = hwcap2 & HWCAP2_FRINT;
    features_.sb = hwcap & HWCAP_SB;
    features_.ssbs = hwcap & HWCAP_SSBS;
    features_.bti = hwcap2 & HWCAP2_BTI;
    features_.mte = hwcap2 & HWCAP2_MTE;
}

void CPUInfo::detectARMCacheInfo() {
    // Try to read cache information from sysfs
    auto readCacheSize = [](const std::string& path) -> uint32_t {
        std::ifstream file(path);
        if (!file.is_open()) return 0;
        
        std::string value;
        std::getline(file, value);
        
        // Value is in format like "32K" or "1024K"
        size_t multiplier = 1;
        if (value.back() == 'K' || value.back() == 'k') {
            value.pop_back();
            multiplier = 1;
        } else if (value.back() == 'M' || value.back() == 'm') {
            value.pop_back();
            multiplier = 1024;
        }
        
        return std::stoul(value) * multiplier;
    };
    
    // Try L1 data cache
    cache_info_.l1_data_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index0/size");
    
    // Try L1 instruction cache
    cache_info_.l1_instruction_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index1/size");
    
    // Try L2 cache
    cache_info_.l2_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index2/size");
    
    // Try L3 cache
    cache_info_.l3_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index3/size");
    
    // Try to get cache line size
    std::ifstream linesize("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size");
    if (linesize.is_open()) {
        linesize >> cache_info_.cache_line_size;
    } else {
        cache_info_.cache_line_size = 64;  // Default for ARM64
    }
}

void CPUInfo::detectARMTopology() {
    // Count CPUs from /sys/devices/system/cpu/
    processor_info_.logical_cores = 0;
    
    std::ifstream present("/sys/devices/system/cpu/present");
    if (present.is_open()) {
        std::string range;
        present >> range;
        // Parse range like "0-7" or "0-3,5-7"
        size_t dashPos = range.find('-');
        if (dashPos != std::string::npos) {
            int last = std::stoi(range.substr(dashPos + 1));
            processor_info_.logical_cores = last + 1;
        } else {
            processor_info_.logical_cores = 1;
        }
    }
    
    // For ARM, physical cores often equals logical cores (no SMT typically)
    // Check for cluster information
    processor_info_.physical_cores = processor_info_.logical_cores;
    
    // Try to read SMT status
    std::ifstream smt("/sys/devices/system/cpu/smt/active");
    if (smt.is_open()) {
        int active;
        smt >> active;
        if (active) {
            processor_info_.physical_cores = processor_info_.logical_cores / 2;
        }
    }
}

void CPUInfo::detectARMFrequency() {
    // Try to read max frequency from cpufreq
    std::ifstream maxfreq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (maxfreq.is_open()) {
        uint32_t freq_khz;
        maxfreq >> freq_khz;
        processor_info_.max_frequency_mhz = freq_khz / 1000;
    }
    
    // Try to read base frequency
    std::ifstream basefreq("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency");
    if (basefreq.is_open()) {
        uint32_t freq_khz;
        basefreq >> freq_khz;
        processor_info_.base_frequency_mhz = freq_khz / 1000;
    } else {
        // Fall back to min frequency as base
        std::ifstream minfreq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
        if (minfreq.is_open()) {
            uint32_t freq_khz;
            minfreq >> freq_khz;
            processor_info_.base_frequency_mhz = freq_khz / 1000;
        }
    }
}

#else // x86 implementation

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
    processor_info_.architecture = "x86_64";
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
        
        if (cache_info_.cache_line_size == 0) {
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

#endif
