// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "string_template.h"
#include "string_stack.h"
#include "variable_map.h"
#include "naive_string.h"
#include "last_error.h"

typedef enum tagSegmentType {
	kSegmentEndOfString = 0, // bySizePadded = ignored
	                         // uParam       = ignored
	                         // awcString    = empty
	kSegmentLiteral     = 1, // bySizePadded = size of trailing padding
	                         // uParam       = size of string with padding
	                         // awcString    = string without the null terminator
	kSegmentFromStack   = 2, // bySizePadded = ignored
	                         // uParam       = index from the top of the stack
	                         // awcString    = empty
	kSegmentFromMap     = 3, // bySizePadded = ignored
	                         // uParam       = size of string with padding
	                         // awcString    = key string with the null terminator
} SegmentType;

typedef struct tagSegment {
	unsigned char byType;
	unsigned char bySizePadded;
	size_t uParam;
	alignas(uintptr_t) wchar_t awcString[];
} Segment;

static size_t GetSizeWithPadding(SegmentType eType, size_t uParam){
	size_t uSizeWithPadding;
	switch(eType){
	case kSegmentEndOfString:
		uSizeWithPadding = 0;
		break;
	case kSegmentLiteral:
		uSizeWithPadding = uParam;
		break;
	case kSegmentFromStack:
		uSizeWithPadding = 0;
		break;
	case kSegmentFromMap:
		uSizeWithPadding = uParam;
		break;
	default:
		__builtin_trap();
	}
	return uSizeWithPadding;
}
static bool PushSegment(MCFBUILD_StringTemplate *restrict pTemplate, size_t *restrict puLastCapacityTaken, SegmentType eType, size_t uParam, const wchar_t *pwcString, size_t uSizeOfString){
	// Reserve storage for the new segment.
	size_t uSizeWithPadding;
	switch(eType){
	case kSegmentEndOfString:
		uSizeWithPadding = 0;
		break;
	case kSegmentLiteral:
		uSizeWithPadding = uSizeOfString;
		break;
	case kSegmentFromStack:
		uSizeWithPadding = 0;
		break;
	case kSegmentFromMap:
		uSizeWithPadding = uSizeOfString + sizeof(wchar_t);
		break;
	default:
		__builtin_trap();
	}
	unsigned char bySizePadded = -uSizeWithPadding % 8;
	uSizeWithPadding += bySizePadded;
	size_t uMinimumSizeToReserve;
	if(__builtin_add_overflow(pTemplate->uOffsetEnd, sizeof(Segment) + uSizeWithPadding, &uMinimumSizeToReserve)){
		return false;
	}
	unsigned char *pbyStorage = pTemplate->pbyStorage;
	size_t uCapacity = pTemplate->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += uCapacity / 2;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		pbyStorage = realloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			return false;
		}
		pTemplate->pbyStorage = pbyStorage;
		pTemplate->uCapacity = uCapacity;
	}
	size_t uLastCapacityTaken = *puLastCapacityTaken;
	Segment *pSegment;
	if((eType == kSegmentLiteral) && (uLastCapacityTaken != 0)){
		// Adjacent segments having type `kSegmentLiteral` will be merged.
		pSegment = (void *)(pbyStorage + pTemplate->uOffsetEnd - uLastCapacityTaken);
		if(pSegment->byType == kSegmentLiteral){
			// Merge into it!
			size_t uSizeOfStringOld = pSegment->uParam - pSegment->bySizePadded;
			// Recalculate the size of the last segment.
			uSizeWithPadding = uSizeOfStringOld + uSizeOfString;
			bySizePadded = -uSizeWithPadding % 8;
			uSizeWithPadding += bySizePadded;
			// Extend the last segment to absorb the string.
			pSegment->bySizePadded = bySizePadded;
			pSegment->uParam = uSizeWithPadding;
			memcpy((wchar_t *)((unsigned char *)(pSegment->awcString) + uSizeOfStringOld), pwcString, uSizeOfString);
			pTemplate->uOffsetEnd -= uLastCapacityTaken;
			uLastCapacityTaken = sizeof(Segment) + uSizeWithPadding;
			goto jDone;
		}
	}
	// Create a new segment.
	pSegment = (void *)(pbyStorage + pTemplate->uOffsetEnd);
	pSegment->byType = eType;
	switch(eType){
	case kSegmentEndOfString:
		pSegment->bySizePadded = 0;
		pSegment->uParam = 0;
		uLastCapacityTaken = sizeof(Segment);
		break;
	case kSegmentLiteral:
		pSegment->bySizePadded = bySizePadded;
		pSegment->uParam = uSizeWithPadding;
		memcpy(pSegment->awcString, pwcString, uSizeOfString);
		uLastCapacityTaken = sizeof(Segment) + uSizeWithPadding;
		break;
	case kSegmentFromStack:
		pSegment->bySizePadded = 0;
		pSegment->uParam = uParam;
		uLastCapacityTaken = sizeof(Segment);
		break;
	case kSegmentFromMap:
		pSegment->bySizePadded = bySizePadded;
		pSegment->uParam = uSizeWithPadding;
		memcpy(pSegment->awcString, pwcString, uSizeOfString);
		*(wchar_t *)((unsigned char *)(pSegment->awcString) + uSizeOfString) = 0;
		uLastCapacityTaken = sizeof(Segment) + uSizeWithPadding;
		break;
	default:
		__builtin_trap();
	}
