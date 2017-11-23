// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_stack.h"
#include "heap.h"
#include "last_error.h"
#include "endian.h"
#include "sha256.h"

typedef struct tagStackElement {
	size_t uSizePadded : 3;
	size_t uOffsetPrevInWords : sizeof(size_t) * 8 - 3;
	wchar_t awszString[];
} StackElement;

void MCFBUILD_StringStackInitialize(MCFBUILD_StringStack *pStack){
	pStack->pbyStorage = 0;
	pStack->uCapacity = 0;
	pStack->uOffsetTop = 0;
	pStack->uOffsetEnd = 0;
}
void MCFBUILD_StringStackUninitialize(MCFBUILD_StringStack *pStack){
	MCFBUILD_HeapFree(pStack->pbyStorage);
}

void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack){
	pStack->uOffsetTop = 0;
	pStack->uOffsetEnd = 0;
}
bool MCFBUILD_StringStackGetTop(const wchar_t **restrict ppwszString, size_t *restrict puLength, const MCFBUILD_StringStack *restrict pStack){
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	if(uOffsetTop == uOffsetEnd){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|*----------|      |
	|         ^storage                                            |
	| BEFORE                                    ^top        ^end  |
	| AFTER                                      ^string          |
	|                                            \________/length |
	\*-----------------------------------------------------------*/
	unsigned char *pbyStorage = pStack->pbyStorage;
	const StackElement *pElement = (void *)(pbyStorage + uOffsetTop);
	size_t uSizeOfString = uOffsetEnd - uOffsetTop - pElement->uSizePadded - sizeof(StackElement) - sizeof(wchar_t);
	*ppwszString = pElement->awszString;
	*puLength = uSizeOfString / sizeof(wchar_t);
	return true;
}
bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwcString, size_t uLength){
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	size_t uSizeOfString = uLength * sizeof(wchar_t);
	size_t uSizeToAdd = sizeof(StackElement) + uSizeOfString + sizeof(wchar_t);
	size_t uSizeToPad = -uSizeToAdd % 8;
	uSizeToAdd += uSizeToPad;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(uOffsetEnd, uSizeToAdd, &uMinimumSizeToReserve)){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uCapacity = pStack->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += pStack->uCapacity / 2;
		uCapacity += 0x0F;
		uCapacity &= (size_t)-0x10;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		pbyStorage = MCFBUILD_HeapRealloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			return false;
		}
		pStack->pbyStorage = pbyStorage;
		pStack->uCapacity = uCapacity;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|????????????      |
	|         ^storage                                            |
	| BEFORE                         ^top       ^end              |
	| AFTER                                     ^top        ^end  |
	\*-----------------------------------------------------------*/
	StackElement *pElement = (void *)(pbyStorage + uOffsetEnd);
	pElement->uSizePadded = uSizeToPad % 8;
	pElement->uOffsetPrevInWords = (uOffsetTop / 8) & (SIZE_MAX >> 3);
	wmemcpy(pElement->awszString, pwcString, uLength)[uLength] = 0;
	pStack->uOffsetTop = uOffsetEnd;
	pStack->uOffsetEnd = uOffsetEnd + uSizeToAdd;
	return true;
}
bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwszString){
	return MCFBUILD_StringStackPush(pStack, pwszString, wcslen(pwszString));
}
bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack){
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	if(uOffsetTop == uOffsetEnd){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|*----------|      |
	|         ^storage                                            |
	| BEFORE                                    ^top        ^end  |
	| AFTER                          ^top       ^end              |
	\*-----------------------------------------------------------*/
	unsigned char *pbyStorage = pStack->pbyStorage;
	const StackElement *pElement = (void *)(pbyStorage + uOffsetTop);
	pStack->uOffsetEnd = uOffsetTop;
	pStack->uOffsetTop = (size_t)(pElement->uOffsetPrevInWords) * 8;
	return true;
}

// In reality, members in this structure are stored in reverse order.
typedef struct tagSerializedElement {
	uint64_t u64WholeAndPadding;
	wchar_t awcReverseString[];
} SerializedElement;

typedef struct tagSerializedHeader {
	uint8_t au8Checksum[32];
	unsigned char abyPayload[];
} SerializedHeader;

// This is the salt used to create checksums. The null terminator is part of the salt.
static const char kMagic[] = "MCFBUILD_StringStack:1";

bool MCFBUILD_StringStackSerialize(void **restrict ppData, size_t *restrict puSize, const MCFBUILD_StringStack *restrict pStack){
	// Estimate the upper bound of number of bytes to allocate.
	size_t uSizeToAlloc = sizeof(SerializedHeader);
	// Iterate from top to bottom, accumulating number of bytes for each element on the way.
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uOffsetTop = pStack->uOffsetTop;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	while(uOffsetTop != uOffsetEnd){
		const StackElement *pElement = (void *)(pbyStorage + uOffsetTop);
		size_t uSizeOfString = uOffsetEnd - uOffsetTop - pElement->uSizePadded - sizeof(StackElement) - sizeof(wchar_t);
		size_t uSizeToPad = -uSizeOfString % 8;
		// There is no need to serialize null terminators.
		uSizeToAlloc += uSizeToPad;
		uSizeToAlloc += uSizeOfString;
		uSizeToAlloc += sizeof(SerializedElement);
		// Scan for the next element.
		uOffsetEnd = uOffsetTop;
		uOffsetTop = (size_t)(pElement->uOffsetPrevInWords) * 8;
	}
	// Allocate the buffer now.
	SerializedHeader *pHeader = MCFBUILD_HeapAlloc(uSizeToAlloc);
	if(!pHeader){
		return false;
	}
	// Copy strings from top to bottom.
	unsigned char *pbyWrite = pHeader->abyPayload;
	uOffsetTop = pStack->uOffsetTop;
	uOffsetEnd = pStack->uOffsetEnd;
	while(uOffsetTop != uOffsetEnd){
		const StackElement *pElement = (void *)(pbyStorage + uOffsetTop);
		size_t uSizeOfString = uOffsetEnd - uOffsetTop - pElement->uSizePadded - sizeof(StackElement) - sizeof(wchar_t);
		size_t uSizeToPad = -uSizeOfString % 8;
		// Fill padding characters with something predictable. Otherwise it will not be possible to get a meaningful checksum.
		for(size_t uIndex = 0; uIndex < uSizeToPad / sizeof(wchar_t); ++uIndex){
			MCFBUILD_store_be_uint16((wchar_t *)pbyWrite, 0xFFFF);
			pbyWrite += sizeof(wchar_t);
		}
		// Store each wide character in reverse order.
		for(size_t uIndex = uSizeOfString / sizeof(wchar_t); uIndex != 0; --uIndex){
			MCFBUILD_store_be_uint16((wchar_t *)pbyWrite, pElement->awszString[uIndex - 1]);
			pbyWrite += sizeof(wchar_t);
		}
		// This is tricky. Acknowledging that `uSizeOfString + uSizeToPad` will be aligned onto a 8-byte boundary, we add
		// another `uSizeToPad` to the result, which will have `uSizeToPad` in its three LSBs intactly, with the rest being
		// `uSizeOfString + uSizeToPad` which can be fetched by bitwise and'ing the three LSBs away.
		SerializedElement *pSerialized = (void *)pbyWrite;
		MCFBUILD_store_be_uint64(&(pSerialized->u64WholeAndPadding), uSizeOfString + uSizeToPad * 2);
		pbyWrite += sizeof(SerializedElement);
		// Scan for the next element.
		uOffsetEnd = uOffsetTop;
		uOffsetTop = (size_t)(pElement->uOffsetPrevInWords) * 8;
	}
	// Calculate the checksum.
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	MCFBUILD_Sha256Update(&vSha256Context, pHeader->abyPayload, (size_t)(pbyWrite - pHeader->abyPayload));
	MCFBUILD_Sha256Finalize(&(pHeader->au8Checksum), &vSha256Context);
	// Hand over the buffer to our caller.
	*ppData = pHeader;
	*puSize = uSizeToAlloc;
	return true;
}
void MCFBUILD_StringStackFreeSerializedBuffer(void *pData){
	MCFBUILD_HeapFree(pData);
}
bool MCFBUILD_StringStackDeserialize(MCFBUILD_StringStack *restrict pStack, void *restrict pData, size_t uSize){
	if(uSize < sizeof(SerializedHeader)){
		MCFBUILD_SetLastError(ERROR_INVALID_DATA);
		return false;
	}
	SerializedHeader *pHeader = pData;
	const unsigned char *pbyEnd = (const unsigned char *)pData + uSize;
	// Verify the checksum.
	uint8_t au8Checksum[32];
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	MCFBUILD_Sha256Update(&vSha256Context, pHeader->abyPayload, (size_t)(pbyEnd - pHeader->abyPayload));
	MCFBUILD_Sha256Finalize(&au8Checksum, &vSha256Context);
	if(memcmp(au8Checksum, pHeader->au8Checksum, 32) != 0){
		MCFBUILD_SetLastError(ERROR_INVALID_DATA);
		return false;
	}
	// Collect serialized strings backwards, performing boundary checks on the way.
	size_t uMinimumSizeToReserve = 0;
	const unsigned char *pbyRead = pbyEnd;
	while(pbyRead != pHeader->abyPayload){
		if((size_t)(pbyRead - pHeader->abyPayload) < sizeof(SerializedElement)){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		pbyRead -= sizeof(SerializedElement);
		SerializedElement *pSerialized = (void *)pbyRead;
		uint64_t u64WholeAndPadding = MCFBUILD_load_be_uint64(&(pSerialized->u64WholeAndPadding));
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64WholeAndPadding`.
		if(u64WholeAndPadding > SIZE_MAX){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		size_t uSizePadded = u64WholeAndPadding % 8;
		size_t uSizeOfString = (size_t)u64WholeAndPadding - uSizePadded * 2;
		if((size_t)(pbyRead - pHeader->abyPayload) < uSizeOfString){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		pbyRead -= uSizeOfString;
		// Add up the number of bytes that a StackElement is going to take.
		size_t uSizeToAdd = sizeof(StackElement) + uSizeOfString + sizeof(wchar_t);
		size_t uSizeToPad = -uSizeToAdd % 8;
		uSizeToAdd += uSizeToPad;
		if(__builtin_add_overflow(uMinimumSizeToReserve, uSizeToAdd, &uMinimumSizeToReserve)){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		// Discard padded characters.
		if((size_t)(pbyRead - pHeader->abyPayload) < uSizePadded){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		pbyRead -= uSizePadded;
	}
	// Allocate the buffer. Reuse the existent buffer if possible.
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uCapacity = pStack->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity = uMinimumSizeToReserve;
		pbyStorage = MCFBUILD_HeapRealloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			return false;
		}
		pStack->pbyStorage = pbyStorage;
		pStack->uCapacity = uCapacity;
	}
	// Rebuild everything from scratch. Boundary checks are unnecessary this time.
	size_t uOffsetTop = 0;
	size_t uOffsetEnd = 0;
	pbyRead = pbyEnd;
	while(pbyRead != pHeader->abyPayload){
		pbyRead -= sizeof(SerializedElement);
		SerializedElement *pSerialized = (void *)pbyRead;
		uint64_t u64WholeAndPadding = MCFBUILD_load_be_uint64(&(pSerialized->u64WholeAndPadding));
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64WholeAndPadding`.
		size_t uSizePadded = u64WholeAndPadding % 8;
		size_t uSizeOfString = (size_t)u64WholeAndPadding - uSizePadded * 2;
		// Create a new element in the stack.
		size_t uSizeToAdd = sizeof(StackElement) + uSizeOfString + sizeof(wchar_t);
		size_t uSizeToPad = -uSizeToAdd % 8;
		uSizeToAdd += uSizeToPad;
		StackElement *pElement = (void *)(pbyStorage + uOffsetEnd);
		pElement->uSizePadded = uSizeToPad % 8;
		pElement->uOffsetPrevInWords = (uOffsetTop / 8) & (SIZE_MAX >> 3);
		// Load each wide character in reverse order.
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			pbyRead -= sizeof(wchar_t);
			pElement->awszString[uIndex] = MCFBUILD_load_be_uint16((const wchar_t *)pbyRead);
		}
		pElement->awszString[uSizeOfString / sizeof(wchar_t)] = 0;
		uOffsetTop = uOffsetEnd;
		uOffsetEnd = uOffsetEnd + uSizeToAdd;
		// Discard padded characters.
		pbyRead -= uSizePadded;
	}
	pStack->uOffsetTop = uOffsetTop;
	pStack->uOffsetEnd = uOffsetEnd;
	return true;
}
