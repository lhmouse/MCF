// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_stack.h"
#include "last_error.h"
#include "endian.h"
#include "sha256.h"

// In reality, members in this structure are stored in reverse order.
typedef struct tagElement {
	size_t uSizeWithPadding;
	unsigned char bySizePadded;
	alignas(uintptr_t) wchar_t awszString[];
} Element;

void MCFBUILD_StringStackConstruct(MCFBUILD_StringStack *pStack){
	pStack->pbyStorage = 0;
	pStack->uCapacity = 0;
	pStack->uOffsetEnd = 0;
}
void MCFBUILD_StringStackDestruct(MCFBUILD_StringStack *pStack){
	free(pStack->pbyStorage);
#ifndef NDEBUG
	memset(pStack, 0xEC, sizeof(*pStack));
#endif
}
void MCFBUILD_StringStackMove(MCFBUILD_StringStack *restrict pStack, MCFBUILD_StringStack *restrict pSource){
	pStack->pbyStorage = pSource->pbyStorage;
	pStack->uCapacity = pSource->uCapacity;
	pStack->uOffsetEnd = pSource->uOffsetEnd;
#ifndef NDEBUG
	memset(pSource, 0xEB, sizeof(*pSource));
#endif
}

void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack){
	pStack->uOffsetEnd = 0;
}
bool MCFBUILD_StringStackGetTop(const wchar_t **restrict ppwszString, size_t *restrict puLength, const MCFBUILD_StringStack *restrict pStack){
	/*-----------------------------------------------------------*\
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- Element                              |
	|         | |      ||                                         |
	| BEFORE  ??=======_*??==_*====_*??=======_*?=========_*      |
	|         ^storage                                      ^end  |
	| AFTER   ??=======_*??==_*====_*??=======_*?=========_*      |
	|         ^storage                           ^string          |
	|                                            \_______/length  |
	\*-----------------------------------------------------------*/
	if(pStack->uOffsetEnd == 0){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	const unsigned char *pbyStorage = pStack->pbyStorage;
	const Element *pElement = (const void *)(pbyStorage + pStack->uOffsetEnd - sizeof(Element));
	size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
	*ppwszString = (const void *)((const unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
	*puLength = uSizeOfString / sizeof(wchar_t);
	return true;
}
bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwcString, size_t uLength){
	/*-----------------------------------------------------------*\
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- Element                              |
	|         | |      ||                                         |
	| BEFORE  ??=======_*??==_*====_*??=======_*                  |
	|         ^storage                          ^end              |
	| AFTER   ??=======_*??==_*====_*??=======_*?=========_*      |
	|         ^storage                                      ^end  |
	\*-----------------------------------------------------------*/
	size_t uSizeOfString = uLength * sizeof(wchar_t);
	size_t uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
	unsigned char bySizePadded = -uSizeWithPadding % 8;
	uSizeWithPadding += bySizePadded;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(pStack->uOffsetEnd, uSizeWithPadding + sizeof(Element), &uMinimumSizeToReserve)){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uCapacity = pStack->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += uCapacity / 2;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		pbyStorage = realloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pStack->pbyStorage = pbyStorage;
		pStack->uCapacity = uCapacity;
	}
	pStack->uOffsetEnd += uSizeWithPadding + sizeof(Element);
	Element *pElement = (void *)(pbyStorage + pStack->uOffsetEnd - sizeof(Element));
	memcpy((unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString, pwcString, uSizeOfString);
	*(wchar_t *)((unsigned char *)pElement - sizeof(wchar_t)) = 0;
	pElement->uSizeWithPadding = uSizeWithPadding;
	pElement->bySizePadded = bySizePadded;
	return true;
}
bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwszString){
	return MCFBUILD_StringStackPush(pStack, pwszString, wcslen(pwszString));
}
bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack){
	/*-----------------------------------------------------------*\
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- Element                              |
	|         | |      ||                                         |
	| BEFORE  ??=======_*??==_*====_*??=======_*?=========_*      |
	|         ^storage                                      ^end  |
	| AFTER   ??=======_*??==_*====_*??=======_*                  |
	|         ^storage                          ^end              |
	\*-----------------------------------------------------------*/
	if(pStack->uOffsetEnd == 0){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	unsigned char *pbyStorage = pStack->pbyStorage;
	const Element *pElement = (const void *)(pbyStorage + pStack->uOffsetEnd - sizeof(Element));
	pStack->uOffsetEnd -= sizeof(Element) + pElement->uSizeWithPadding;
	return true;
}

void MCFBUILD_StringStackEnumerateBegin(MCFBUILD_StringStackEnumerationCookie *restrict pCookie, const MCFBUILD_StringStack *restrict pStack){
	pCookie->pStack = pStack;
	pCookie->uOffsetNext = pStack->uOffsetEnd;
}
bool MCFBUILD_StringStackEnumerateNext(const wchar_t **restrict ppwszString, size_t *restrict puLength, MCFBUILD_StringStackEnumerationCookie *restrict pCookie){
	/*-----------------------------------------------------------*\
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- Element                              |
	|         | |      ||                                         |
	| BEFORE  ??=======_*??==_*====_*??=======_*?=========_*      |
	|         ^storage                                      ^end  |
	| AFTER   ??=======_*??==_*====_*??=======_*?=========_*      |
	|         ^storage                           ^string          |
	|                                            \_______/length  |
	\*-----------------------------------------------------------*/
	const MCFBUILD_StringStack *restrict pStack = pCookie->pStack;
	const unsigned char *pbyStorage = pStack->pbyStorage;
	const Element *pElement;
	size_t uOffsetCursor = pCookie->uOffsetNext;
	if(uOffsetCursor == 0){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	if(uOffsetCursor > pStack->uOffsetEnd){
		MCFBUILD_SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	pElement = (const void *)(pbyStorage + uOffsetCursor - sizeof(Element));
	size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
	*ppwszString = (const void *)((const unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
	*puLength = uSizeOfString / sizeof(wchar_t);
	uOffsetCursor -= sizeof(Element) + pElement->uSizeWithPadding;
	pCookie->uOffsetNext = uOffsetCursor;
	return true;
}

// In reality, members in this structure are stored in reverse order.
typedef struct tagSerializedElement {
	uint64_t u64SizeWholeSerialized;
	// wchar_t awcReverseString[];
} SerializedElement;

typedef struct tagSerializedHeader {
	uint8_t au8Checksum[32];
	unsigned char abyPayload[];
} SerializedHeader;

// This is the salt used to create checksums. The null terminator is part of the salt.
static const char kMagic[] = "MCFBUILD_StringStack:2017-11-23";

bool MCFBUILD_StringStackSerialize(void **restrict ppData, size_t *restrict puSize, const MCFBUILD_StringStack *restrict pStack){
	// Estimate the upper bound of number of bytes to allocate.
	size_t uSizeToAlloc = sizeof(SerializedHeader);
	// Iterate from top to bottom, accumulating number of bytes for each element on the way.
	const unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uOffsetReadEnd = pStack->uOffsetEnd;
	while(uOffsetReadEnd != 0){
		const Element *pElement = (const void *)(pbyStorage + uOffsetReadEnd - sizeof(Element));
		size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
		// There is no need to serialize null terminators.
		size_t uSizeWithPadding = uSizeOfString;
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		uSizeToAlloc += uSizeWithPadding;
		uSizeToAlloc += sizeof(SerializedElement);
		// Scan the next element.
		uOffsetReadEnd -= sizeof(Element) + pElement->uSizeWithPadding;
	}
	// Allocate the buffer now.
	SerializedHeader *pHeader = malloc(uSizeToAlloc);
	if(!pHeader){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	// Populate the buffer with something predictable to ensure padding bytes have fixed values.
	memset(pHeader, -1, uSizeToAlloc);
	// Copy strings from top to bottom.
	unsigned char *pbyWrite = pHeader->abyPayload;
	uOffsetReadEnd = pStack->uOffsetEnd;
	while(uOffsetReadEnd != 0){
		const Element *pElement = (const void *)(pbyStorage + uOffsetReadEnd - sizeof(Element));
		size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
		// There is no need to serialize null terminators.
		size_t uSizeWithPadding = uSizeOfString;
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		// Store the string without the null terminator.
		const wchar_t *pwcReadBase = (const void *)((const unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
		wchar_t *pwcWriteBase = (void *)(pbyWrite + bySizePadded);
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			MCFBUILD_move_be_uint16(pwcWriteBase + uIndex, pwcReadBase + uIndex);
		}
		pbyWrite += uSizeWithPadding;
		// This is tricky. Acknowledging that `uSizeWithPadding` will be aligned onto an 8-byte boundary,
		// we add `bySizePadded` to it, preserving `bySizePadded` in its three LSBs intactly, with the rest being
		// `uSizeWithPadding` which can be fetched by bitwise and'ing the three LSBs away.
		SerializedElement *pSerialized = (void *)pbyWrite;
		MCFBUILD_store_be_uint64(&(pSerialized->u64SizeWholeSerialized), uSizeWithPadding + bySizePadded);
		pbyWrite += sizeof(SerializedElement);
		// Scan the next element.
		uOffsetReadEnd -= sizeof(Element) + pElement->uSizeWithPadding;
	}
	// Calculate the checksum.
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	MCFBUILD_Sha256Update(&vSha256Context, pHeader->abyPayload, (size_t)(pbyWrite - pHeader->abyPayload));
	MCFBUILD_Sha256 vSha256;
	MCFBUILD_Sha256Finalize(&vSha256, &vSha256Context);
	memcpy(pHeader->au8Checksum, &vSha256, 32);
	// Hand over the buffer to our caller.
	*ppData = pHeader;
	*puSize = uSizeToAlloc;
	return true;
}
void MCFBUILD_StringStackFreeSerializedBuffer(void *pData){
	free(pData);
}
bool MCFBUILD_StringStackDeserialize(MCFBUILD_StringStack *restrict pStack, const void *restrict pData, size_t uSize){
	if(uSize < sizeof(SerializedHeader)){
		MCFBUILD_SetLastError(ERROR_INVALID_DATA);
		return false;
	}
	const SerializedHeader *pHeader = pData;
	const unsigned char *pbyEnd = (const unsigned char *)pData + uSize;
	// Verify the checksum.
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	MCFBUILD_Sha256Update(&vSha256Context, pHeader->abyPayload, (size_t)(pbyEnd - pHeader->abyPayload));
	MCFBUILD_Sha256 vSha256;
	MCFBUILD_Sha256Finalize(&vSha256, &vSha256Context);
	if(memcmp(pHeader->au8Checksum, &vSha256, 32) != 0){
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
		const SerializedElement *pSerialized = (const void *)pbyRead;
		uint64_t u64SizeWholeSerialized = MCFBUILD_load_be_uint64(&(pSerialized->u64SizeWholeSerialized));
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64SizeWholeSerialized`.
		if(u64SizeWholeSerialized > PTRDIFF_MAX){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		size_t uSizeWholeSerialized = (size_t)u64SizeWholeSerialized;
		if((size_t)(pbyRead - pHeader->abyPayload) < uSizeWholeSerialized / 8 * 8){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		pbyRead -= uSizeWholeSerialized / 8 * 8;
		size_t uSizeOfString = uSizeWholeSerialized - uSizeWholeSerialized % 8 * 2;
		// Add up the number of bytes that a Element is going to take.
		size_t uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		if(__builtin_add_overflow(uMinimumSizeToReserve, uSizeWithPadding + sizeof(Element), &uMinimumSizeToReserve)){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
	}
	// Allocate the buffer. Reuse the existent buffer if possible.
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uCapacity = pStack->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity = uMinimumSizeToReserve;
		pbyStorage = realloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pStack->pbyStorage = pbyStorage;
		pStack->uCapacity = uCapacity;
	}
	// Rebuild everything from scratch. Boundary checks are unnecessary this time.
	pStack->uOffsetEnd = 0;
	pbyRead = pbyEnd;
	while(pbyRead != pHeader->abyPayload){
		pbyRead -= sizeof(SerializedElement);
		const SerializedElement *pSerialized = (const void *)pbyRead;
		size_t uSizeWholeSerialized = (size_t)MCFBUILD_load_be_uint64(&(pSerialized->u64SizeWholeSerialized));
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64SizeWholeSerialized`.
		pbyRead -= uSizeWholeSerialized / 8 * 8;
		size_t uSizeOfString = uSizeWholeSerialized - uSizeWholeSerialized % 8 * 2;
		// Create a new element in the stack.
		size_t uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		Element *pElement = (void *)(pbyStorage + pStack->uOffsetEnd + uSizeWithPadding);
		// Load the string. There is no null terminator so we have to append one.
		const wchar_t *pwcReadBase = (const void *)(pbyRead + uSizeWholeSerialized % 8);
		wchar_t *pwcWriteBase = (void *)((unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			MCFBUILD_move_be_uint16(pwcWriteBase + uIndex, pwcReadBase + uIndex);
		}
		pwcWriteBase[uSizeOfString / sizeof(wchar_t)] = 0;
		pElement->uSizeWithPadding = uSizeWithPadding;
		pElement->bySizePadded = bySizePadded;
		pStack->uOffsetEnd += uSizeWithPadding + sizeof(Element);
	}
	return true;
}
