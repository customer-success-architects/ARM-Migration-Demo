#include "cpu_info.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>

#ifdef TARGET_ARCH_X86
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

#ifdef TARGET_ARCH_ARM
#ifdef __linux__
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif
#endif

CPUInfo::CPUInfo() {
    detect();
}

#ifdef TARGET_ARCH_X86
// ==================== x86 Implementation ====================

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
    
    processor_info_.architecture = "x86_64";
    
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
            // uint32_t core_mask_width = eax & 0x1F;
            
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

#elif defined(TARGET_ARCH_ARM)
// ==================== ARM Implementation ====================

uint64_t CPUInfo::getAuxVal(unsigned long type) {
#ifdef __linux__
    return getauxval(type);
#else
    (void)type;
    return 0;
#endif
}

void CPUInfo::detect() {
    processor_info_.architecture = "ARM64";
    detectARMInfo();
    detectARMFeatures();
    detectARMCacheInfo();
    detectARMTopology();
}

void CPUInfo::detectARMInfo() {
#ifdef __linux__
    // Read from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("CPU implementer") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.implementer = std::stoul(line.substr(pos + 1), nullptr, 0);
                // Map implementer code to vendor name
                switch (processor_info_.implementer) {
                    case 0x41: processor_info_.vendor = "ARM"; break;
                    case 0x42: processor_info_.vendor = "Broadcom"; break;
                    case 0x43: processor_info_.vendor = "Cavium"; break;
                    case 0x44: processor_info_.vendor = "DEC"; break;
                    case 0x46: processor_info_.vendor = "Fujitsu"; break;
                    case 0x48: processor_info_.vendor = "HiSilicon"; break;
                    case 0x49: processor_info_.vendor = "Infineon"; break;
                    case 0x4D: processor_info_.vendor = "Motorola/Freescale"; break;
                    case 0x4E: processor_info_.vendor = "NVIDIA"; break;
                    case 0x50: processor_info_.vendor = "APM"; break;
                    case 0x51: processor_info_.vendor = "Qualcomm"; break;
                    case 0x53: processor_info_.vendor = "Samsung"; break;
                    case 0x56: processor_info_.vendor = "Marvell"; break;
                    case 0x61: processor_info_.vendor = "Apple"; break;
                    case 0x66: processor_info_.vendor = "Faraday"; break;
                    case 0x69: processor_info_.vendor = "Intel"; break;
                    case 0xC0: processor_info_.vendor = "Ampere"; break;
                    default: processor_info_.vendor = "Unknown ARM"; break;
                }
            }
        } else if (line.find("CPU variant") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.variant = std::stoul(line.substr(pos + 1), nullptr, 0);
            }
        } else if (line.find("CPU part") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.part = std::stoul(line.substr(pos + 1), nullptr, 0);
            }
        } else if (line.find("CPU revision") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.revision = std::stoul(line.substr(pos + 1), nullptr, 0);
            }
        } else if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.brand = line.substr(pos + 2);
            }
        } else if (line.find("Hardware") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos && processor_info_.brand.empty()) {
                processor_info_.brand = line.substr(pos + 2);
            }
        }
    }
    
    // Map family/model based on implementer and part
    processor_info_.family = processor_info_.implementer;
    processor_info_.model = processor_info_.part;
    processor_info_.stepping = processor_info_.revision;
    
    // Build brand string if not found
    if (processor_info_.brand.empty()) {
        std::stringstream ss;
        ss << processor_info_.vendor << " ";
        
        // Map ARM part numbers to common names
        if (processor_info_.implementer == 0x41) { // ARM
            switch (processor_info_.part) {
                case 0xD03: ss << "Cortex-A53"; break;
                case 0xD04: ss << "Cortex-A35"; break;
                case 0xD05: ss << "Cortex-A55"; break;
                case 0xD06: ss << "Cortex-A65"; break;
                case 0xD07: ss << "Cortex-A57"; break;
                case 0xD08: ss << "Cortex-A72"; break;
                case 0xD09: ss << "Cortex-A73"; break;
                case 0xD0A: ss << "Cortex-A75"; break;
                case 0xD0B: ss << "Cortex-A76"; break;
                case 0xD0C: ss << "Neoverse-N1"; break;
                case 0xD0D: ss << "Cortex-A77"; break;
                case 0xD0E: ss << "Cortex-A76AE"; break;
                case 0xD40: ss << "Neoverse-V1"; break;
                case 0xD41: ss << "Cortex-A78"; break;
                case 0xD42: ss << "Cortex-A78AE"; break;
                case 0xD43: ss << "Cortex-A65AE"; break;
                case 0xD44: ss << "Cortex-X1"; break;
                case 0xD46: ss << "Cortex-A510"; break;
                case 0xD47: ss << "Cortex-A710"; break;
                case 0xD48: ss << "Cortex-X2"; break;
                case 0xD49: ss << "Neoverse-N2"; break;
                case 0xD4A: ss << "Neoverse-E1"; break;
                case 0xD4B: ss << "Cortex-A78C"; break;
                case 0xD4C: ss << "Cortex-X1C"; break;
                case 0xD4D: ss << "Cortex-A715"; break;
                case 0xD4E: ss << "Cortex-X3"; break;
                case 0xD4F: ss << "Neoverse-V2"; break;
                default: ss << "Unknown (0x" << std::hex << processor_info_.part << ")"; break;
            }
        } else if (processor_info_.implementer == 0x61) { // Apple
            switch (processor_info_.part) {
                case 0x20: ss << "M1 (Icestorm)"; break;
                case 0x21: ss << "M1 (Firestorm)"; break;
                case 0x22: ss << "M1 Pro/Max (Icestorm)"; break;
                case 0x23: ss << "M1 Pro/Max (Firestorm)"; break;
                case 0x24: ss << "M2 (Blizzard)"; break;
                case 0x25: ss << "M2 (Avalanche)"; break;
                default: ss << "Apple Silicon (0x" << std::hex << processor_info_.part << ")"; break;
            }
        } else {
            ss << "CPU (Part 0x" << std::hex << processor_info_.part << ")";
        }
        
        processor_info_.brand = ss.str();
    }
    
    // Try to get frequency from cpuinfo_max_freq
    std::ifstream max_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (max_freq.is_open()) {
        uint32_t freq_khz;
        max_freq >> freq_khz;
        processor_info_.max_frequency_mhz = freq_khz / 1000;
    }
    
    std::ifstream cur_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
    if (cur_freq.is_open()) {
        uint32_t freq_khz;
        cur_freq >> freq_khz;
        processor_info_.base_frequency_mhz = freq_khz / 1000;
    }
