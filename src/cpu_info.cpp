#include "cpu_info.h"
#include "arch_detect.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>

#ifdef CPU_ARCH_X86
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

#ifdef CPU_ARCH_ARM
#ifdef __linux__
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif
#endif

CPUInfo::CPUInfo() {
    detect();
}

bool CPUInfo::isARM() const {
#ifdef CPU_ARCH_ARM
    return true;
#else
    return false;
#endif
}

bool CPUInfo::isX86() const {
#ifdef CPU_ARCH_X86
    return true;
#else
    return false;
#endif
}

#ifdef CPU_ARCH_X86
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

#else
// ==================== ARM Implementation ====================

void CPUInfo::detect() {
#ifdef __aarch64__
    processor_info_.architecture = "ARM64 (AArch64)";
#else
    processor_info_.architecture = "ARM (AArch32)";
#endif
    
    detectARMInfo();
    detectARMFeatures();
    detectARMCache();
    detectARMTopology();
}

std::string CPUInfo::getARMImplementerName(uint8_t implementer) {
    switch (implementer) {
        case 0x41: return "ARM";
        case 0x42: return "Broadcom";
        case 0x43: return "Cavium";
        case 0x44: return "DEC";
        case 0x46: return "Fujitsu";
        case 0x48: return "HiSilicon";
        case 0x49: return "Infineon";
        case 0x4D: return "Motorola/Freescale";
        case 0x4E: return "NVIDIA";
        case 0x50: return "APM";
        case 0x51: return "Qualcomm";
        case 0x53: return "Samsung";
        case 0x56: return "Marvell";
        case 0x61: return "Apple";
        case 0x66: return "Faraday";
        case 0x69: return "Intel";
        case 0x6D: return "Microsoft";
        case 0x70: return "Phytium";
        case 0xC0: return "Ampere";
        default: return "Unknown";
    }
}

std::string CPUInfo::getARMPartName(uint8_t implementer, uint16_t part) {
    // ARM cores
    if (implementer == 0x41) {
        switch (part) {
            case 0xD02: return "Cortex-A34";
            case 0xD03: return "Cortex-A53";
            case 0xD04: return "Cortex-A35";
            case 0xD05: return "Cortex-A55";
            case 0xD06: return "Cortex-A65";
            case 0xD07: return "Cortex-A57";
            case 0xD08: return "Cortex-A72";
            case 0xD09: return "Cortex-A73";
            case 0xD0A: return "Cortex-A75";
            case 0xD0B: return "Cortex-A76";
            case 0xD0C: return "Neoverse N1";
            case 0xD0D: return "Cortex-A77";
            case 0xD0E: return "Cortex-A76AE";
            case 0xD40: return "Neoverse V1";
            case 0xD41: return "Cortex-A78";
            case 0xD42: return "Cortex-A78AE";
            case 0xD43: return "Cortex-A65AE";
            case 0xD44: return "Cortex-X1";
            case 0xD46: return "Cortex-A510";
            case 0xD47: return "Cortex-A710";
            case 0xD48: return "Cortex-X2";
            case 0xD49: return "Neoverse N2";
            case 0xD4A: return "Neoverse E1";
            case 0xD4B: return "Cortex-A78C";
            case 0xD4C: return "Cortex-X1C";
            case 0xD4D: return "Cortex-A715";
            case 0xD4E: return "Cortex-X3";
            case 0xD4F: return "Neoverse V2";
            case 0xD80: return "Cortex-A520";
            case 0xD81: return "Cortex-A720";
            case 0xD82: return "Cortex-X4";
            default: break;
        }
    }
    // Apple cores
    else if (implementer == 0x61) {
        switch (part) {
            case 0x020: return "Apple Icestorm (A14)";
            case 0x021: return "Apple Firestorm (A14)";
            case 0x022: return "Apple Icestorm (M1)";
            case 0x023: return "Apple Firestorm (M1)";
            case 0x024: return "Apple Icestorm (M1 Pro)";
            case 0x025: return "Apple Firestorm (M1 Pro)";
            case 0x028: return "Apple Icestorm (M1 Max)";
            case 0x029: return "Apple Firestorm (M1 Max)";
            case 0x030: return "Apple Blizzard (A15)";
            case 0x031: return "Apple Avalanche (A15)";
            case 0x032: return "Apple Blizzard (M2)";
            case 0x033: return "Apple Avalanche (M2)";
            default: break;
        }
    }
    // Qualcomm cores
    else if (implementer == 0x51) {
        switch (part) {
            case 0x800: return "Kryo 2xx Gold";
            case 0x801: return "Kryo 2xx Silver";
            case 0x802: return "Kryo 3xx Gold";
            case 0x803: return "Kryo 3xx Silver";
            case 0x804: return "Kryo 4xx Gold";
            case 0x805: return "Kryo 4xx Silver";
            case 0xC00: return "Falkor";
            case 0xC01: return "Saphira";
            default: break;
        }
    }
    // Ampere cores
    else if (implementer == 0xC0) {
        switch (part) {
            case 0xAC3: return "Ampere-1";
            case 0xAC4: return "Ampere-1A";
            default: break;
        }
    }
    // AWS Graviton (NVIDIA/Annapurna)
    else if (implementer == 0x4E) {
        switch (part) {
            case 0x003: return "Denver";
            case 0x004: return "Denver 2";
            default: break;
        }
    }
    
    return "Unknown";
}

