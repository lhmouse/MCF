// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "cpu.h"
#include "expect.h"
#include "once_flag.h"
#include "bail.h"
#include "xassert.h"
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

static const uint16_t g_fcw_init   = (RND_NEAREST << 10) | (PRCS_EXTENDED << 8) | ((EXCEPT_PM | EXCEPT_UM | EXCEPT_DM) << 0);
static const uint32_t g_mxcsr_init = (RND_NEAREST << 13) |                        ((EXCEPT_PM | EXCEPT_UM | EXCEPT_DM) << 7);

void __MCFCRT_CpuResetFloatingPointEnvironment(void){
	__asm__ volatile (
		"fninit \n"
		"fldcw %0 \n"
		"ldmxcsr %1 \n"
		:
		: "m"(g_fcw_init), "m"(g_mxcsr_init)
	);
}

static _MCFCRT_OnceFlag g_once;
static unsigned g_cache_sizes[_MCFCRT_kCpuCacheLevelMax + 1];

static void FetchCpuInfoOnce(void){
	const _MCFCRT_OnceResult result = _MCFCRT_WaitForOnceFlagForever(&g_once);
	if(result == _MCFCRT_kOnceResultFinished){
		return;
	}
	_MCFCRT_ASSERT(result == _MCFCRT_kOnceResultInitial);

	// Reference:
	//   Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 2 (2A, 2B & 2C):
	//     Table 3-17. Information Returned by CPUID Instruction
	// Iterate from cache level 1, until there are no more levels.
	unsigned level = 1;
	do {
		unsigned eax, ebx, ecx, edx;
		__cpuid_count(0x04, level, eax, ebx, ecx, edx);
		if((eax & 0x1F) == 0){
			// No more cache levels. Stop.
			break;
		}
		const unsigned ways = ((ebx >> 22) & 0x3FF) + 1;
		const unsigned partitions = ((ebx >> 12) & 0x3FF) + 1;
		const unsigned line_size = (ebx & 0xFFF) + 1;
		const unsigned sets = ecx + 1;
		g_cache_sizes[level] = ways * partitions * line_size * sets;
	} while(++level < _MCFCRT_kCpuCacheLevelMax);
	// Set up boundary values.
	g_cache_sizes[_MCFCRT_kCpuCacheLevelMin] = g_cache_sizes[_MCFCRT_kCpuCacheLevel1];
	g_cache_sizes[_MCFCRT_kCpuCacheLevelMax] = g_cache_sizes[level - 1];

	_MCFCRT_SignalOnceFlagAsFinished(&g_once);
}

size_t _MCFCRT_CpuGetCacheSize(_MCFCRT_CpuCacheLevel level){
	if(_MCFCRT_EXPECT_NOT((unsigned)level > _MCFCRT_kCpuCacheLevelMax)){
		return 0;
	}
	FetchCpuInfoOnce();
	return g_cache_sizes[level];
}
