#include "cpu_info.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <thread>

#ifdef __aarch64__
#include <sys/auxv.h>
#ifdef __linux__
#include <asm/hwcap.h>
#endif
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#else
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

CPUInfo::CPUInfo() {
    detect();
}

void CPUInfo::detect() {
#ifdef __aarch64__
    detectARM();
#else
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
#endif
}

#ifdef __aarch64__

void CPUInfo::detectARM() {
    parseARMCPUInfo();
    detectARMFeatures();
    detectARMCacheInfo();
    detectARMTopology();
}

void CPUInfo::parseARMCPUInfo() {
#ifdef __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) {
        processor_info_.brand = "Unknown ARM Processor";
        return;
    }
    
    std::string line;
    bool first_processor = true;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("CPU implementer") != std::string::npos && first_processor) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                processor_info_.implementer = value;
                
                // Map implementer codes to vendor names
                if (value == "0x41") processor_info_.vendor = "ARM";
                else if (value == "0x42") processor_info_.vendor = "Broadcom";
                else if (value == "0x43") processor_info_.vendor = "Cavium";
                else if (value == "0x44") processor_info_.vendor = "DEC";
                else if (value == "0x4e") processor_info_.vendor = "Nvidia";
                else if (value == "0x50") processor_info_.vendor = "APM";
                else if (value == "0x51") processor_info_.vendor = "Qualcomm";
                else if (value == "0x53") processor_info_.vendor = "Samsung";
                else if (value == "0x56") processor_info_.vendor = "Marvell";
                else if (value == "0x61") processor_info_.vendor = "Apple";
                else processor_info_.vendor = "Unknown (" + value + ")";
            }
        }
        else if (line.find("CPU architecture") != std::string::npos && first_processor) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.architecture = line.substr(pos + 1);
                processor_info_.architecture.erase(0, processor_info_.architecture.find_first_not_of(" \t"));
            }
        }
        else if (line.find("CPU variant") != std::string::npos && first_processor) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.variant = line.substr(pos + 1);
                processor_info_.variant.erase(0, processor_info_.variant.find_first_not_of(" \t"));
            }
        }
        else if (line.find("CPU part") != std::string::npos && first_processor) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.part = line.substr(pos + 1);
                processor_info_.part.erase(0, processor_info_.part.find_first_not_of(" \t"));
            }
        }
        else if (line.find("CPU revision") != std::string::npos && first_processor) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.revision = line.substr(pos + 1);
                processor_info_.revision.erase(0, processor_info_.revision.find_first_not_of(" \t"));
            }
            first_processor = false;
        }
        else if (line.find("model name") != std::string::npos && processor_info_.brand.empty()) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                processor_info_.brand = line.substr(pos + 1);
                processor_info_.brand.erase(0, processor_info_.brand.find_first_not_of(" \t"));
            }
        }
    }
    
    if (processor_info_.brand.empty()) {
        processor_info_.brand = processor_info_.vendor + " ARM Processor";
    }
#elif defined(__APPLE__)
    // macOS ARM detection
    char brand[128];
    size_t size = sizeof(brand);
    if (sysctlbyname("machdep.cpu.brand_string", &brand, &size, NULL, 0) == 0) {
        processor_info_.brand = brand;
    } else {
        processor_info_.brand = "Apple Silicon";
    }
    processor_info_.vendor = "Apple";
    processor_info_.implementer = "0x61";
#endif
}