void CPUInfo::detectARMInfo() {
    processor_info_.vendor = "ARM";
    processor_info_.brand = "ARM Processor";
    
#ifdef __linux__
    // Read from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("CPU implementer") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    uint8_t impl = static_cast<uint8_t>(std::stoul(value, nullptr, 0));
                    processor_info_.implementer = getARMImplementerName(impl);
                    processor_info_.midr = (impl << 24);  // Store in MIDR format
                }
            }
            else if (line.find("CPU part") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    uint16_t part = static_cast<uint16_t>(std::stoul(value, nullptr, 0));
                    uint8_t impl = (processor_info_.midr >> 24) & 0xFF;
                    processor_info_.part_name = getARMPartName(impl, part);
                    processor_info_.model = part;
                }
            }
            else if (line.find("CPU revision") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    processor_info_.stepping = static_cast<uint32_t>(std::stoul(value, nullptr, 0));
                }
            }
            else if (line.find("CPU variant") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    processor_info_.family = static_cast<uint32_t>(std::stoul(value, nullptr, 0));
                }
            }
            else if (line.find("model name") != std::string::npos || 
                     line.find("Model") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 2);
                    if (!value.empty()) {
                        processor_info_.brand = value;
                    }
                }
            }
        }
        cpuinfo.close();
    }
    
    // Build a better brand string if we have part info
    if (!processor_info_.part_name.empty() && processor_info_.part_name != "Unknown") {
        processor_info_.brand = processor_info_.implementer + " " + processor_info_.part_name;
    }
    processor_info_.vendor = processor_info_.implementer;
#endif
}

void CPUInfo::detectARMFeatures() {
#ifdef __linux__
    unsigned long hwcap = getauxval(AT_HWCAP);
    unsigned long hwcap2 = getauxval(AT_HWCAP2);
    
#ifdef __aarch64__
    // AArch64 hwcap flags
    features_.fp = (hwcap & HWCAP_FP) != 0;
    features_.asimd = (hwcap & HWCAP_ASIMD) != 0;
    features_.neon = features_.asimd;  // NEON is called ASIMD on AArch64
    features_.aes = (hwcap & HWCAP_AES) != 0;
    features_.pmull = (hwcap & HWCAP_PMULL) != 0;
    features_.sha1 = (hwcap & HWCAP_SHA1) != 0;
    features_.sha2 = (hwcap & HWCAP_SHA2) != 0;
    features_.crc32 = (hwcap & HWCAP_CRC32) != 0;
    features_.atomics = (hwcap & HWCAP_ATOMICS) != 0;
    features_.lse = features_.atomics;
    features_.fphp = (hwcap & HWCAP_FPHP) != 0;
    features_.asimdhp = (hwcap & HWCAP_ASIMDHP) != 0;
    features_.dcpop = (hwcap & HWCAP_DCPOP) != 0;
    features_.fcma = (hwcap & HWCAP_FCMA) != 0;
    features_.jscvt = (hwcap & HWCAP_JSCVT) != 0;
    features_.rdma = (hwcap & HWCAP_ASIMDRDM) != 0;
    features_.ssbs = (hwcap & HWCAP_SSBS) != 0;
    features_.sb = (hwcap & HWCAP_SB) != 0;
    features_.dotprod = (hwcap & HWCAP_ASIMDDP) != 0;
    features_.sha3 = (hwcap & HWCAP_SHA3) != 0;
    features_.sha512 = (hwcap & HWCAP_SHA512) != 0;
    features_.sve = (hwcap & HWCAP_SVE) != 0;
    
    // HWCAP2 flags (extended features)
    features_.bf16 = (hwcap2 & HWCAP2_BF16) != 0;
    features_.i8mm = (hwcap2 & HWCAP2_I8MM) != 0;
    features_.sve2 = (hwcap2 & HWCAP2_SVE2) != 0;
    
    // Get SVE vector length if supported
    if (features_.sve) {
        // SVE vector length can be read from /proc/sys/abi/sve_default_vector_length
        std::ifstream sve_vl("/proc/sys/abi/sve_default_vector_length");
        if (sve_vl.is_open()) {
            sve_vl >> features_.sve_vector_length;
            features_.sve_vector_length *= 8;  // Convert bytes to bits
            sve_vl.close();
        }
    }
#else
    // AArch32 hwcap flags
    features_.neon = (hwcap & HWCAP_NEON) != 0;
#endif
#endif
}

