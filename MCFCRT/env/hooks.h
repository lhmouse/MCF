// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_HOOKS_H_
#define MCF_CRT_HOOKS_H_

#include "_crtdef.h"

__MCF_CRT_EXTERN_C_BEGIN

// heap.c
extern void (*__MCF_OnHeapAlloc)(void *pNewBlock, MCF_STD size_t uSize, const void *pRetAddr) MCF_NOEXCEPT;
extern void (*__MCF_OnHeapRealloc)(void *pNewBlock, void *pBlock, MCF_STD size_t uSize, const void *pRetAddr) MCF_NOEXCEPT;
extern void (*__MCF_OnHeapDealloc)(void *pBlock, const void *pRetAddr) MCF_NOEXCEPT;
extern bool (*__MCF_OnBadAlloc)(const void *pRetAddr) MCF_NOEXCEPT;

#ifdef __cplusplus
#	include <cxxabi.h>

// cxa_throw.cpp
extern void (*__MCF_OnException)(void *pException, const std::type_info &tiType, const void *pRetAddr);

#endif // __cplusplus

__MCF_CRT_EXTERN_C_END

#endif