#else
    processor_info_.vendor = "ARM";
    processor_info_.brand = "ARM Processor";
#endif
}

void CPUInfo::detectARMFeatures() {
#ifdef __linux__
    // Use getauxval to detect ARM CPU features
    unsigned long hwcap = getauxval(AT_HWCAP);
    unsigned long hwcap2 = getauxval(AT_HWCAP2);
    
    // HWCAP flags (ARM64)
    features_.fp = hwcap & HWCAP_FP;
    features_.neon = hwcap & HWCAP_ASIMD;  // ASIMD = Advanced SIMD (NEON)
    features_.aes = hwcap & HWCAP_AES;
    features_.pmull = hwcap & HWCAP_PMULL;
    features_.sha1 = hwcap & HWCAP_SHA1;
    features_.sha2 = hwcap & HWCAP_SHA2;
    features_.crc32 = hwcap & HWCAP_CRC32;
    features_.atomics = hwcap & HWCAP_ATOMICS;
    features_.neon_fp16 = hwcap & HWCAP_FPHP;
    features_.dcpop = hwcap & HWCAP_DCPOP;
    features_.sha3 = hwcap & HWCAP_SHA3;
    features_.sha512 = hwcap & HWCAP_SHA512;
    features_.sve = hwcap & HWCAP_SVE;
    features_.neon_dotprod = hwcap & HWCAP_ASIMDDP;
    features_.fp16 = hwcap & HWCAP_ASIMDHP;
    features_.flagm = hwcap & HWCAP_FLAGM;
    features_.ssbs = hwcap & HWCAP_SSBS;
    features_.sb = hwcap & HWCAP_SB;
    features_.paca = hwcap & HWCAP_PACA;
    features_.pacg = hwcap & HWCAP_PACG;
    
    // HWCAP2 flags (ARM64)
    features_.dcpodp = hwcap2 & HWCAP2_DCPODP;
    features_.sve2 = hwcap2 & HWCAP2_SVE2;
    features_.frint = hwcap2 & HWCAP2_FRINT;
    features_.i8mm = hwcap2 & HWCAP2_I8MM;
    features_.bf16 = hwcap2 & HWCAP2_BF16;
    features_.rng = hwcap2 & HWCAP2_RNG;
    features_.bti = hwcap2 & HWCAP2_BTI;
    features_.mte = hwcap2 & HWCAP2_MTE;
#endif
}

