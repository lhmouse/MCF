// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_stack.h"
#include <windows.h>

typedef struct tagStackElement {
	uint64_t u64OffsetPadded : 3;
	uint64_t u64OffsetPrev : 61;
	wchar_t awcData[];
} StackElement;

void MCFBUILD_StringStackInitialize(MCFBUILD_StringStack *pStack){
	pStack->pbyStorage = 0;
	pStack->uCapacity = 0;
	pStack->uOffsetTop = 0;
	pStack->uOffsetEnd = 0;
}
void MCFBUILD_StringStackUninitialize(MCFBUILD_StringStack *pStack){
	// Passing a null pointer to `HeapFree()` is undefined behavior.
	if(pStack->pbyStorage){
		HeapFree(GetProcessHeap(), 0, pStack->pbyStorage);
	}
}

bool MCFBUILD_StringStackGetTop(const wchar_t **restrict ppwszString, size_t *restrict puLength, const MCFBUILD_StringStack *restrict pStack){
	if(pStack->uOffsetTop == pStack->uOffsetEnd){
		SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|*----------|      |
	|         ^storage                                            |
	| BEFORE                                    ^top        ^end  |
	| AFTER                                      ^string          |
	|                                            \________/length |
	\*-----------------------------------------------------------*/
	const StackElement *pElement = (void *)(pStack->pbyStorage + pStack->uOffsetTop);
	*ppwszString = pElement->awcData;
	*puLength = (pStack->uOffsetEnd - pStack->uOffsetTop - pElement->u64OffsetPadded - sizeof(StackElement)) / sizeof(wchar_t) - 1;
	return true;
}
void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack){
	pStack->uOffsetTop = 0;
	pStack->uOffsetEnd = 0;
}
bool MCFBUILD_StringStackPushN(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwcString, size_t uLength){
	size_t uSizeToAdd = (uLength + 1) * sizeof(wchar_t) + sizeof(StackElement);
	size_t uSizeToPad = -uSizeToAdd % 8;
	uSizeToAdd += uSizeToPad;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(pStack->uOffsetEnd, uSizeToAdd, &uMinimumSizeToReserve)){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
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
		// Is passing a null pointer to `HeapReAlloc()` undefined behavior?
		// MSDN says passing a null pointer to `HeapFree()` is UB, but merely says 'this pointer is returned
		// by an earlier call to the `HeapAlloc` or `HeapReAlloc` function'. We assume it is unsafe here.
		pbyStorage = pbyStorage ? HeapReAlloc(GetProcessHeap(), 0, pbyStorage, uCapacity)
		                        : HeapAlloc(GetProcessHeap(), 0, uCapacity);
		if(!pbyStorage){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
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
	StackElement *pElement = (void *)(pStack->pbyStorage + pStack->uOffsetEnd);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
	pElement->u64OffsetPadded = uSizeToPad;
	pElement->u64OffsetPrev = pStack->uOffsetTop;
#pragma GCC diagnostic pop
	wmemcpy(pElement->awcData, pwcString, uLength)[uLength] = 0;
	size_t uOffsetEndNew = pStack->uOffsetEnd + uSizeToAdd;
	pStack->uOffsetTop = pStack->uOffsetEnd;
	pStack->uOffsetEnd = uOffsetEndNew;
	return true;
}
bool MCFBUILD_StringStackPushZ(MCFBUILD_StringStack *restrict pStack, const wchar_t *restrict pwszString){
	return MCFBUILD_StringStackPushN(pStack, pwszString, wcslen(pwszString));
}
bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack){
	if(pStack->uOffsetTop == pStack->uOffsetEnd){
		SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	/*-----------------------------------------------------------*\
	|         *---------|*----|*----|*---------|*----------|      |
	|         ^storage                                            |
	| BEFORE                                    ^top        ^end  |
	| AFTER                          ^top       ^end              |
	\*-----------------------------------------------------------*/
	const StackElement *pElement = (void *)(pStack->pbyStorage + pStack->uOffsetTop);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
	size_t uOffsetTopNew = pElement->u64OffsetPrev;
#pragma GCC diagnostic pop
	pStack->uOffsetEnd = pStack->uOffsetTop;
	pStack->uOffsetTop = uOffsetTopNew;
	return true;
}
