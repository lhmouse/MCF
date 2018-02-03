// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "cpu.h"
#include "expect.h"
#include "bail.h"
#include <cpuid.h>

#define RND_NEAREST     (0u)            // 四舍六入五凑双。
#define RND_DOWN        (1u)            // 向负无穷舍入。
#define RND_UP          (2u)            // 向正无穷舍入。
#define RND_ZERO        (3u)            // 向零舍入。

#define PRCS_SINGLE     (0u)            // 24 位。
#define PRCS_DOUBLE     (2u)            // 53 位。
#define PRCS_EXTENDED   (3u)            // 64 位。

#define EXCEPT_PM       (1u << 5)       // 精度损失异常。
#define EXCEPT_UM       (1u << 4)       // 下溢异常。
#define EXCEPT_OM       (1u << 3)       // 上溢异常。
#define EXCEPT_ZM       (1u << 2)       // 除以零异常。
#define EXCEPT_DM       (1u << 1)       // 非规格化数异常。
#define EXCEPT_IM       (1u << 0)       // 无效操作异常。

static const uint16_t s_fcw_init   = (RND_NEAREST << 10) | (PRCS_EXTENDED << 8) | ((EXCEPT_PM | EXCEPT_UM | EXCEPT_DM) << 0);
static const uint32_t s_mxcsr_init = (RND_NEAREST << 13) |                        ((EXCEPT_PM | EXCEPT_UM | EXCEPT_DM) << 7);

void __MCFCRT_CpuResetFloatingPointEnvironment(void){
	__asm__ volatile (
		"fnclex \n"
		"fldcw %0 \n"
		"ldmxcsr %1 \n"
		:
		: "m"(s_fcw_init), "m"(s_mxcsr_init)
	);
}

static volatile size_t s_cache_sizes[4];

size_t _MCFCRT_CpuGetCacheSize(_MCFCRT_CpuCacheLevel level){
	if((unsigned)level >= 4){
		return 0;
	}
	size_t size = __atomic_load_n(s_cache_sizes + level, __ATOMIC_RELAXED);
	if(_MCFCRT_EXPECT_NOT(size == 0)){
		// Reference:
		//   Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 2 (2A, 2B & 2C):
		//     Table 3-17. Information Returned by CPUID Instruction
		unsigned eax, ebx, ecx, edx;
		if(!__get_cpuid_count(0x04, level, &eax, &ebx, &ecx, &edx)){
			_MCFCRT_Bail(L"__get_cpuid_count() 失败。");
		}
		const unsigned ways = ((ebx >> 22) & 0x3FF) + 1;
		const unsigned partitions = ((ebx >> 12) & 0x3FF) + 1;
		const unsigned line_size = (ebx & 0xFFF) + 1;
		const unsigned sets = ecx + 1;
		size = (size_t)ways * partitions * line_size * sets;
		__atomic_store_n(s_cache_sizes + level, size, __ATOMIC_RELAXED);
	}
	return size;
}
