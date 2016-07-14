// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "contiguous_buffer.h"
#include "heap.h"

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
void _MCFCRT_ContiguousBufferRecycle(_MCFCRT_ContiguousBuffer *restrict pBuffer){
	char *const pchStorageBegin = pBuffer->__pStorageBegin;

	_MCFCRT_free(pchStorageBegin);

	pBuffer->__pStorageBegin = nullptr;
	pBuffer->__pDataBegin    = nullptr;
	pBuffer->__pDataEnd      = nullptr;
	pBuffer->__pStorageEnd   = nullptr;
}
