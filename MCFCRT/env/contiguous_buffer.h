// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CONTIGUOUS_BUFFER_H_
#define __MCFCRT_ENV_CONTIGUOUS_BUFFER_H_

#include "_crtdef.h"
#include "../ext/assert.h"

_MCFCRT_EXTERN_C_BEGIN

typedef struct __MCFCRT_tagContiguousBuffer {
	void *__pStorageBegin;
	void *__pDataBegin;
	void *__pDataEnd;
	void *__pStorageEnd;
} _MCFCRT_ContiguousBuffer;

static inline void _MCFCRT_ContiguousBufferPeek(const _MCFCRT_ContiguousBuffer *restrict __pBuffer, void **restrict __ppData, _MCFCRT_STD size_t *restrict __puSize) _MCFCRT_NOEXCEPT {
	char *const __pchDataBegin = (char *)__pBuffer->__pDataBegin;
	char *const __pchDataEnd   = (char *)__pBuffer->__pDataEnd;

	*__ppData = __pchDataBegin;
	*__puSize = (_MCFCRT_STD size_t)(__pchDataEnd - __pchDataBegin);
}
static inline void _MCFCRT_ContiguousBufferDiscard(_MCFCRT_ContiguousBuffer *restrict __pBuffer, _MCFCRT_STD size_t __uSizeToDiscard) _MCFCRT_NOEXCEPT {
	char *const __pchDataBegin = __pBuffer->__pDataBegin;
	char *const __pchDataEnd   = __pBuffer->__pDataEnd;

	_MCFCRT_ASSERT(__uSizeToDiscard <= (_MCFCRT_STD size_t)(__pchDataEnd - __pchDataBegin));

	__pBuffer->__pDataBegin = __pchDataBegin + __uSizeToDiscard;
}
static inline void _MCFCRT_ContiguousBufferAdopt(_MCFCRT_ContiguousBuffer *restrict __pBuffer, _MCFCRT_STD size_t __uSizeToAdopt) _MCFCRT_NOEXCEPT {
	char *const __pchDataEnd    = __pBuffer->__pDataEnd;
	char *const __pchStorageEnd = __pBuffer->__pStorageEnd;

	_MCFCRT_ASSERT(__uSizeToAdopt <= (_MCFCRT_STD size_t)(__pchStorageEnd - __pchDataEnd));

	__pBuffer->__pDataEnd = __pchDataEnd + __uSizeToAdopt;
}

extern bool _MCFCRT_ContiguousBufferReserve(_MCFCRT_ContiguousBuffer *restrict __pBuffer, void **restrict __ppData, _MCFCRT_STD size_t *restrict __puSizeReserved, _MCFCRT_STD size_t __uSizeToReserve) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_ContiguousBufferRecycle(_MCFCRT_ContiguousBuffer *restrict __pBuffer) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
