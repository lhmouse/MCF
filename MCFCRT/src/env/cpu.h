// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CPU_H_
#define __MCFCRT_ENV_CPU_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern void __MCFCRT_CpuResetFloatingPointEnvironment(void) _MCFCRT_NOEXCEPT;

typedef enum __MCFCRT_tagCpuCacheLevel {
	_MCFCRT_kCpuCacheLevelMin = 0,
	_MCFCRT_kCpuCacheLevel1   = 1,
	_MCFCRT_kCpuCacheLevel2   = 2,
	_MCFCRT_kCpuCacheLevel3   = 3,
	_MCFCRT_kCpuCacheLevelMax = 7,
} _MCFCRT_CpuCacheLevel;

// For `_MCFCRT_kCpuCacheLevelMin`: Returns the size of the first level of cache.
// For `_MCFCRT_kCpuCacheLevel{1,2,3,4}` : Returns the size of the specified level of cache.
// For `_MCFCRT_kCpuCacheLevelMax` : Returns the size of the last level of cache.
extern _MCFCRT_STD size_t _MCFCRT_CpuGetCacheSize(_MCFCRT_CpuCacheLevel __level) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
