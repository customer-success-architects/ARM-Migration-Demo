#pragma once

#include <string>
#include <cstdint>
#include <array>

class CPUInfo {
public:
    struct Features {
#if defined(TARGET_ARCH_X86) || (!defined(TARGET_ARCH_ARM64) && !defined(TARGET_ARCH_ARM) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)))
        // x86 SIMD Instructions
        bool mmx = false;
        bool sse = false;
        bool sse2 = false;
        bool sse3 = false;
        bool ssse3 = false;
        bool sse4_1 = false;
        bool sse4_2 = false;
        bool avx = false;
        bool avx2 = false;
        bool avx512f = false;
        bool avx512dq = false;
        bool avx512bw = false;
        bool avx512vl = false;
        bool fma = false;
        bool fma4 = false;
        
        // x86 Cryptographic
        bool aes = false;
        bool sha = false;
        bool pclmulqdq = false;
        
        // x86 Virtualization
        bool vmx = false;  // Intel VT-x
        bool svm = false;  // AMD-V
        
        // x86 Security
        bool nx = false;
        bool smep = false;
        bool smap = false;
        bool sgx = false;
        
        // x86 Other
        bool rdrand = false;
        bool rdseed = false;
        bool popcnt = false;
        bool bmi1 = false;
        bool bmi2 = false;
        bool tsc = false;
        bool x87_fpu = false;
#else
        // ARM SIMD Instructions (NEON/Advanced SIMD)
        bool neon = false;
        bool asimd = false;  // Advanced SIMD
        
        // ARM Floating Point
        bool fp = false;     // Hardware floating point
        bool fphp = false;   // Half-precision FP
        bool asimdhp = false;// Advanced SIMD half-precision
        
        // ARM Cryptographic Extensions
        bool aes = false;
        bool sha1 = false;
        bool sha2 = false;
        bool sha3 = false;
        bool sha512 = false;
        bool crc32 = false;
        bool pmull = false;
        
        // ARM Atomics and Memory
        bool atomics = false;  // Large System Extensions (LSE)
        bool lse = false;      // Large System Extensions
        
        // ARM SVE (Scalable Vector Extensions)
        bool sve = false;
        bool sve2 = false;
        uint32_t sve_vector_length = 0;  // in bits
        
        // ARM Other features
        bool bf16 = false;     // BFloat16
        bool i8mm = false;     // Int8 matrix multiply
        bool dotprod = false;  // Dot product
        bool rdma = false;     // Rounding Double Multiply Accumulate
        bool jscvt = false;    // JavaScript conversion
        bool fcma = false;     // Floating-point complex
        bool dcpop = false;    // DC POP (Data cache clean to PoP)
        bool ssbs = false;     // Speculative Store Bypass Safe
        bool sb = false;       // Speculation Barrier
#endif
    };
    
    struct CacheInfo {
        uint32_t l1_data_size = 0;      // in KB
        uint32_t l1_instruction_size = 0;
        uint32_t l2_size = 0;
        uint32_t l3_size = 0;
        uint32_t cache_line_size = 0;   // in bytes
    };
    
    struct ProcessorInfo {
        std::string vendor;
        std::string brand;
        std::string architecture;
        uint32_t family = 0;
        uint32_t model = 0;
        uint32_t stepping = 0;
        uint32_t physical_cores = 0;
        uint32_t logical_cores = 0;
        uint32_t base_frequency_mhz = 0;
        uint32_t max_frequency_mhz = 0;
#if defined(TARGET_ARCH_ARM64) || defined(TARGET_ARCH_ARM) || defined(__aarch64__) || defined(__arm__)
        uint32_t midr = 0;        // Main ID Register
        uint32_t revidr = 0;      // Revision ID Register
        std::string implementer;  // CPU implementer (ARM, Apple, Qualcomm, etc.)
        std::string part_name;    // CPU part name
#endif
    };

    CPUInfo();
    
    void detect();
    
    const Features& getFeatures() const { return features_; }
    const CacheInfo& getCacheInfo() const { return cache_info_; }
    const ProcessorInfo& getProcessorInfo() const { return processor_info_; }
    
    bool isARM() const;
    bool isX86() const;

private:
    Features features_;
    CacheInfo cache_info_;
    ProcessorInfo processor_info_;
    
#if defined(TARGET_ARCH_X86) || (!defined(TARGET_ARCH_ARM64) && !defined(TARGET_ARCH_ARM) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)))
    uint32_t max_basic_leaf_ = 0;
    uint32_t max_extended_leaf_ = 0;
    
    void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx);
    void detectVendor();
    void detectBrand();
    void detectFeatures();
    void detectCacheInfo();
    void detectTopology();
    void detectFrequency();
#else
    void detectARMInfo();
    void detectARMFeatures();
    void detectARMCache();
    void detectARMTopology();
    std::string getARMImplementerName(uint8_t implementer);
    std::string getARMPartName(uint8_t implementer, uint16_t part);
#endif
};
