// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_ENV_HOOKS_H_
#define __MCF_CRT_ENV_HOOKS_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

// heap.c
extern void (*__MCF_CRT_OnHeapAlloc)(void *__pNewBlock, MCF_STD size_t __uSize, const void *__pRetAddr) MCF_NOEXCEPT;
extern void (*__MCF_CRT_OnHeapRealloc)(void *__pNewBlock, void *__pBlock, MCF_STD size_t __uSize, const void *__pRetAddr) MCF_NOEXCEPT;
extern void (*__MCF_CRT_OnHeapFree)(void *__pBlock, const void *__pRetAddr) MCF_NOEXCEPT;
extern bool (*__MCF_CRT_OnHeapBadAlloc)(const void *__pRetAddr) MCF_NOEXCEPT;

#ifdef __cplusplus
#	include <cxxabi.h>

// cxa_throw.cpp
extern void (*__MCF_CRT_OnException)(void *__pException, const std::type_info &__tiType, const void *__pRetAddr);

#endif // __cplusplus

__MCF_CRT_EXTERN_C_END

#endif
