#include "cpu_info.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef ARM_BUILD
#include <sys/auxv.h>
#include <asm/hwcap.h>
#include <unistd.h>
#include <thread>

CPUInfo::CPUInfo() {
    detect();
}

void CPUInfo::detect() {
    detectARMVendor();
    detectARMBrand();
    detectARMFeatures();
    detectARMCacheInfo();
    detectARMTopology();
}

void CPUInfo::detectARMVendor() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("CPU implementer") != std::string::npos) {
            size_t pos = line.find(": 0x");
            if (pos != std::string::npos) {
                processor_info_.implementer = std::stoul(line.substr(pos + 4), nullptr, 16);
            }
        } else if (line.find("CPU variant") != std::string::npos) {
            size_t pos = line.find(": 0x");
            if (pos != std::string::npos) {
                processor_info_.variant = std::stoul(line.substr(pos + 4), nullptr, 16);
            }
        } else if (line.find("CPU part") != std::string::npos) {
            size_t pos = line.find(": 0x");
            if (pos != std::string::npos) {
                processor_info_.part = std::stoul(line.substr(pos + 4), nullptr, 16);
            }
        } else if (line.find("CPU revision") != std::string::npos) {
            size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                processor_info_.revision = std::stoul(line.substr(pos + 2));
            }
        }
    }
    
    // Map implementer code to vendor name
    switch (processor_info_.implementer) {
        case 0x41: processor_info_.vendor = "ARM"; break;
        case 0x42: processor_info_.vendor = "Broadcom"; break;
        case 0x43: processor_info_.vendor = "Cavium"; break;
        case 0x44: processor_info_.vendor = "DEC"; break;
        case 0x46: processor_info_.vendor = "Fujitsu"; break;
        case 0x48: processor_info_.vendor = "HiSilicon"; break;
        case 0x49: processor_info_.vendor = "Infineon"; break;
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

void CPUInfo::detectARMBrand() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("Model") != std::string::npos || 
            line.find("model name") != std::string::npos) {
            size_t pos = line.find(": ");
            if (pos != std::string::npos) {
                processor_info_.brand = line.substr(pos + 2);
                return;
            }
        }
    }
    
    // Fallback: construct brand from vendor and part number
    processor_info_.brand = processor_info_.vendor + " ARMv8";
    
    // Try to map common ARM part numbers
    switch (processor_info_.part) {
        case 0xD03: processor_info_.brand = "Cortex-A53"; break;
        case 0xD04: processor_info_.brand = "Cortex-A35"; break;
        case 0xD05: processor_info_.brand = "Cortex-A55"; break;
        case 0xD07: processor_info_.brand = "Cortex-A57"; break;
        case 0xD08: processor_info_.brand = "Cortex-A72"; break;
        case 0xD09: processor_info_.brand = "Cortex-A73"; break;
        case 0xD0A: processor_info_.brand = "Cortex-A75"; break;
        case 0xD0B: processor_info_.brand = "Cortex-A76"; break;
        case 0xD0C: processor_info_.brand = "Neoverse-N1"; break;
        case 0xD0D: processor_info_.brand = "Cortex-A77"; break;
        case 0xD40: processor_info_.brand = "Neoverse-V1"; break;
        case 0xD41: processor_info_.brand = "Cortex-A78"; break;
        case 0xD44: processor_info_.brand = "Cortex-X1"; break;
        case 0xD46: processor_info_.brand = "Cortex-A510"; break;
        case 0xD47: processor_info_.brand = "Cortex-A710"; break;
        case 0xD48: processor_info_.brand = "Cortex-X2"; break;
        case 0xD49: processor_info_.brand = "Neoverse-N2"; break;
        case 0xD4A: processor_info_.brand = "Neoverse-E1"; break;
        default: break;
    }
    
    processor_info_.brand = processor_info_.vendor + " " + processor_info_.brand;
}

