// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_stack.h"
#include "last_error.h"
#include "endian.h"
#include "sha256.h"

typedef struct tagElement {
	size_t uSizeWithPadding;
	unsigned char bySizePadded;
	alignas(uintptr_t) wchar_t awszString[];
} Element;

const MCFBUILD_StringStack *MCFBUILD_StringStackGetEmpty(void){
	static const MCFBUILD_StringStack s_vStack;
	return &s_vStack;
}

void MCFBUILD_StringStackConstruct(MCFBUILD_StringStack *pStack){
	pStack->pbyStorage = 0;
	pStack->uCapacity = 0;
	pStack->uOffsetBegin = 0;
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
	pStack->uOffsetBegin = pSource->uOffsetBegin;
#ifndef NDEBUG
	memset(pSource, 0xEB, sizeof(*pSource));
#endif
}

bool MCFBUILD_StringStackIsEmpty(const MCFBUILD_StringStack *pStack){
	return pStack->uOffsetBegin == pStack->uCapacity;
}
void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack){
	pStack->uOffsetBegin = pStack->uCapacity;
}
bool MCFBUILD_StringStackGetTop(const wchar_t **restrict ppwszString, size_t *restrict puLength, const MCFBUILD_StringStack *restrict pStack){
	/*-----------------------------------------------------------*\
	|                        /----------- Element                 |
	|                        |/---------- Beginning of string     |
	|                        ||      /--- Null terminator         |
	|                        ||      |/-- Beginning of padding    |
	|                        ||      ||                           |
	| BEFORE  ???????????????*=======_??*==_???*======_           |
	|         ^storage       ^begin                    ^capacity  |
	| AFTER   ???????????????*=======_??*==_???*======_           |
	|         ^storage       ^string                              |
	|                        \______/length                       |
	\*-----------------------------------------------------------*/
	if(pStack->uOffsetBegin == pStack->uCapacity){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	const unsigned char *pbyStorage = pStack->pbyStorage;
	const Element *pElement = (const void *)(pbyStorage + pStack->uOffsetBegin);
	size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
	*ppwszString = pElement->awszString;
	*puLength = uSizeOfString / sizeof(wchar_t);
	return true;
}
bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwcString, size_t uLength){
	/*-----------------------------------------------------------*\
	|                        /----------- Element                 |
	|                        |/---------- Beginning of string     |
	|                        ||      /--- Null terminator         |
	|                        ||      |/-- Beginning of padding    |
	|                        ||      ||                           |
	| BEFORE  ???????????????*=======_??*==_???*======_           |
	|         ^storage       ^begin                    ^capacity  |
	| AFTER   ??????????*==_?*=======_??*==_???*======_           |
	|         ^storage  ^begin                                    |
	\*-----------------------------------------------------------*/
	size_t uSizeOfString = uLength * sizeof(wchar_t);
	size_t uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
	unsigned char bySizePadded = -uSizeWithPadding % 8;
	uSizeWithPadding += bySizePadded;
	size_t uSizeOfAllDataOld = pStack->uCapacity - pStack->uOffsetBegin;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(uSizeOfAllDataOld, sizeof(Element) + uSizeWithPadding, &uMinimumSizeToReserve)){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uCapacity = pStack->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += uCapacity / 2;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		unsigned char *pbyStorageNew = malloc(uCapacity);
		if(!pbyStorageNew){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		if(uSizeOfAllDataOld != 0){
			memcpy(pbyStorageNew + uCapacity - uSizeOfAllDataOld, pbyStorage + pStack->uOffsetBegin, uSizeOfAllDataOld);
		}
		free(pbyStorage);
		pbyStorage = pbyStorageNew;
		pStack->pbyStorage = pbyStorage;
		pStack->uCapacity = uCapacity;
		pStack->uOffsetBegin = uCapacity - uSizeOfAllDataOld;
	}
	pStack->uOffsetBegin -= sizeof(Element) + uSizeWithPadding;
	Element *pElement = (void *)(pbyStorage + pStack->uOffsetBegin);
	pElement->uSizeWithPadding = uSizeWithPadding;
	pElement->bySizePadded = bySizePadded;
	memcpy(pElement->awszString, pwcString, uSizeOfString);
	pElement->awszString[uSizeOfString / sizeof(wchar_t)] = 0;
	return true;
}
bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwszString){
	return MCFBUILD_StringStackPush(pStack, pwszString, wcslen(pwszString));
}
bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack){
	/*-----------------------------------------------------------*\
	|                        /----------- Element                 |
	|                        |/---------- Beginning of string     |
	|                        ||      /--- Null terminator         |
	|                        ||      |/-- Beginning of padding    |
	|                        ||      ||                           |
	| BEFORE  ???????????????*=======_??*==_???*======_           |
	|         ^storage       ^begin                    ^capacity  |
	| AFTER   ??????????????????????????*==_???*======_           |
	|         ^storage                  ^begin                    |
	\*-----------------------------------------------------------*/
	if(pStack->uOffsetBegin == pStack->uCapacity){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	const unsigned char *pbyStorage = pStack->pbyStorage;
	const Element *pElement = (const void *)(pbyStorage + pStack->uOffsetBegin);
	pStack->uOffsetBegin += sizeof(Element) + pElement->uSizeWithPadding;
	return true;
}

void MCFBUILD_StringStackEnumerateBegin(MCFBUILD_StringStackEnumerationCookie *restrict pCookie, const MCFBUILD_StringStack *restrict pStack){
	pCookie->pStack = pStack;
	pCookie->uOffsetNext = pStack->uOffsetBegin;
}
bool MCFBUILD_StringStackEnumerateNext(const wchar_t **restrict ppwszString, size_t *restrict puLength, MCFBUILD_StringStackEnumerationCookie *restrict pCookie){
	/*-----------------------------------------------------------*\
	|                        /----------- Element                 |
	|                        |/---------- Beginning of string     |
	|                        ||      /--- Null terminator         |
	|                        ||      |/-- Beginning of padding    |
	|                        ||      ||                           |
	| BEFORE  ???????????????*=======_??*==_???*======_           |
	|         ^storage       ^begin                    ^capacity  |
	| AFTER   ???????????????*=======_??*==_???*======_           |
	|         ^storage       ^string                              |
	|                        \______/length                       |
	\*-----------------------------------------------------------*/
	const MCFBUILD_StringStack *restrict pStack = pCookie->pStack;
	size_t uOffsetCursor = pCookie->uOffsetNext;
	if(uOffsetCursor == pStack->uCapacity){
		MCFBUILD_SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	const unsigned char *pbyStorage = pStack->pbyStorage;
	const Element *pElement = (const void *)(pbyStorage + uOffsetCursor);
	size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
	*ppwszString = pElement->awszString;
	*puLength = uSizeOfString / sizeof(wchar_t);
	uOffsetCursor += sizeof(Element) + pElement->uSizeWithPadding;
	pCookie->uOffsetNext = uOffsetCursor;
	return true;
}

typedef struct tagSerializedElement {
	uint64_t u64SizeWholeSerialized;
	alignas(uintptr_t) wchar_t awcString[];
} SerializedElement;

typedef struct tagSerializedHeader {
	uint8_t au8Checksum[32];
	alignas(uintptr_t) unsigned char abyPayload[];
} SerializedHeader;

// This is the salt used to create checksums. The null terminator is part of the salt.
static const char kMagic[] = "MCFBUILD_StringStack:2017-12-12";

bool MCFBUILD_StringStackSerialize(void **restrict ppData, size_t *restrict puSize, const MCFBUILD_StringStack *restrict pStack){
	// Estimate the upper bound of number of bytes to allocate.
	size_t uSize = sizeof(SerializedHeader);
	// Iterate from top to bottom, accumulating number of bytes for each element on the way.
	const unsigned char *pbyStorage = pStack->pbyStorage;
	size_t uOffsetCursor = pStack->uOffsetBegin;
	while(uOffsetCursor != pStack->uCapacity){
		const Element *pElement = (const void *)(pbyStorage + uOffsetCursor);
		size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
		// There is no need to serialize null terminators.
		size_t uSizeWithPadding = uSizeOfString;
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		uSize += sizeof(SerializedElement) + uSizeWithPadding;
		// Scan the next element.
		uOffsetCursor += sizeof(Element) + pElement->uSizeWithPadding;
	}
	// Allocate the buffer now.
	SerializedHeader *pHeader = malloc(uSize);
	if(!pHeader){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	// Populate the buffer with something predictable to ensure padding bytes have fixed values.
	memset(pHeader, -1, uSize);
	// Copy strings from top to bottom, writing backwards.
	unsigned char *pbyWrite = pHeader->abyPayload + uSize - sizeof(SerializedHeader);
	uOffsetCursor = pStack->uOffsetBegin;
	while(uOffsetCursor != pStack->uCapacity){
		const Element *pElement = (const void *)(pbyStorage + uOffsetCursor);
		size_t uSizeOfString = pElement->uSizeWithPadding - pElement->bySizePadded - sizeof(wchar_t);
		// There is no need to serialize null terminators.
		size_t uSizeWithPadding = uSizeOfString;
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		pbyWrite -= sizeof(SerializedElement) + uSizeWithPadding;
		// This is tricky. Acknowledging that `uSizeWithPadding` will be aligned onto an 8-byte boundary,
		// we add `bySizePadded` to it, preserving `bySizePadded` in its three LSBs intactly, with the rest being
		// `uSizeWithPadding` which can be fetched by bitwise and'ing the three LSBs away.
		SerializedElement *pSerialized = (void *)pbyWrite;
		MCFBUILD_store_be_uint64(&(pSerialized->u64SizeWholeSerialized), uSizeWithPadding + bySizePadded);
		// Store the string without the null terminator.
		const wchar_t *pwcReadBase = pElement->awszString;
		wchar_t *pwcWriteBase = pSerialized->awcString;
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			MCFBUILD_move_be_uint16(pwcWriteBase++, pwcReadBase++);
		}
		// Scan the next element.
		uOffsetCursor += sizeof(Element) + pElement->uSizeWithPadding;
	}
	// Calculate the checksum.
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	MCFBUILD_Sha256Update(&vSha256Context, pHeader->abyPayload, uSize - sizeof(SerializedHeader));
	MCFBUILD_Sha256 vSha256;
	MCFBUILD_Sha256Finalize(&vSha256, &vSha256Context);
	memcpy(pHeader->au8Checksum, &vSha256, 32);
	// Hand over the buffer to our caller.
	*ppData = pHeader;
	*puSize = uSize;
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
	// Verify the checksum.
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	MCFBUILD_Sha256Update(&vSha256Context, pHeader->abyPayload, uSize - sizeof(SerializedHeader));
	MCFBUILD_Sha256 vSha256;
	MCFBUILD_Sha256Finalize(&vSha256, &vSha256Context);
	if(memcmp(pHeader->au8Checksum, &vSha256, 32) != 0){
		MCFBUILD_SetLastError(ERROR_INVALID_DATA);
		return false;
	}
	// Collect serialized strings, performing boundary checks on the way.
	size_t uMinimumSizeToReserve = 0;
	const unsigned char *pbyRead = pHeader->abyPayload;
	const unsigned char *pbyEnd = pbyRead + uSize - sizeof(SerializedHeader);
	while(pbyRead != pbyEnd){
		if((size_t)(pbyEnd - pbyRead) < sizeof(SerializedElement)){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		const SerializedElement *pSerialized = (const void *)pbyRead;
		uint64_t u64SizeWholeSerialized = MCFBUILD_load_be_uint64(&(pSerialized->u64SizeWholeSerialized));
		pbyRead += sizeof(SerializedElement);
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64SizeWholeSerialized`.
		if(u64SizeWholeSerialized > PTRDIFF_MAX){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		size_t uSizeWholeSerialized = (size_t)u64SizeWholeSerialized;
		if((size_t)(pbyEnd - pbyRead) < uSizeWholeSerialized / 8 * 8){
			MCFBUILD_SetLastError(ERROR_INVALID_DATA);
			return false;
		}
		size_t uSizeOfString = uSizeWholeSerialized - uSizeWholeSerialized % 8 * 2;
		// Add up the number of bytes that a Element is going to take.
		size_t uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		if(__builtin_add_overflow(uMinimumSizeToReserve, sizeof(Element) + uSizeWithPadding, &uMinimumSizeToReserve)){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pbyRead += uSizeWholeSerialized / 8 * 8;
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
	pStack->uOffsetBegin = uCapacity;
	pbyRead = pHeader->abyPayload;
	while(pbyRead != pbyEnd){
		const SerializedElement *pSerialized = (const void *)pbyRead;
		size_t uSizeWholeSerialized = (size_t)MCFBUILD_load_be_uint64(&(pSerialized->u64SizeWholeSerialized));
		pbyRead += sizeof(SerializedElement);
		// See comments in `MCFBUILD_StringStackSerialize()` for description of `SerializedElement::u64SizeWholeSerialized`.
		// Create a new element in the stack.
		size_t uSizeOfString = uSizeWholeSerialized - uSizeWholeSerialized % 8 * 2;
		size_t uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
		unsigned char bySizePadded = -uSizeWithPadding % 8;
		uSizeWithPadding += bySizePadded;
		pStack->uOffsetBegin -= sizeof(Element) + uSizeWithPadding;
		Element *pElement = (void *)(pbyStorage + pStack->uOffsetBegin);
		pElement->uSizeWithPadding = uSizeWithPadding;
		pElement->bySizePadded = bySizePadded;
		// Load the string. There is no null terminator so we have to append one.
		const wchar_t *pwcReadBase = pSerialized->awcString;
		wchar_t *pwcWriteBase = pElement->awszString;
		for(size_t uIndex = 0; uIndex < uSizeOfString / sizeof(wchar_t); ++uIndex){
			MCFBUILD_move_be_uint16(pwcWriteBase++, pwcReadBase++);
		}
		*pwcWriteBase = 0;
		pbyRead += uSizeWholeSerialized / 8 * 8;
	}
	return true;
}
