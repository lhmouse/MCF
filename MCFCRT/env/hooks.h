// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_HOOKS_H_
#define MCF_CRT_HOOKS_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

// heap.c
extern bool MCF_OnBadAlloc() MCF_NOEXCEPT
	__attribute__((__weak__));

// heap_dbg.c
extern void MCF_OnHeapAlloc(void *pBlock, MCF_STD size_t uBytes, const void *pRetAddr) MCF_NOEXCEPT
	__attribute__((__weak__));
extern void MCF_OnHeapDealloc(void *pBlock, const void *pRetAddr) MCF_NOEXCEPT
	__attribute__((__weak__));

__MCF_CRT_EXTERN_C_END

#ifdef __cplusplus
#	include <cxxabi.h>

// cxa_throw.cpp
extern void MCF_OnException(void *pException, const std::type_info &tiType, const void *pRetAddr) noexcept
	__attribute__((__weak__));

#endif // __cplusplus

#endif