void CPUInfo::detectARMFeatures() {
#ifdef __linux__
    unsigned long hwcaps = getauxval(AT_HWCAP);
    unsigned long hwcaps2 = getauxval(AT_HWCAP2);
    
    // AT_HWCAP features
    features_.fp = hwcaps & HWCAP_FP;
    features_.asimd = hwcaps & HWCAP_ASIMD;
    features_.neon = features_.asimd;  // NEON is another name for ASIMD
    
#ifdef HWCAP_AES
    features_.aes = hwcaps & HWCAP_AES;
#endif
#ifdef HWCAP_PMULL
    features_.pmull = hwcaps & HWCAP_PMULL;
#endif
#ifdef HWCAP_SHA1
    features_.sha1 = hwcaps & HWCAP_SHA1;
#endif
#ifdef HWCAP_SHA2
    features_.sha2 = hwcaps & HWCAP_SHA2;
#endif
#ifdef HWCAP_CRC32
    features_.crc32 = hwcaps & HWCAP_CRC32;
#endif
#ifdef HWCAP_ATOMICS
    features_.atomics = hwcaps & HWCAP_ATOMICS;
    features_.lse = features_.atomics;
#endif
#ifdef HWCAP_FPHP
    features_.fp16 = hwcaps & HWCAP_FPHP;
#endif
#ifdef HWCAP_ASIMDHP
    features_.fp16 = features_.fp16 || (hwcaps & HWCAP_ASIMDHP);
#endif
#ifdef HWCAP_ASIMDRDM
    features_.rdm = hwcaps & HWCAP_ASIMDRDM;
#endif
#ifdef HWCAP_JSCVT
    features_.jscvt = hwcaps & HWCAP_JSCVT;
#endif
#ifdef HWCAP_FCMA
    features_.fcma = hwcaps & HWCAP_FCMA;
#endif
#ifdef HWCAP_ASIMDDP
    features_.dotprod = hwcaps & HWCAP_ASIMDDP;
#endif
#ifdef HWCAP_SHA3
    features_.sha3 = hwcaps & HWCAP_SHA3;
#endif
#ifdef HWCAP_SM3
    features_.sm3 = hwcaps & HWCAP_SM3;
#endif
#ifdef HWCAP_SM4
    features_.sm4 = hwcaps & HWCAP_SM4;
#endif
#ifdef HWCAP_SHA512
    features_.sha512 = hwcaps & HWCAP_SHA512;
#endif
#ifdef HWCAP_SVE
    features_.sve = hwcaps & HWCAP_SVE;
#endif
#ifdef HWCAP_FRINT
    features_.frint = hwcaps & HWCAP_FRINT;
#endif
    
    // AT_HWCAP2 features
#ifdef HWCAP2_DCPOP
    features_.dcpop = hwcaps2 & HWCAP2_DCPOP;
#endif
#ifdef HWCAP2_SHA3
    features_.sha3 = hwcaps2 & HWCAP2_SHA3;
#endif
#ifdef HWCAP2_SM3
    features_.sm3 = hwcaps2 & HWCAP2_SM3;
#endif
#ifdef HWCAP2_SM4
    features_.sm4 = hwcaps2 & HWCAP2_SM4;
#endif
#ifdef HWCAP2_SHA512
    features_.sha512 = hwcaps2 & HWCAP2_SHA512;
#endif
#ifdef HWCAP2_SVE2
    features_.sve2 = hwcaps2 & HWCAP2_SVE2;
#endif
#ifdef HWCAP2_SVEAES
    features_.sve_aes = hwcaps2 & HWCAP2_SVEAES;
#endif
#ifdef HWCAP2_SVEPMULL
    features_.sve_pmull = hwcaps2 & HWCAP2_SVEPMULL;
#endif
#ifdef HWCAP2_SVESM4
    features_.sm4 = features_.sm4 || (hwcaps2 & HWCAP2_SVESM4);
#endif
#ifdef HWCAP2_I8MM
    features_.i8mm = hwcaps2 & HWCAP2_I8MM;
#endif
#ifdef HWCAP2_BF16
    features_.bf16 = hwcaps2 & HWCAP2_BF16;
#endif
#ifdef HWCAP2_BTI
    features_.bti = hwcaps2 & HWCAP2_BTI;
#endif
#ifdef HWCAP2_MTE
    features_.mte = hwcaps2 & HWCAP2_MTE;
#endif
    
#elif defined(__APPLE__)
    // macOS ARM feature detection
    int val = 0;
    size_t size = sizeof(val);
    
    // Check for NEON (always present on Apple Silicon)
    features_.neon = true;
    features_.asimd = true;
    features_.fp = true;
    
    // Check for specific features
    if (sysctlbyname("hw.optional.arm.FEAT_AES", &val, &size, NULL, 0) == 0 && val) {
        features_.aes = true;
    }
    if (sysctlbyname("hw.optional.arm.FEAT_SHA1", &val, &size, NULL, 0) == 0 && val) {
        features_.sha1 = true;
    }
    if (sysctlbyname("hw.optional.arm.FEAT_SHA256", &val, &size, NULL, 0) == 0 && val) {
        features_.sha2 = true;
    }
    if (sysctlbyname("hw.optional.armv8_crc32", &val, &size, NULL, 0) == 0 && val) {
        features_.crc32 = true;
    }
    if (sysctlbyname("hw.optional.arm.FEAT_LSE", &val, &size, NULL, 0) == 0 && val) {
        features_.lse = true;
        features_.atomics = true;
    }
    if (sysctlbyname("hw.optional.arm.FEAT_DotProd", &val, &size, NULL, 0) == 0 && val) {
        features_.dotprod = true;
    }
    if (sysctlbyname("hw.optional.arm.FEAT_FP16", &val, &size, NULL, 0) == 0 && val) {
        features_.fp16 = true;
    }
#endif
}

