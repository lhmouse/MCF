// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_HOOKS_H_
#define __MCFCRT_ENV_HOOKS_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

// heap.c
typedef void (*_MCFCRT_HeapAllocCallback    )(void *__pNewBlock, _MCFCRT_STD size_t __uSize, const void *__pRetAddr);
typedef void (*_MCFCRT_HeapReallocCallback  )(void *__pNewBlock, void *__pBlock, _MCFCRT_STD size_t __uSize, const void *__pRetAddr);
typedef void (*_MCFCRT_HeapFreeCallback     )(void *__pBlock, const void *__pRetAddr);
typedef bool (*_MCFCRT_HeapBadAllocCallback )(const void *__pRetAddr);

extern volatile _MCFCRT_HeapAllocCallback    _MCFCRT_pfnOnHeapAlloc;
extern volatile _MCFCRT_HeapReallocCallback  _MCFCRT_pfnOnHeapRealloc;
extern volatile _MCFCRT_HeapFreeCallback     _MCFCRT_pfnOnHeapFree;
extern volatile _MCFCRT_HeapBadAllocCallback _MCFCRT_pfnOnHeapBadAlloc;

_MCFCRT_EXTERN_C_END

#endif