void CPUInfo::detectARMCache() {
#ifdef __linux__
    // Try to read cache info from sysfs
    auto readCacheSize = [](const std::string& path) -> uint32_t {
        std::ifstream file(path);
        if (file.is_open()) {
            std::string size_str;
            file >> size_str;
            file.close();
            
            // Parse size (e.g., "32K", "512K", "4M")
            uint32_t size = 0;
            char unit = 'K';
            std::sscanf(size_str.c_str(), "%u%c", &size, &unit);
            
            if (unit == 'M' || unit == 'm') {
                size *= 1024;
            }
            return size;
        }
        return 0;
    };
    
    auto readCacheLineSize = [](const std::string& path) -> uint32_t {
        std::ifstream file(path);
        if (file.is_open()) {
            uint32_t line_size;
            file >> line_size;
            file.close();
            return line_size;
        }
        return 0;
    };
    
    // Read cache sizes from CPU 0
    std::string cache_base = "/sys/devices/system/cpu/cpu0/cache/";
    
    for (int i = 0; i < 4; i++) {
        std::string index_path = cache_base + "index" + std::to_string(i) + "/";
        
        std::ifstream level_file(index_path + "level");
        std::ifstream type_file(index_path + "type");
        
        if (level_file.is_open() && type_file.is_open()) {
            int level;
            std::string type;
            level_file >> level;
            type_file >> type;
            level_file.close();
            type_file.close();
            
            uint32_t size = readCacheSize(index_path + "size");
            
            if (level == 1) {
                if (type == "Data" || type == "data") {
                    cache_info_.l1_data_size = size;
                } else if (type == "Instruction" || type == "instruction") {
                    cache_info_.l1_instruction_size = size;
                }
            } else if (level == 2) {
                cache_info_.l2_size = size;
            } else if (level == 3) {
                cache_info_.l3_size = size;
            }
            
            if (cache_info_.cache_line_size == 0) {
                cache_info_.cache_line_size = readCacheLineSize(index_path + "coherency_line_size");
            }
        }
    }
#endif
}

void CPUInfo::detectARMTopology() {
    // Use std::thread::hardware_concurrency as fallback
    processor_info_.logical_cores = std::thread::hardware_concurrency();
    processor_info_.physical_cores = processor_info_.logical_cores;  // ARM typically doesn't have SMT
    
#ifdef __linux__
    // Try to read more accurate info from sysfs
    std::ifstream present("/sys/devices/system/cpu/present");
    if (present.is_open()) {
        std::string range;
        present >> range;
        present.close();
        
        // Parse range like "0-7" or "0-3,4-7"
        size_t dash = range.find('-');
        if (dash != std::string::npos) {
            int last = std::stoi(range.substr(dash + 1));
            processor_info_.logical_cores = last + 1;
            processor_info_.physical_cores = processor_info_.logical_cores;
        }
    }
    
    // Try to read frequency info
    std::ifstream max_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (max_freq.is_open()) {
        uint32_t freq_khz;
        max_freq >> freq_khz;
        max_freq.close();
        processor_info_.max_frequency_mhz = freq_khz / 1000;
    }
    
    std::ifstream base_freq("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency");
    if (base_freq.is_open()) {
        uint32_t freq_khz;
        base_freq >> freq_khz;
        base_freq.close();
        processor_info_.base_frequency_mhz = freq_khz / 1000;
    } else {
        // Fallback to cpuinfo_min_freq as base
        std::ifstream min_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
        if (min_freq.is_open()) {
            uint32_t freq_khz;
            min_freq >> freq_khz;
            min_freq.close();
            processor_info_.base_frequency_mhz = freq_khz / 1000;
        }
    }
#endif
}

#endif