void CPUInfo::detectARMCacheInfo() {
#ifdef __linux__
    // Read cache information from sysfs
    auto readCacheSize = [](const std::string& path) -> uint32_t {
        std::ifstream file(path);
        if (!file.is_open()) return 0;
        
        std::string size_str;
        file >> size_str;
        
        uint32_t size = 0;
        char unit = 'K';
        
        // Parse size like "32K" or "1M"
        size_t pos = 0;
        while (pos < size_str.length() && std::isdigit(size_str[pos])) {
            size = size * 10 + (size_str[pos] - '0');
            pos++;
        }
        if (pos < size_str.length()) {
            unit = size_str[pos];
        }
        
        if (unit == 'M' || unit == 'm') {
            size *= 1024;
        }
        
        return size; // in KB
    };
    
    auto readCacheType = [](const std::string& path) -> std::string {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::string type;
        file >> type;
        return type;
    };
    
    auto readLineSize = [](const std::string& path) -> uint32_t {
        std::ifstream file(path);
        if (!file.is_open()) return 0;
        uint32_t size;
        file >> size;
        return size;
    };
    
    // Iterate through cache indices
    for (int i = 0; i < 10; i++) {
        std::string base = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i);
        
        std::ifstream level_file(base + "/level");
        if (!level_file.is_open()) break;
        
        int level;
        level_file >> level;
        
        std::string type = readCacheType(base + "/type");
        uint32_t size = readCacheSize(base + "/size");
        
        if (cache_info_.cache_line_size == 0) {
            cache_info_.cache_line_size = readLineSize(base + "/coherency_line_size");
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
#endif
}

void CPUInfo::detectARMTopology() {
#ifdef __linux__
    // Get the number of logical cores
    processor_info_.logical_cores = std::thread::hardware_concurrency();
    
    // Try to read physical package count and cores per package from sysfs
    // For simplicity, assume physical cores = logical cores (ARM typically doesn't have SMT)
    processor_info_.physical_cores = processor_info_.logical_cores;
    
    // Some ARM processors support SMT, try to detect
    std::ifstream smt("/sys/devices/system/cpu/smt/active");
    if (smt.is_open()) {
        int active;
        smt >> active;
        if (active) {
            // SMT is active, assume 2 threads per core
            processor_info_.physical_cores = processor_info_.logical_cores / 2;
        }
    }
#else
    processor_info_.logical_cores = std::thread::hardware_concurrency();
    processor_info_.physical_cores = processor_info_.logical_cores;
#endif
}

#else
// ==================== Generic Implementation ====================

void CPUInfo::detect() {
    detectGenericInfo();
}

void CPUInfo::detectGenericInfo() {
    processor_info_.vendor = "Unknown";
    processor_info_.brand = "Unknown Processor";
    processor_info_.architecture = "Unknown";
    processor_info_.logical_cores = std::thread::hardware_concurrency();
    processor_info_.physical_cores = processor_info_.logical_cores;
}

#endif
