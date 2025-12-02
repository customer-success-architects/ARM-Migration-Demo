#pragma once

#include <string>
#include <cstdint>
#include <array>

class CPUInfo {
public:
    struct Features {
        // ARM SIMD Instructions
        bool neon = false;           // ARM NEON (Advanced SIMD)
        bool asimd = false;          // Advanced SIMD
        bool sve = false;            // Scalable Vector Extension
        bool sve2 = false;           // SVE2
        bool fp = false;             // Floating Point
        bool fp16 = false;           // Half-precision floating point
        bool bf16 = false;           // Brain Float16
        bool i8mm = false;           // Int8 Matrix Multiply
        
        // Cryptographic
        bool aes = false;            // AES instructions
        bool pmull = false;          // Polynomial Multiply Long
        bool sha1 = false;           // SHA1 instructions
        bool sha2 = false;           // SHA2 instructions
        bool sha3 = false;           // SHA3 instructions
        bool sha512 = false;         // SHA512 instructions
        bool sm3 = false;            // SM3 instructions
        bool sm4 = false;            // SM4 instructions
        
        // Atomic & Memory
        bool atomics = false;        // Large System Extensions (LSE)
        bool lse = false;            // Atomic instructions
        bool lse2 = false;           // LSE2
        bool dcpop = false;          // Data cache clean to PoP
        
        // Security & Virtualization
        bool sve_aes = false;        // SVE AES
        bool sve_pmull = false;      // SVE polynomial multiply
        bool pauth = false;          // Pointer Authentication
        bool bti = false;            // Branch Target Identification
        bool mte = false;            // Memory Tagging Extension
        
        // Other Features
        bool crc32 = false;          // CRC32 instructions
        bool rdm = false;            // Rounding Double Multiply
        bool dotprod = false;        // Dot Product
        bool fcma = false;           // Complex number operations
        bool jscvt = false;          // JavaScript conversion
        bool frint = false;          // Floating-point rounding
        
        // x86 compatibility flags (for code that checks these)
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
        bool pclmulqdq = false;
        bool vmx = false;
        bool svm = false;
        bool nx = false;
        bool smep = false;
        bool smap = false;
        bool sgx = false;
        bool rdrand = false;
        bool rdseed = false;
        bool popcnt = false;
        bool bmi1 = false;
        bool bmi2 = false;
        bool tsc = false;
        bool x87_fpu = false;
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
        std::string implementer;     // ARM implementer
        std::string architecture;    // ARM architecture version
        std::string variant;         // CPU variant
        std::string part;            // CPU part number
        std::string revision;        // CPU revision
        uint32_t family = 0;
        uint32_t model = 0;
        uint32_t stepping = 0;
        uint32_t physical_cores = 0;
        uint32_t logical_cores = 0;
        uint32_t base_frequency_mhz = 0;
        uint32_t max_frequency_mhz = 0;
    };

    CPUInfo();
    
    void detect();
    
    const Features& getFeatures() const { return features_; }
    const CacheInfo& getCacheInfo() const { return cache_info_; }
    const ProcessorInfo& getProcessorInfo() const { return processor_info_; }

private:
    Features features_;
    CacheInfo cache_info_;
    ProcessorInfo processor_info_;
    
    uint32_t max_basic_leaf_ = 0;
    uint32_t max_extended_leaf_ = 0;
    
#ifdef __aarch64__
    void detectARM();
    void parseARMCPUInfo();
    void detectARMFeatures();
    void detectARMCacheInfo();
    void detectARMTopology();
#else
    void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx);
    void detectVendor();
    void detectBrand();
    void detectFeatures();
    void detectCacheInfo();
    void detectTopology();
    void detectFrequency();
#endif
};
