// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CONTIGUOUS_BUFFER_H_
#define __MCFCRT_ENV_CONTIGUOUS_BUFFER_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

typedef struct __MCFCRT_tagContiguousBuffer {
	void *__pData;
	_MCFCRT_STD size_t __uCapacity;
	void *__pBegin;
	void *__pEnd;
} _MCFCRT_ContiguousBuffer;

extern bool _MCFCRT_ContiguousBufferReserve(void **restrict __ppData, _MCFCRT_STD size_t *restrict __puSizeReserved, _MCFCRT_STD size_t __uSizeToReserve) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_ContiguousBufferAdopt(_MCFCRT_STD size_t __uSizeToAdopt) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
