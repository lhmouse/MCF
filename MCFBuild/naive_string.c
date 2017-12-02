// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "naive_string.h"
#include "heap.h"
#include "last_error.h"
#include "endian.h"
#include "sha256.h"

void MCFBUILD_NaiveStringConstruct(MCFBUILD_NaiveString *pString){
	pString->pbyStorage = 0;
	pString->uCapacity = 0;
	pString->uSize = 0;
}
void MCFBUILD_NaiveStringDestruct(MCFBUILD_NaiveString *pString){
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
		MCFBUILD_SetLastError(ERROR_INVALID_PARAMETER);
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
		wmemmove((wchar_t *)pbyStorage + uInsertAt + uLengthToInsert, (wchar_t *)pbyStorage + uInsertAt, pString->uSize / sizeof(wchar_t) - uInsertAt);
		pString->uSize += uLengthToInsert * sizeof(wchar_t);
	}
	*ppwcCaret = (wchar_t *)(pbyStorage + uInsertAt * sizeof(wchar_t));
	return true;
}
bool MCFBUILD_NaiveStringRemove(MCFBUILD_NaiveString *pString, size_t uRemoveFrom, size_t uLengthToRemove){
	// Make sure `uRemoveFrom` is within the string.
	if(uRemoveFrom > pString->uSize / sizeof(wchar_t)){
		MCFBUILD_SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	// Do the same with `uRemoveFrom + uLengthToRemove`. Be noted that the addition may wrap.
	if(uLengthToRemove > pString->uSize / sizeof(wchar_t) - uRemoveFrom){
		MCFBUILD_SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	unsigned char *pbyStorage = pString->pbyStorage;
	// Relocate characters after `uRemoveFrom + uLengthToRemove`.
	if(uLengthToRemove != 0){
		wmemmove((wchar_t *)pbyStorage + uRemoveFrom, (wchar_t *)pbyStorage + uRemoveFrom + uLengthToRemove, pString->uSize / sizeof(wchar_t) - uRemoveFrom - uLengthToRemove);
		pString->uSize -= uLengthToRemove * sizeof(wchar_t);
	}
	return true;
}

// This is the salt used to create checksums. The null terminator is part of the salt.
static const char kMagic[] = "MCFBUILD_NaiveString:2017-12-01";

void MCFBUILD_NaiveStringGetSha256(MCFBUILD_Sha256 *pau8Sha256, const MCFBUILD_NaiveString *pString){
	MCFBUILD_Sha256Context vSha256Context;
	MCFBUILD_Sha256Initialize(&vSha256Context);
	MCFBUILD_Sha256Update(&vSha256Context, kMagic, sizeof(kMagic));
	const unsigned char *pbyStorage = pString->pbyStorage;
	// Read characters in big endian order.
	for(size_t uSizeTotal = 0; uSizeTotal < pString->uSize; uSizeTotal += sizeof(wchar_t)){
		wchar_t awchStep[1];
		MCFBUILD_move_be_uint16(awchStep, (const wchar_t *)(pbyStorage + uSizeTotal));
		MCFBUILD_Sha256Update(&vSha256Context, awchStep, sizeof(awchStep));
	}
	MCFBUILD_Sha256Finalize(pau8Sha256, &vSha256Context);
}

bool MCFBUILD_NaiveStringInsert(MCFBUILD_NaiveString *restrict pString, size_t uInsertAt, const wchar_t *restrict pwcStringToInsert, size_t uLengthToInsert){
	wchar_t *pwcCaret;
	if(!MCFBUILD_NaiveStringReserve(&pwcCaret, pString, uInsertAt, uLengthToInsert)){
		return false;
	}
	wmemcpy(pwcCaret, pwcStringToInsert, uLengthToInsert);
	return true;
}
bool MCFBUILD_NaiveStringInsertNullTerminated(MCFBUILD_NaiveString *restrict pString, size_t uInsertAt, const wchar_t *restrict pwszStringToInsert){
	return MCFBUILD_NaiveStringInsert(pString, uInsertAt, pwszStringToInsert, wcslen(pwszStringToInsert));
}
bool MCFBUILD_NaiveStringInsertRepeated(MCFBUILD_NaiveString *restrict pString, size_t uInsertAt, wchar_t wcCharacterToInsert, size_t uLengthToInsert){
	wchar_t *pwcCaret;
	if(!MCFBUILD_NaiveStringReserve(&pwcCaret, pString, uInsertAt, uLengthToInsert)){
		return false;
	}
	wmemset(pwcCaret, wcCharacterToInsert, uLengthToInsert);
	return true;
}
bool MCFBUILD_NaiveStringAppend(MCFBUILD_NaiveString *restrict pString, const wchar_t *restrict pwcStringToInsert, size_t uLengthToInsert){
	size_t uInsertAt = MCFBUILD_NaiveStringGetLength(pString);
	return MCFBUILD_NaiveStringInsert(pString, uInsertAt, pwcStringToInsert, uLengthToInsert);
}
bool MCFBUILD_NaiveStringAppendNullTerminated(MCFBUILD_NaiveString *restrict pString, const wchar_t *restrict pwszStringToInsert){
	return MCFBUILD_NaiveStringAppend(pString, pwszStringToInsert, wcslen(pwszStringToInsert));
}
bool MCFBUILD_NaiveStringPush(MCFBUILD_NaiveString *pString, wchar_t wcCharacterToInsert, size_t uLengthToInsert){
	size_t uInsertAt = MCFBUILD_NaiveStringGetLength(pString);
	return MCFBUILD_NaiveStringInsertRepeated(pString, uInsertAt, wcCharacterToInsert, uLengthToInsert);
}
bool MCFBUILD_NaiveStringPop(MCFBUILD_NaiveString *pString, size_t uLengthToRemove){
	size_t uRemoveFrom;
	if(__builtin_sub_overflow(MCFBUILD_NaiveStringGetLength(pString), uLengthToRemove, &uRemoveFrom)){
		MCFBUILD_SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	return MCFBUILD_NaiveStringRemove(pString, uRemoveFrom, uLengthToRemove);
}
