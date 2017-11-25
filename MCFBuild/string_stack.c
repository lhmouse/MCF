// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_stack.h"
#include "heap.h"
#include "last_error.h"
#include "endian.h"
#include "sha256.h"

// In reality, members in this structure are stored in reverse order.
typedef struct tagStackElement {
	size_t uSizePadded : 3;
	size_t uSizeWholeInQwords : sizeof(size_t) * 8 - 3;
	// wchar_t awszString[];
} StackElement;

void MCFBUILD_StringStackInitialize(MCFBUILD_StringStack *pStack){
	pStack->pbyStorage = 0;
	pStack->uCapacity = 0;
	pStack->uOffsetEnd = 0;
}
void MCFBUILD_StringStackUninitialize(MCFBUILD_StringStack *pStack){
	MCFBUILD_HeapFree(pStack->pbyStorage);
}

void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack){
	pStack->uOffsetEnd = 0;
}
bool MCFBUILD_StringStackGetTop(const wchar_t **restrict ppwszString, size_t *restrict puLength, const MCFBUILD_StringStack *restrict pStack){
	size_t uOffsetEnd = pStack->uOffsetEnd;
	if(uOffsetEnd == 0){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- StackElement                         |
	|         | |      ||                                         |
	|         ??=======_*??==_*====_*??=======_*?=========_*      |
	| BEFORE  ^storage                                      ^end  |
	| AFTER   ^storage                           ^string          |
	|                                            \_______/length  |
	\*-----------------------------------------------------------*/
	const unsigned char *pbyStorage = pStack->pbyStorage;
	const StackElement *pElement = (const void *)(pbyStorage + uOffsetEnd - sizeof(StackElement));
	size_t uSizeOfString = (size_t)(pElement->uSizeWholeInQwords) * 8 - pElement->uSizePadded - sizeof(wchar_t);
	*ppwszString = (const void *)((const unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
	*puLength = uSizeOfString / sizeof(wchar_t);
	return true;
}
bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwcString, size_t uLength){
	size_t uOffsetEnd = pStack->uOffsetEnd;
	size_t uSizeOfString = uLength * sizeof(wchar_t);
	size_t uSizeWhole = uSizeOfString + sizeof(wchar_t);
	size_t uSizePadded = -uSizeWhole % 8;
	uSizeWhole += uSizePadded;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(uOffsetEnd, uSizeWhole + sizeof(StackElement), &uMinimumSizeToReserve)){
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
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- StackElement                         |
	|         | |      ||                                         |
	|         ??=======_*??==_*====_*??=======_*?=========_*      |
	| BEFORE  ^storage                          ^end              |
	| AFTER   ^storage                                      ^end  |
	\*-----------------------------------------------------------*/
	StackElement *pElement = (void *)(pbyStorage + uOffsetEnd + uSizeWhole);
	memcpy((unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString, pwcString, uSizeOfString);
	*(wchar_t *)((unsigned char *)pElement - sizeof(wchar_t)) = 0;
	pElement->uSizePadded = uSizePadded % 8;
	pElement->uSizeWholeInQwords = (uSizeWhole / 8) & (SIZE_MAX >> 3);
	pStack->uOffsetEnd = uOffsetEnd + uSizeWhole + sizeof(StackElement);
	return true;
}
bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwszString){
	return MCFBUILD_StringStackPush(pStack, pwszString, wcslen(pwszString));
}
bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack){
	size_t uOffsetEnd = pStack->uOffsetEnd;
	if(uOffsetEnd == 0){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         /------------- Beginning of padding                 |
	|         | /----------- Beginning of string body             |
	|         | |      /---- Null terminator                      |
	|         | |      |/--- StackElement                         |
	|         | |      ||                                         |
	|         ??=======_*??==_*====_*??=======_*?=========_*      |
	| BEFORE  ^storage                                      ^end  |
	| AFTER   ^storage                          ^end              |
	\*-----------------------------------------------------------*/
	unsigned char *pbyStorage = pStack->pbyStorage;
	const StackElement *pElement = (const void *)(pbyStorage + uOffsetEnd - sizeof(StackElement));
	pStack->uOffsetEnd = uOffsetEnd - sizeof(StackElement) - (size_t)(pElement->uSizeWholeInQwords) * 8;
	return true;
}

// In reality, members in this structure are stored in reverse order.
typedef struct tagSerializedElement {
	uint64_t u64SizeWholeAndPadding;
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
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uOffsetEnd = pStack->uOffsetEnd;
	while(uOffsetEnd != 0){
		const StackElement *pElement = (const void *)(pbyStorage + uOffsetEnd - sizeof(StackElement));
		size_t uSizeOfString = (size_t)(pElement->uSizeWholeInQwords) * 8 - pElement->uSizePadded - sizeof(wchar_t);
		// There is no need to serialize null terminators.
		size_t uSizeWhole = uSizeOfString;
		size_t uSizePadded = -uSizeWhole % 8;
		uSizeWhole += uSizePadded;
		uSizeToAlloc += uSizeWhole;
		uSizeToAlloc += sizeof(SerializedElement);
		// Scan for the next element.
		uOffsetEnd = uOffsetEnd - sizeof(StackElement) - (size_t)(pElement->uSizeWholeInQwords) * 8;
	}
	// Allocate the buffer now.
	SerializedHeader *pHeader = MCFBUILD_HeapAlloc(uSizeToAlloc);
	if(!pHeader){
		return false;
	}
	// Copy strings from top to bottom.
	unsigned char *pbyWrite = pHeader->abyPayload;
	uOffsetEnd = pStack->uOffsetEnd;
	while(uOffsetEnd != 0){
		const StackElement *pElement = (const void *)(pbyStorage + uOffsetEnd - sizeof(StackElement));
		size_t uSizeOfString = (size_t)(pElement->uSizeWholeInQwords) * 8 - pElement->uSizePadded - sizeof(wchar_t);
		// There is no need to serialize null terminators.
		size_t uSizeWhole = uSizeOfString;
		size_t uSizePadded = -uSizeWhole % 8;
		uSizeWhole += uSizePadded;
		// Fill padding characters with something predictable. Otherwise it will not be possible to get a meaningful checksum.
		for(size_t uOffset = 0; uOffset < uSizePadded; uOffset += sizeof(wchar_t)){
			MCFBUILD_store_be_uint16((void *)(pbyWrite + uOffset), 0xFFFF);
		}
		// Store this string without the null terminator.
		const wchar_t *pwcReadBase = (const void *)((const unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
		wchar_t *pwcWriteBase = (void *)(pbyWrite + uSizePadded);
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			MCFBUILD_store_be_uint16(pwcWriteBase + uIndex, *(pwcReadBase + uIndex));
		}
		pbyWrite += uSizeWhole;
		// This is tricky. Acknowledging that `uSizeOfString + uSizePadded` will be aligned onto an 8-byte boundary, we add
		// another `uSizePadded` to the result, which will have `uSizePadded` in its three LSBs intactly, with the rest being
		// `uSizeOfString + uSizePadded` which can be fetched by bitwise and'ing the three LSBs away.
		SerializedElement *pSerialized = (void *)pbyWrite;
		MCFBUILD_store_be_uint64(&(pSerialized->u64SizeWholeAndPadding), uSizeOfString + uSizePadded * 2);
		pbyWrite += sizeof(SerializedElement);
		// Scan for the next element.
		uOffsetEnd = uOffsetEnd - sizeof(StackElement) - (size_t)(pElement->uSizeWholeInQwords) * 8;
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
		const SerializedElement *pSerialized = (const void *)pbyRead;
		uint64_t u64SizeWholeAndPadding = MCFBUILD_load_be_uint64(&(pSerialized->u64SizeWholeAndPadding));
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64SizeWholeAndPadding`.
		if(u64SizeWholeAndPadding > SIZE_MAX){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		if((size_t)(pbyRead - pHeader->abyPayload) < (size_t)u64SizeWholeAndPadding / 8 * 8){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		pbyRead -= (size_t)u64SizeWholeAndPadding / 8 * 8;
		size_t uSizeOfString = (size_t)(u64SizeWholeAndPadding - u64SizeWholeAndPadding % 8 * 2);
		// Add up the number of bytes that a StackElement is going to take.
		size_t uSizeWhole = uSizeOfString + sizeof(wchar_t);
		size_t uSizePadded = -uSizeWhole % 8;
		uSizeWhole += uSizePadded;
		if(__builtin_add_overflow(uMinimumSizeToReserve, uSizeWhole + sizeof(StackElement), &uMinimumSizeToReserve)){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
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
	size_t uOffsetEnd = 0;
	pbyRead = pbyEnd;
	while(pbyRead != pHeader->abyPayload){
		pbyRead -= sizeof(SerializedElement);
		const SerializedElement *pSerialized = (const void *)pbyRead;
		uint64_t u64SizeWholeAndPadding = MCFBUILD_load_be_uint64(&(pSerialized->u64SizeWholeAndPadding));
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64SizeWholeAndPadding`.
		pbyRead -= (size_t)u64SizeWholeAndPadding / 8 * 8;
		size_t uSizeOfString = (size_t)(u64SizeWholeAndPadding - u64SizeWholeAndPadding % 8 * 2);
		// Create a new element in the stack.
		size_t uSizeWhole = uSizeOfString + sizeof(wchar_t);
		size_t uSizePadded = -uSizeWhole % 8;
		uSizeWhole += uSizePadded;
		StackElement *pElement = (void *)(pbyStorage + uOffsetEnd + uSizeWhole);
		// Load this string. There is no null terminator so we have to append one.
		const wchar_t *pwcReadBase = (const void *)(pbyRead + (size_t)(u64SizeWholeAndPadding % 8));
		wchar_t *pwcWriteBase = (void *)((unsigned char *)pElement - sizeof(wchar_t) - uSizeOfString);
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			MCFBUILD_store_be_uint16(pwcWriteBase + uIndex, *(pwcReadBase + uIndex));
		}
		*(wchar_t *)((unsigned char *)pElement - sizeof(wchar_t)) = 0;
		pElement->uSizePadded = uSizePadded % 8;
		pElement->uSizeWholeInQwords = (uSizeWhole / 8) & (SIZE_MAX >> 3);
		uOffsetEnd = uOffsetEnd + uSizeWhole + sizeof(StackElement);
	}
	pStack->uOffsetEnd = uOffsetEnd;
	return true;
}