void CPUInfo::detectARMFeatures() {
    unsigned long hwcap = getauxval(AT_HWCAP);
    unsigned long hwcap2 = getauxval(AT_HWCAP2);
    
    // Basic features from HWCAP
    features_.fp = hwcap & HWCAP_FP;
    features_.asimd = hwcap & HWCAP_ASIMD;
    features_.neon = hwcap & HWCAP_ASIMD;  // NEON is ASIMD on AArch64
    features_.aes = hwcap & HWCAP_AES;
    features_.pmull = hwcap & HWCAP_PMULL;
    features_.sha1 = hwcap & HWCAP_SHA1;
    features_.sha2 = hwcap & HWCAP_SHA2;
    features_.crc32 = hwcap & HWCAP_CRC32;
    features_.atomics = hwcap & HWCAP_ATOMICS;
    features_.fphp = hwcap & HWCAP_FPHP;
    features_.asimdhp = hwcap & HWCAP_ASIMDHP;
    features_.neon_fp16 = hwcap & HWCAP_ASIMDHP;
    features_.asimddp = hwcap & HWCAP_ASIMDDP;
    features_.neon_dotprod = hwcap & HWCAP_ASIMDDP;
    features_.jscvt = hwcap & HWCAP_JSCVT;
    features_.fcma = hwcap & HWCAP_FCMA;
    features_.lrcpc = hwcap & HWCAP_LRCPC;
    features_.dcpop = hwcap & HWCAP_DCPOP;
    
#ifdef HWCAP_SHA3
    features_.sha3 = hwcap & HWCAP_SHA3;
#endif
#ifdef HWCAP_SHA512
    features_.sha512 = hwcap & HWCAP_SHA512;
#endif
#ifdef HWCAP_SVE
    features_.sve = hwcap & HWCAP_SVE;
#endif
#ifdef HWCAP_PACA
    features_.paca = hwcap & HWCAP_PACA;
#endif
#ifdef HWCAP_PACG
    features_.pacg = hwcap & HWCAP_PACG;
#endif
#ifdef HWCAP_SSBS
    features_.ssbs = hwcap & HWCAP_SSBS;
#endif
#ifdef HWCAP_FLAGM
    features_.flagm = hwcap & HWCAP_FLAGM;
#endif
#ifdef HWCAP_SB
    features_.sb = hwcap & HWCAP_SB;
#endif

    // Features from HWCAP2
#ifdef HWCAP2_SVE2
    features_.sve2 = hwcap2 & HWCAP2_SVE2;
#endif
#ifdef HWCAP2_BTI
    features_.bti = hwcap2 & HWCAP2_BTI;
#endif
#ifdef HWCAP2_MTE
    features_.mte = hwcap2 & HWCAP2_MTE;
#endif
#ifdef HWCAP2_BF16
    features_.neon_bf16 = hwcap2 & HWCAP2_BF16;
#endif
#ifdef HWCAP2_I8MM
    features_.neon_i8mm = hwcap2 & HWCAP2_I8MM;
#endif

    // Get SVE vector length if SVE is available
    if (features_.sve) {
#ifdef HWCAP_SVE
        // SVE vector length can be read from /proc/sys/abi/sve_default_vector_length
        std::ifstream sve_vl("/proc/sys/abi/sve_default_vector_length");
        if (sve_vl.is_open()) {
            sve_vl >> features_.sve_vector_length;
            features_.sve_vector_length *= 8;  // Convert bytes to bits
        }
#endif
    }
}

void CPUInfo::detectARMCacheInfo() {
    // Try to read from sysfs
    std::string base_path = "/sys/devices/system/cpu/cpu0/cache/";
    
    for (int i = 0; i < 4; i++) {
        std::string index_path = base_path + "index" + std::to_string(i) + "/";
        
        std::ifstream level_file(index_path + "level");
        std::ifstream type_file(index_path + "type");
        std::ifstream size_file(index_path + "size");
        std::ifstream line_size_file(index_path + "coherency_line_size");
        
        if (!level_file.is_open() || !type_file.is_open() || !size_file.is_open()) {
            continue;
        }
        
        int level;
        std::string type;
        std::string size_str;
        
        level_file >> level;
        type_file >> type;
        size_file >> size_str;
        
        // Parse size (e.g., "32K", "512K", "4M")
        uint32_t size = 0;
        if (!size_str.empty()) {
            size = std::stoul(size_str);
            if (size_str.back() == 'K' || size_str.back() == 'k') {
                // Already in KB
            } else if (size_str.back() == 'M' || size_str.back() == 'm') {
                size *= 1024;
            }
        }
        
        if (cache_info_.cache_line_size == 0 && line_size_file.is_open()) {
            line_size_file >> cache_info_.cache_line_size;
        }
        
        if (level == 1) {
            if (type == "Data") {
                cache_info_.l1_data_size = size;
            } else if (type == "Instruction") {
                cache_info_.l1_instruction_size = size;
            }
        } else if (level == 2) {
            cache_info_.l2_size = size;
        } else if (level == 3) {
            cache_info_.l3_size = size;
        }
    }
    
    // Default cache line size for ARM if not found
    if (cache_info_.cache_line_size == 0) {
        cache_info_.cache_line_size = 64;  // Common default for ARMv8
    }
}

void CPUInfo::detectARMTopology() {
    // Count processors from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    uint32_t processor_count = 0;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("processor") != std::string::npos && line.find(":") != std::string::npos) {
            processor_count++;
        }
    }
    
    if (processor_count == 0) {
        // Fallback to hardware concurrency
        processor_count = std::thread::hardware_concurrency();
    }
    
    processor_info_.logical_cores = processor_count;
    processor_info_.physical_cores = processor_count;  // ARM typically doesn't have SMT
    
    // Try to read online CPUs from sysfs
    std::ifstream online("/sys/devices/system/cpu/online");
    if (online.is_open()) {
        std::string range;
        online >> range;
        // Parse range like "0-7" to get count
        size_t dash = range.find('-');
        if (dash != std::string::npos) {
            int last = std::stoi(range.substr(dash + 1));
            processor_info_.logical_cores = last + 1;
            processor_info_.physical_cores = last + 1;
        }
    }
    
    // Read frequency if available
    std::ifstream max_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (max_freq.is_open()) {
        uint32_t freq_khz;
        max_freq >> freq_khz;
        processor_info_.max_frequency_mhz = freq_khz / 1000;
    }
    
    std::ifstream base_freq("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency");
    if (base_freq.is_open()) {
        uint32_t freq_khz;
        base_freq >> freq_khz;
        processor_info_.base_frequency_mhz = freq_khz / 1000;
    } else {
        // Fallback: use scaling_min_freq as base
        std::ifstream min_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
        if (min_freq.is_open()) {
            uint32_t freq_khz;
            min_freq >> freq_khz;
            processor_info_.base_frequency_mhz = freq_khz / 1000;
        }
    }
}

#else // x86 implementation

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
            // uint32_t core_mask_width = eax & 0x1F;  // Unused
            
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

#endif // ARM_BUILD
