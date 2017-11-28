// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "naive_string.h"
#include "heap.h"
#include "last_error.h"

void MCFBUILD_NaiveStringInitialize(MCFBUILD_NaiveString *pString){
	pString->pbyStorage = 0;
	pString->uCapacity = 0;
	pString->uSize = 0;
}
void MCFBUILD_NaiveStringUninitialize(MCFBUILD_NaiveString *pString){
	MCFBUILD_HeapFree(pString->pbyStorage);
}

const wchar_t *MCFBUILD_NaiveStringGetNullTerminated(const MCFBUILD_NaiveString *pString){
	// If the string is empty, return a pointer to something allocated statically.
	if(pString->uSize == 0){
		return L"";
	}
	unsigned char *pbyStorage = pString->pbyStorage;
	// If there is no null terminator, append one.
	wchar_t *pwcTerminator = (void *)(pbyStorage + pString->uSize);
	if(*pwcTerminator != 0){
		*pwcTerminator = 0;
	}
	return (wchar_t *)pbyStorage;
}
wchar_t *MCFBUILD_NaiveStringGetData(MCFBUILD_NaiveString *pString){
	unsigned char *pbyStorage = pString->pbyStorage;
	return (wchar_t *)pbyStorage;
}
size_t MCFBUILD_NaiveStringGetLength(MCFBUILD_NaiveString *pString){
	return pString->uSize / sizeof(wchar_t);
}

void MCFBUILD_NaiveStringClear(MCFBUILD_NaiveString *pString){
	pString->uSize = 0;
}
bool MCFBUILD_NaiveStringReserve(wchar_t **restrict ppwcCaret, MCFBUILD_NaiveString *pString, size_t uInsertAt, size_t uLengthToInsert){
	// Make sure `uInsertAt` is within the string.
	if(uInsertAt > pString->uSize / sizeof(wchar_t)){
		MCFBUILD_SetLastError(ERROR_INVALID_INDEX);
		return false;
	}
	// Allocate buffer for the area to be reserved.
	size_t uMinimumSizeToReserve;
	if(__builtin_mul_overflow(uLengthToInsert, sizeof(wchar_t), &uMinimumSizeToReserve)){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	if(__builtin_add_overflow(uMinimumSizeToReserve, pString->uSize, &uMinimumSizeToReserve)){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	unsigned char *pbyStorage = pString->pbyStorage;
	size_t uCapacity = pString->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += uCapacity / 2;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		// Reserve space for the null terminator.
		size_t uRealSizeToAlloc;
		if(__builtin_add_overflow(uCapacity, sizeof(wchar_t), &uRealSizeToAlloc)){
			MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pbyStorage = MCFBUILD_HeapRealloc(pbyStorage, uRealSizeToAlloc);
		if(!pbyStorage){
			return false;
		}
		pString->pbyStorage = pbyStorage;
		pString->uCapacity = uCapacity;
	}
	// Relocate characters after `uInsertAt`.
	if(uLengthToInsert != 0){
		memmove(pbyStorage + (uInsertAt + uLengthToInsert) * sizeof(wchar_t), pbyStorage + uInsertAt * sizeof(wchar_t), pString->uSize - uInsertAt * sizeof(wchar_t));
		pString->uSize += uLengthToInsert * sizeof(wchar_t);
	}
	*ppwcCaret = (wchar_t *)(pbyStorage + uInsertAt * sizeof(wchar_t));
	return true;
}
bool MCFBUILD_NaiveStringRemove(MCFBUILD_NaiveString *pString, size_t uRemoveFrom, size_t uLengthToRemove){
	// Make sure `uRemoveFrom` is within the string.
	if(uRemoveFrom > pString->uSize / sizeof(wchar_t)){
		MCFBUILD_SetLastError(ERROR_INVALID_INDEX);
		return false;
	}
	// Do the same with `uRemoveFrom + uLengthToRemove`. Be noted that the addition may wrap.
	if(uLengthToRemove > pString->uSize / sizeof(wchar_t) - uRemoveFrom){
		MCFBUILD_SetLastError(ERROR_INVALID_INDEX);
		return false;
	}
	unsigned char *pbyStorage = pString->pbyStorage;
	// Relocate characters after `uRemoveFrom + uLengthToRemove`.
	if(uLengthToRemove != 0){
		memmove(pbyStorage + uRemoveFrom * sizeof(wchar_t), pbyStorage + (uRemoveFrom + uLengthToRemove) * sizeof(wchar_t), pString->uSize - (uRemoveFrom + uLengthToRemove) * sizeof(wchar_t));
		pString->uSize -= uLengthToRemove * sizeof(wchar_t);
	}
	return true;
}
