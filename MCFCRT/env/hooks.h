// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HOOKS_H_
#define __MCFCRT_ENV_HOOKS_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// heap.c
extern void (*__MCFCRT_OnHeapAlloc)(void *__pNewBlock, _MCFCRT_STD size_t __uSize, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern void (*__MCFCRT_OnHeapRealloc)(void *__pNewBlock, void *__pBlock, _MCFCRT_STD size_t __uSize, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern void (*__MCFCRT_OnHeapFree)(void *__pBlock, const void *__pRetAddr) _MCFCRT_NOEXCEPT;
extern bool (*__MCFCRT_OnHeapBadAlloc)(const void *__pRetAddr) _MCFCRT_NOEXCEPT;

#ifdef __cplusplus
#	include <cxxabi.h>

// cxa_throw.cpp
extern void (*__MCFCRT_OnException)(void *__pException, const std::type_info &__tiType, const void *__pRetAddr);

#endif // __cplusplus

__MCFCRT_EXTERN_C_END

#endif
