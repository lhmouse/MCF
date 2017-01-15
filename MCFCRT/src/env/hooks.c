// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "hooks.h"

volatile _MCFCRT_HeapAllocCallback    _MCFCRT_pfnOnHeapAlloc     = _MCFCRT_NULLPTR;
volatile _MCFCRT_HeapReallocCallback  _MCFCRT_pfnOnHeapRealloc   = _MCFCRT_NULLPTR;
volatile _MCFCRT_HeapFreeCallback     _MCFCRT_pfnOnHeapFree      = _MCFCRT_NULLPTR;
volatile _MCFCRT_HeapBadAllocCallback _MCFCRT_pfnOnHeapBadAlloc  = _MCFCRT_NULLPTR;