jDone:
	pTemplate->uOffsetEnd += uLastCapacityTaken;
	*puLastCapacityTaken = uLastCapacityTaken;
	return true;
}

/*static MCFBUILD_StringTemplateParseResult ParseDigits(uint32_t *restrict pu32Value, unsigned uRadix, const wchar_t *pwcString, size_t uLength){
	static const wchar_t kHexTable[] = L"00112233445566778899aAbBcCdDeEfF";
	uint32_t u32Value = 0;
	for(size_t uIndex = 0; uIndex < uLength; ++uIndex){
		wchar_t wcDigit = pwcString[uIndex];
		if(wcDigit == 0){
			return MCFBUILD_kStringTemplateParseDigitsTooFew;
		}
		const wchar_t *pwcFound = wmemchr(kHexTable, wcDigit, uRadix * 2);
		if(!pwcFound){
			return MCFBUILD_kStringTemplateParseDigitsInvalid;
		}
		u32Value += (unsigned)(pwcFound - kHexTable) / 2;
		u32Value *= uRadix;
	}
	*pu32Value = u32Value;
	return MCFBUILD_kStringTemplateParseSuccess;
}*/

void MCFBUILD_StringTemplateConstruct(MCFBUILD_StringTemplate *pTemplate){
	pTemplate->pbyStorage = 0;
	pTemplate->uCapacity = 0;
	pTemplate->uOffsetEnd = 0;
}
void MCFBUILD_StringTemplateDestruct(MCFBUILD_StringTemplate *pTemplate){
	free(pTemplate->pbyStorage);
#ifndef NDEBUG
	memset(pTemplate, 0xEC, sizeof(*pTemplate));
#endif
}
void MCFBUILD_StringTemplateMove(MCFBUILD_StringTemplate *restrict pTemplate, MCFBUILD_StringTemplate *restrict pSource){
	pTemplate->pbyStorage = pSource->pbyStorage;
	pTemplate->uCapacity = pSource->uCapacity;
	pTemplate->uOffsetEnd = pSource->uOffsetEnd;
#ifndef NDEBUG
	memset(pSource, 0xEB, sizeof(*pSource));
#endif
}

