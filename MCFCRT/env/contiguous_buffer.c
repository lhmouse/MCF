// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "contiguous_buffer.h"
#include "heap.h"
#include "mcfwin.h"
#include "../ext/assert.h"

void _MCFCRT_ContiguousBufferPeek(const _MCFCRT_ContiguousBuffer *restrict pBuffer, void **restrict ppData, size_t *restrict puSize){
	char *const pchDataBegin = pBuffer->__pDataBegin;
	char *const pchDataEnd   = pBuffer->__pDataEnd;

	*ppData = pchDataBegin;
	*puSize = (size_t)(pchDataEnd - pchDataBegin);
}
void _MCFCRT_ContiguousBufferDiscard(_MCFCRT_ContiguousBuffer *restrict pBuffer, size_t uSizeToDiscard){
	char *const pchDataBegin = pBuffer->__pDataBegin;
	char *const pchDataEnd   = pBuffer->__pDataEnd;

	_MCFCRT_ASSERT(uSizeToDiscard <= (size_t)(pchDataEnd - pchDataBegin));

	pBuffer->__pDataBegin = pchDataBegin + uSizeToDiscard;
}

bool _MCFCRT_ContiguousBufferReserve(_MCFCRT_ContiguousBuffer *restrict pBuffer, void **restrict ppData, size_t *restrict puSizeReserved, size_t uSizeToReserve){
	char *pchStorageBegin = pBuffer->__pStorageBegin;
	char *pchDataBegin    = pBuffer->__pDataBegin;
	char *pchDataEnd      = pBuffer->__pDataEnd;
	char *pchStorageEnd   = pBuffer->__pStorageEnd;

	if(uSizeToReserve <= (size_t)(pchStorageEnd - pchDataEnd)){
		// 无事可做。
	} else {
		const size_t uOldSize = (size_t)(pchDataEnd - pchDataBegin);
		const size_t uMinNewCapacity = uOldSize + uSizeToReserve;
		if(uMinNewCapacity < uOldSize){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		const size_t uOldCapacity = (size_t)(pchStorageEnd - pchStorageBegin);
		if(uMinNewCapacity <= uOldCapacity){
			memmove(pchStorageBegin, pchDataBegin, uOldSize);

			pchDataBegin = pchStorageBegin;
			pchDataEnd   = pchDataBegin + uOldSize;

			pBuffer->__pDataBegin = pchDataBegin;
			pBuffer->__pDataEnd   = pchDataEnd;
		} else {
			size_t uNewCapacity = uOldCapacity + 1;
			uNewCapacity += (uNewCapacity >> 1);
			uNewCapacity = (uNewCapacity + 0x0F) & (size_t)-0x10;
			if(uNewCapacity < uMinNewCapacity){
				uNewCapacity = uMinNewCapacity;
			}
			char *const pchNewStorage = _MCFCRT_malloc(uNewCapacity);
			if(!pchNewStorage){
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return false;
			}
			memcpy(pchNewStorage, pchDataBegin, uOldSize);
			_MCFCRT_free(pchStorageBegin);

			pchStorageBegin = pchNewStorage;
			pchDataBegin    = pchNewStorage;
			pchDataEnd      = pchDataBegin + uOldSize;
			pchStorageEnd   = pchStorageBegin + uNewCapacity;

			pBuffer->__pStorageBegin = pchStorageBegin;
			pBuffer->__pDataBegin    = pchDataBegin;
			pBuffer->__pDataEnd      = pchDataEnd;
			pBuffer->__pStorageEnd   = pchStorageEnd;
		}
	}
	_MCFCRT_ASSERT(uSizeToReserve <= (size_t)(pchStorageEnd - pchDataEnd));

	*ppData         = pchDataEnd;
	*puSizeReserved = (size_t)(pchStorageEnd - pchDataEnd);
	return true;
}
void _MCFCRT_ContiguousBufferAdopt(_MCFCRT_ContiguousBuffer *restrict pBuffer, size_t uSizeToAdopt){
	char *const pchDataEnd    = pBuffer->__pDataEnd;
	char *const pchStorageEnd = pBuffer->__pStorageEnd;

	_MCFCRT_ASSERT(uSizeToAdopt <= (size_t)(pchStorageEnd - pchDataEnd));

	pBuffer->__pDataEnd = pchDataEnd + uSizeToAdopt;
}

void _MCFCRT_ContiguousBufferRecycle(_MCFCRT_ContiguousBuffer *restrict pBuffer){
	char *const pchStorageBegin = pBuffer->__pStorageBegin;

	_MCFCRT_free(pchStorageBegin);

	pBuffer->__pStorageBegin = nullptr;
	pBuffer->__pDataBegin    = nullptr;
	pBuffer->__pDataEnd      = nullptr;
	pBuffer->__pStorageEnd   = nullptr;
}