void CPUInfo::detectARMCacheInfo() {
#ifdef __linux__
    // Try to read cache information from sysfs
    auto readCacheSize = [](const std::string& path) -> uint32_t {
        std::ifstream file(path);
        if (file.is_open()) {
            std::string size_str;
            std::getline(file, size_str);
            // Remove 'K' suffix if present
            if (!size_str.empty() && size_str.back() == 'K') {
                size_str.pop_back();
            }
            try {
                return std::stoul(size_str);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    };
    
    // Read L1 data cache
    cache_info_.l1_data_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index0/size");
    
    // Read L1 instruction cache
    cache_info_.l1_instruction_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index1/size");
    
    // Read L2 cache
    cache_info_.l2_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index2/size");
    
    // Read L3 cache (if available)
    cache_info_.l3_size = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index3/size");
    
    // Read cache line size
    std::ifstream coherency_file("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size");
    if (coherency_file.is_open()) {
        coherency_file >> cache_info_.cache_line_size;
    }
    
#elif defined(__APPLE__)
    // macOS cache detection
    uint64_t val64;
    size_t size = sizeof(val64);
    
    if (sysctlbyname("hw.l1dcachesize", &val64, &size, NULL, 0) == 0) {
        cache_info_.l1_data_size = val64 / 1024;
    }
    if (sysctlbyname("hw.l1icachesize", &val64, &size, NULL, 0) == 0) {
        cache_info_.l1_instruction_size = val64 / 1024;
    }
    if (sysctlbyname("hw.l2cachesize", &val64, &size, NULL, 0) == 0) {
        cache_info_.l2_size = val64 / 1024;
    }
    if (sysctlbyname("hw.l3cachesize", &val64, &size, NULL, 0) == 0) {
        cache_info_.l3_size = val64 / 1024;
    }
    if (sysctlbyname("hw.cachelinesize", &val64, &size, NULL, 0) == 0) {
        cache_info_.cache_line_size = val64;
    }
#endif
}

void CPUInfo::detectARMTopology() {
    // Use std::thread::hardware_concurrency() as a fallback
    processor_info_.logical_cores = std::thread::hardware_concurrency();
    
#ifdef __linux__
    // Count physical cores from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        int processor_count = 0;
        
        while (std::getline(cpuinfo, line)) {
            if (line.find("processor") != std::string::npos && line.find(':') != std::string::npos) {
                processor_count++;
            }
        }
        
        if (processor_count > 0) {
            processor_info_.logical_cores = processor_count;
        }
    }
    
    // ARM processors typically don't have hyperthreading
    processor_info_.physical_cores = processor_info_.logical_cores;
    
#elif defined(__APPLE__)
    uint32_t val;
    size_t size = sizeof(val);
    
    if (sysctlbyname("hw.physicalcpu", &val, &size, NULL, 0) == 0) {
        processor_info_.physical_cores = val;
    }
    if (sysctlbyname("hw.logicalcpu", &val, &size, NULL, 0) == 0) {
        processor_info_.logical_cores = val;
    }
    
    // Get frequency information on macOS
    uint64_t freq;
    size = sizeof(freq);
    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) == 0) {
        processor_info_.base_frequency_mhz = freq / 1000000;
    }
    if (sysctlbyname("hw.cpufrequency_max", &freq, &size, NULL, 0) == 0) {
        processor_info_.max_frequency_mhz = freq / 1000000;
    }
#endif
    
    // Ensure physical cores is set
    if (processor_info_.physical_cores == 0) {
        processor_info_.physical_cores = processor_info_.logical_cores;
    }
}

#else

// x86/x64 detection code (unchanged)

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

#endif