void MCFBUILD_StringTemplateClear(MCFBUILD_StringTemplate *pTemplate){
	pTemplate->uOffsetEnd = 0;
}
bool MCFBUILD_StringTemplateParse(MCFBUILD_StringTemplate *restrict pTemplate, MCFBUILD_StringTemplateParseResult *restrict peResult, size_t *restrict puResultOffset, const wchar_t *restrict pwszRawString){
	// Calculate the size of the last segment, if any.
	size_t uLastCapacityTaken = 0;
	for(size_t uOffsetCursor = 0; uOffsetCursor != pTemplate->uOffsetEnd; uOffsetCursor += uLastCapacityTaken){
		const Segment *pSegment = (void *)(pTemplate->pbyStorage + uOffsetCursor);
		uLastCapacityTaken = sizeof(Segment) + GetSizeWithPadding(pSegment->byType, pSegment->uParam);
	}
	// Save the offset to data end. If anything goes wrong we will have to restore it to this value.
	size_t uOffsetEndOld = pTemplate->uOffsetEnd;

	// Parse the input using an LL(n) parser.
	// Define the parser output.
	size_t uResultOffset = 0;
	// Define parser states.
	enum {
		kDelimiter,
		kSingleQuoted,
		kDoubleQuoted,
		kPlain,
	} eState = kDelimiter;
	// A token here consists of a single character.
	wchar_t wcToken;
	for(;;){
		wcToken = pwszRawString[uResultOffset];
		switch(eState){
		case kDelimiter:
			if((wcToken == L' ') || (wcToken == L'\t')){
				eState = kDelimiter;
				break;
			
			if(wcToken == L'\''){
				eState = kSingleQuoted;
				break;
			} 
			if(wcToken == L'\"'){
				eState = kDoubleQuoted;
				break;
			}
			goto jNormalMayBeEscaped;
		case kPlain:
			if((wcToken == L' ') || (wcToken == L'\t')){
				if(!PushSegment(pTemplate, &kSegmentEndOfString, 0, 0, 0)){
					pTemplate->uOffsetEnd = uOffsetEndOld;
					*peResult = MCFBUILD_kStringTemplateParseNotEnoughMemory;
					*puResultOffset = uResultOffset;
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					return false;
				}
				eState = kDelimiter;
				break;
			}
			if(wcToken == L'\''){
				eState = kSingleQuoted;
				break;
			}
			if(wcToken == L'\"'){
				eState = kDoubleQuoted;
				break;
			} 
		jNormalMayBeEscaped:
			if(!PushSegment(pTemplate, &kSegmentLiteral, 0, &wcToken, sizeof(wcToken))){
				pTemplate->uOffsetEnd = uOffsetEndOld;
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return false;
			}
			eState = kPlain;
			break;
		case kSingleQuoted:
			if(wcToken == 0){
				pTemplate->uOffsetEnd = uOffsetEndOld;
				*peResult = MCFBUILD_kStringTemplateParseSingleQuoteUnclosed;
				*puResultOffset = uResultOffset;
				SetLastError(ERROR_INVALID_DATA);
				return false;
			}
			if(wcToken == L'\''){
				eState = kSingleQuoted;
				break;
			}
			if(!PushSegment(pTemplate, &kSegmentLiteral, 0, &wcToken, sizeof(wcToken))){
				pTemplate->uOffsetEnd = uOffsetEndOld;
				*peResult = MCFBUILD_kStringTemplateParseNotEnoughMemory;
				*puResultOffset = uResultOffset;
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return false;
			}
			eState = kPlain;
			break;
		case kDoubleQuoted:
			if(wcToken == 0){
				pTemplate->uOffsetEnd = uOffsetEndOld;
				*peResult = MCFBUILD_kStringTemplateParseDoubleQuoteUnclosed;
				*puResultOffset = uResultOffset;
				SetLastError(ERROR_INVALID_DATA);
				return false;
			}
			if(wcToken == L'\"'){
				eState = kDoubleQuoted;
				break;
			} 
			goto jNormalMayBeEscaped;
		default:
			__builtin_trap();
		}
		if(wcToken == 0){
			break;
		}
		++uResultOffset;
	}
/*
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentLiteral, 0, L"fghi", 6);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentLiteral, 0, L"ijk", 6);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentEndOfString, 0, 0, 0);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentLiteral, 0, L"ABCDE\n", 6);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentFromStack, 12, 0, 0);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentFromMap, 0, L"key2", 10);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentLiteral, 0, L"DEF", 6);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentLiteral, 0, L"G", 2);
	PushSegment(pTemplate, &uLastCapacityTaken, kSegmentFromStack, 0, 0, 0);
*/
	*peResult = MCFBUILD_kStringTemplateParseSuccess;
	*puResultOffset = uResultOffset;
	return true;
}

typedef struct tagStringVectorHeader {
	size_t uStringCount;
	MCFBUILD_NaiveString aStrings[];
} StringVectorHeader;

static StringVectorHeader *CreateStringVector(size_t uStringCount){
	size_t uSizeToAlloc;
	if(__builtin_mul_overflow(uStringCount, sizeof(MCFBUILD_NaiveString), &uSizeToAlloc)){
		return 0;
	}
	if(__builtin_add_overflow(uSizeToAlloc, sizeof(StringVectorHeader), &uSizeToAlloc)){
		return 0;
	}
	StringVectorHeader *pHeader = malloc(uSizeToAlloc);
	if(!pHeader){
		return 0;
	}
	pHeader->uStringCount = uStringCount;
	for(size_t uIndex = 0; uIndex < pHeader->uStringCount; ++uIndex){
		MCFBUILD_NaiveStringConstruct(pHeader->aStrings + uIndex);
	}
	return pHeader;
}
static void DestroyStringVector(StringVectorHeader *pHeader){
	for(size_t uIndex = 0; uIndex < pHeader->uStringCount; ++uIndex){
		MCFBUILD_NaiveStringDestruct(pHeader->aStrings + uIndex);
	}
	free(pHeader);
}

bool MCFBUILD_StringTemplateBuildStringVector(MCFBUILD_NaiveString **restrict ppStringVector, size_t *restrict puStringCount, const MCFBUILD_StringTemplate *restrict pTemplate, const MCFBUILD_StringStack *restrict pStack, const MCFBUILD_VariableMap *restrict pMap){
	// Estimate the number of strings.
	size_t uStringCount = 0;
	// Behave as if there were an EOS mark before the very first segment.
	SegmentType eLastSegmentType = kSegmentEndOfString;
	// Iterate from beginning to end, accumulating number of EOS marks on the way.
	const unsigned char *pbyStorage = pTemplate->pbyStorage;
	const Segment *pSegment;
	size_t uOffsetCursor = 0;
	while(uOffsetCursor != pTemplate->uOffsetEnd){
		pSegment = (const void *)(pbyStorage + uOffsetCursor);
		size_t uCapacityTaken = sizeof(Segment) + GetSizeWithPadding(pSegment->byType, pSegment->uParam);
		eLastSegmentType = pSegment->byType;
		// If an EOS mark is encountered, create a new string.
		if(eLastSegmentType == kSegmentEndOfString){
			++uStringCount;
		}
		// Scan the next segment.
		uOffsetCursor += uCapacityTaken;
	}
	// If the last segment is not terminated by an EOS mark, it is probably due to a backslash at the EOF. Create a new string for the last segment anyway.
	if(eLastSegmentType != kSegmentEndOfString){
		++uStringCount;
	}
	// Allocate the strings now.
	StringVectorHeader *pHeader = CreateStringVector(uStringCount);
	if(!pHeader){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	// Initialize the strings.
	MCFBUILD_NaiveString *pStringWrite = pHeader->aStrings;
	uOffsetCursor = 0;
	while(uOffsetCursor != pTemplate->uOffsetEnd){
		pSegment = (const void *)(pbyStorage + uOffsetCursor);
		size_t uCapacityTaken = sizeof(Segment) + GetSizeWithPadding(pSegment->byType, pSegment->uParam);
		eLastSegmentType = pSegment->byType;
		if(eLastSegmentType == kSegmentEndOfString){
			// Aim at the next string.
			++pStringWrite;
		} else {
			const wchar_t *pwcString;
			size_t uLength;
			// Obtain the string to append.
			switch(eLastSegmentType){
				bool bSuccess;
			case kSegmentEndOfString:
				__builtin_trap();
			case kSegmentLiteral:
				// Append the payload to this string.
				pwcString = pSegment->awcString;
				uLength = (pSegment->uParam - pSegment->bySizePadded) / sizeof(wchar_t);
				break;
			case kSegmentFromStack:
				// Get the Nth segment from the stack.
				if(pSegment->uParam == 0){
					// Fast path: If the index is zero, just get the top.
					bSuccess = MCFBUILD_StringStackGetTop(&pwcString, &uLength, pStack);
				} else {
					// Slow path: Iterate from top to bottom. Stop as soon as N segments are skipped.
					MCFBUILD_StringStackEnumerationCookie vCookie;
					MCFBUILD_StringStackEnumerateBegin(&vCookie, pStack);
					size_t uRemaining = pSegment->uParam + 1;
					do {
						bSuccess = MCFBUILD_StringStackEnumerateNext(&pwcString, &uLength, &vCookie);
					} while(bSuccess && (--uRemaining != 0));
				}
				uLength &= -(size_t)bSuccess;
				break;
			case kSegmentFromMap:
				// Get the value designated by this key from the map.
				bSuccess = MCFBUILD_VariableMapGet(&pwcString, &uLength, pMap, pSegment->awcString);
				uLength &= -(size_t)bSuccess;
				break;
			default:
				__builtin_trap();
			}
			// Append it.
			if((uLength != 0) && !MCFBUILD_NaiveStringAppend(pStringWrite, pwcString, uLength)){
				DestroyStringVector(pHeader);
				MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return false;
			}
		}
		// Scan the next segment.
		uOffsetCursor += uCapacityTaken;
	}
	*ppStringVector = (void *)((unsigned char *)pHeader + sizeof(StringVectorHeader));
	*puStringCount = uStringCount;
	return true;
}
void MCFBUILD_StringTemplateFreeStringVector(MCFBUILD_NaiveString *pStringVector){
	if(!pStringVector){
		return;
	}
	DestroyStringVector((void *)((unsigned char *)pStringVector - sizeof(StringVectorHeader)));
}
