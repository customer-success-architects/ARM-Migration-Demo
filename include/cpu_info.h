#pragma once

#include <string>
#include <cstdint>
#include <array>

class CPUInfo {
public:
    struct Features {
#ifdef ARM_BUILD
        // ARM SIMD Instructions
        bool neon = false;
        bool sve = false;
        bool sve2 = false;
        
        // ARM Cryptographic
        bool aes = false;
        bool sha1 = false;
        bool sha2 = false;
        bool sha3 = false;
        bool sha512 = false;
        bool pmull = false;
        
        // ARM Floating Point
        bool fp = false;
        bool fp16 = false;
        bool bf16 = false;
        
        // ARM Atomics and Memory
        bool atomics = false;
        bool crc32 = false;
        bool lse = false;
        
        // ARM Other Features
        bool dotprod = false;
        bool i8mm = false;
        bool jscvt = false;
        bool fcma = false;
        bool frint = false;
        bool sb = false;
        bool ssbs = false;
        bool bti = false;
        bool mte = false;
#else
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
    void detectARMInfo();
    void detectARMFeatures();
    void detectARMCacheInfo();
    void detectARMTopology();
    void detectARMFrequency();
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
