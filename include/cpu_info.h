#pragma once

#include <string>
#include <cstdint>
#include <array>

class CPUInfo {
public:
#ifdef ARM_BUILD
    struct Features {
        // NEON (Advanced SIMD) Instructions
        bool neon = false;
        bool neon_fp16 = false;
        bool neon_bf16 = false;
        bool neon_dotprod = false;
        bool neon_i8mm = false;
        
        // SVE (Scalable Vector Extension)
        bool sve = false;
        bool sve2 = false;
        uint32_t sve_vector_length = 0;  // in bits
        
        // Cryptographic Extensions
        bool aes = false;
        bool sha1 = false;
        bool sha2 = false;
        bool sha3 = false;
        bool sha512 = false;
        bool pmull = false;
        
        // Atomic Operations
        bool atomics = false;  // LSE (Large System Extensions)
        
        // Memory Tagging
        bool mte = false;
        
        // Other Features
        bool crc32 = false;
        bool fp = false;        // Floating Point
        bool asimd = false;     // Advanced SIMD
        bool asimdhp = false;   // Half-precision SIMD
        bool asimddp = false;   // Dot Product
        bool dcpop = false;     // Data cache clean to Point of Persistence
        bool fphp = false;      // Half-precision floating point
        bool jscvt = false;     // JavaScript conversion
        bool fcma = false;      // Complex number support
        bool lrcpc = false;     // Load-Acquire RCpc Register
        bool ilrcpc = false;    // Load-Acquire RCpc Register v2
        bool flagm = false;     // Condition flag manipulation
        bool ssbs = false;      // Speculative Store Bypass Safe
        bool sb = false;        // Speculation Barrier
        bool paca = false;      // Pointer Authentication (address)
        bool pacg = false;      // Pointer Authentication (generic)
        bool bti = false;       // Branch Target Identification
    };
#else
    struct Features {
        // SIMD Instructions
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
        
        // Cryptographic
        bool aes = false;
        bool sha = false;
        bool pclmulqdq = false;
        
        // Virtualization
        bool vmx = false;  // Intel VT-x
        bool svm = false;  // AMD-V
        
        // Security
        bool nx = false;
        bool smep = false;
        bool smap = false;
        bool sgx = false;
        
        // Other
        bool rdrand = false;
        bool rdseed = false;
        bool popcnt = false;
        bool bmi1 = false;
        bool bmi2 = false;
        bool tsc = false;
        bool x87_fpu = false;
    };
#endif
    
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
        uint32_t family = 0;
        uint32_t model = 0;
        uint32_t stepping = 0;
        uint32_t physical_cores = 0;
        uint32_t logical_cores = 0;
        uint32_t base_frequency_mhz = 0;
        uint32_t max_frequency_mhz = 0;
#ifdef ARM_BUILD
        uint32_t implementer = 0;      // ARM implementer code
        uint32_t variant = 0;          // Variant number
        uint32_t part = 0;             // Part number
        uint32_t revision = 0;         // Revision number
#endif
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
    
#ifdef ARM_BUILD
    void detectARMVendor();
    void detectARMBrand();
    void detectARMFeatures();
    void detectARMCacheInfo();
    void detectARMTopology();
#else
    uint32_t max_basic_leaf_ = 0;
    uint32_t max_extended_leaf_ = 0;
    
    void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx);
    void detectVendor();
    void detectBrand();
    void detectFeatures();
    void detectCacheInfo();
    void detectTopology();
    void detectFrequency();
#endif
};
