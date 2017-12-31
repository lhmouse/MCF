// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

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

static inline unsigned long MakeParseResult(MCFBUILD_StringTemplateParseResult *restrict peResult, unsigned long ulErrorCode, MCFBUILD_StringTemplateParseResult eResult){
	*peResult = eResult;
	return ulErrorCode;
}
static unsigned long PushSegment(MCFBUILD_StringTemplateParseResult *restrict peResult, MCFBUILD_StringTemplate *restrict pTemplate, size_t *restrict puLastCapacityTaken, SegmentType eType, size_t uParam, const wchar_t *pwcString, size_t uSizeOfString){
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
		return MakeParseResult(peResult, ERROR_NOT_ENOUGH_MEMORY, MCFBUILD_kStringTemplateParseNotEnoughMemory);
	}
	unsigned char *pbyStorage = pTemplate->pbyStorage;
	size_t uCapacity = pTemplate->uCapacity;
	if(uCapacity < uMinimumSizeToReserve){
		uCapacity += uCapacity / 2;
		uCapacity |= uMinimumSizeToReserve;
		uCapacity |= 0x400;
		pbyStorage = realloc(pbyStorage, uCapacity);
		if(!pbyStorage){
			return MakeParseResult(peResult, ERROR_NOT_ENOUGH_MEMORY, MCFBUILD_kStringTemplateParseNotEnoughMemory);
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
			// Pop the last segment, then merge into it!
			pTemplate->uOffsetEnd -= uLastCapacityTaken;
			size_t uSizeOfStringOld = pSegment->uParam - pSegment->bySizePadded;
			// Recalculate the size of the last segment.
			uSizeWithPadding = uSizeOfStringOld + uSizeOfString;
			bySizePadded = -uSizeWithPadding % 8;
			uSizeWithPadding += bySizePadded;
			// Extend the last segment to absorb the string.
			pSegment->bySizePadded = bySizePadded;
			pSegment->uParam = uSizeWithPadding;
			memcpy(pSegment->awcString + uSizeOfStringOld / sizeof(wchar_t), pwcString, uSizeOfString);
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
		pSegment->awcString[uSizeOfString / sizeof(wchar_t)] = 0;
		uLastCapacityTaken = sizeof(Segment) + uSizeWithPadding;
		break;
	default:
		__builtin_trap();
	}
jDone:
	pTemplate->uOffsetEnd += uLastCapacityTaken;
	*puLastCapacityTaken = uLastCapacityTaken;
	return 0;
}
static unsigned long ParseDigits(MCFBUILD_StringTemplateParseResult *restrict peResult, uint32_t *restrict pu32Value, unsigned uRadix, const wchar_t *pwcString, size_t uLength){
	static const wchar_t kHexTable[] = L"00112233445566778899aAbBcCdDeEfF";
	uint32_t u32Value = 0;
	for(size_t uIndex = 0; uIndex < uLength; ++uIndex){
		wchar_t wcDigit = pwcString[uIndex];
		if(wcDigit == 0){
			return MakeParseResult(peResult, ERROR_HANDLE_EOF, MCFBUILD_kStringTemplateParseDigitsTooFew);
		}
		const wchar_t *pwcFound = wmemchr(kHexTable, wcDigit, uRadix * 2);
		if(!pwcFound){
			return MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseDigitsInvalid);
		}
		u32Value *= uRadix;
		u32Value += (unsigned)(pwcFound - kHexTable) / 2;
	}
	*pu32Value = u32Value;
	return 0;
}
static unsigned long ParseAndPushEscapeable(MCFBUILD_StringTemplateParseResult *restrict peResult, size_t *restrict puTokenLength, MCFBUILD_StringTemplate *restrict pTemplate, size_t *restrict puLastCapacityTaken, const wchar_t *pwszToken){
	unsigned long ulErrorCode;
	uint32_t u32Value;
	wchar_t awcString[4];
	const wchar_t *pwcPosition;
	size_t uLength, uInvalidIndex;
	switch(pwszToken[0]){
	case L'\\':
		switch(pwszToken[1]){
		case L'\\':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\\", sizeof(wchar_t));
			break;
		case L'\'':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\'", sizeof(wchar_t));
			break;
		case L'\"':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\"", sizeof(wchar_t));
			break;
		case L'?':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"?", sizeof(wchar_t));
			break;
		case L'$':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"$", sizeof(wchar_t));
			break;
		case L'#':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"#", sizeof(wchar_t));
			break;
		case L'a':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\a", sizeof(wchar_t));
			break;
		case L'b':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\b", sizeof(wchar_t));
			break;
		case L'f':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\f", sizeof(wchar_t));
			break;
		case L'n':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\n", sizeof(wchar_t));
			break;
		case L'r':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\r", sizeof(wchar_t));
			break;
		case L't':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\t", sizeof(wchar_t));
			break;
		case L'v':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"\v", sizeof(wchar_t));
			break;
		case L'x':
			*puTokenLength = 4;
			ulErrorCode = ParseDigits(peResult, &u32Value, 16, pwszToken + 2, 2);
			if(ulErrorCode != 0){
				break;
			}
			awcString[0] = (uint8_t)u32Value;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, awcString, sizeof(wchar_t));
			break;
		case L'u':
			*puTokenLength = 6;
			ulErrorCode = ParseDigits(peResult, &u32Value, 16, pwszToken + 2, 4);
			if(ulErrorCode != 0){
				break;
			}
			awcString[0] = (uint16_t)u32Value;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, awcString, sizeof(wchar_t));
			break;
		case L'U':
			*puTokenLength = 10;
			ulErrorCode = ParseDigits(peResult, &u32Value, 16, pwszToken + 2, 8);
			if(ulErrorCode != 0){
				break;
			}
			if(u32Value < 0x10000){
				if(u32Value - 0xD800 < 0x800){
					ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseUtfCodePointInvalid);
					break;
				}
				awcString[0] = (uint16_t)u32Value;
				ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, awcString, sizeof(wchar_t));
			} else if(u32Value < 0x110000){
				awcString[0] = (char16_t)((((u32Value - 0x10000)      ) >> 10) + 0xD800);
				awcString[1] = (char16_t)((((u32Value          ) << 22) >> 22) + 0xDC00);
				ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, awcString, sizeof(wchar_t) * 2);
			} else {
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseUtfCodePointInvalid);
				break;
			}
			break;
		default:
			ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseEscapeInvalid);
			break;
		}
		break;
	case L'$':
		switch(pwszToken[1]){
		case L'$':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, L"$", sizeof(wchar_t));
			break;
		case L'[':
			pwcPosition = wcschr(pwszToken + 2, L']');
			if(!pwcPosition){
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseSquareBracketUnclosed);
				break;
			}
			uLength = (size_t)(pwcPosition - (pwszToken + 2));
			if(uLength == 0){
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseStackSubscriptInvalid);
				break;
			}
			if(uLength > 4){
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseStackSubscriptInvalid);
				break;
			}
			*puTokenLength = uLength + 3;
			ulErrorCode = ParseDigits(peResult, &u32Value, 10, pwszToken + 2, uLength);
			if(ulErrorCode != 0){
				break;
			}
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentFromStack, u32Value, 0, 0);
			break;
		case L'0':  case L'1':  case L'2':  case L'3':  case L'4':
		case L'5':  case L'6':  case L'7':  case L'8':  case L'9':
			*puTokenLength = 2;
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentFromStack, (uint16_t)(pwszToken[1] - L'0'), 0, 0);
			break;
		case L'{':
			pwcPosition = wcschr(pwszToken + 2, L'}');
			if(!pwcPosition){
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseBraceUnclosed);
				break;
			}
			uLength = (size_t)(pwcPosition - (pwszToken + 2));
			if(uLength == 0){
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseKeyInvalid);
				break;
			}
			*puTokenLength = uLength + 3;
			MCFBUILD_VariableMapValidateKey(&uInvalidIndex, pwszToken + 2);
			if(uInvalidIndex < uLength){
				ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseKeyInvalid);
				break;
			}
			ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentFromMap, 0, pwszToken + 2, uLength);
			break;
		default:
			ulErrorCode = MakeParseResult(peResult, ERROR_INVALID_DATA, MCFBUILD_kStringTemplateParseReplacementInvalid);
			break;
		}
		break;
	default:
		// *puTokenLength = 1;
		ulErrorCode = PushSegment(peResult, pTemplate, puLastCapacityTaken, kSegmentLiteral, 0, pwszToken, sizeof(wchar_t));
		break;
	}
	return ulErrorCode;
}

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

bool MCFBUILD_StringTemplateIsEmpty(const MCFBUILD_StringTemplate *pTemplate){
	return pTemplate->uOffsetEnd == 0;
}
void MCFBUILD_StringTemplateClear(MCFBUILD_StringTemplate *pTemplate){
	pTemplate->uOffsetEnd = 0;
}
bool MCFBUILD_StringTemplateParse(MCFBUILD_StringTemplate *restrict pTemplate, MCFBUILD_StringTemplateParseResult *restrict peResult, size_t *restrict puResultOffset, const wchar_t *restrict pwszRawString){
	// Save the offset to data end. If anything goes wrong we just reset it to this value.
	size_t uOffsetEndOld = pTemplate->uOffsetEnd;
	// Parse the input using an LL(n) parser, appending data to the string template as we walk.
	// Do not merge into segments existent before this call, otherwise it will not be possible to restore the string template in case of errors.
	size_t uLastCapacityTaken = 0;
	// Define the output.
	unsigned long ulErrorCode = 0;
	size_t uResultOffset = 0;
	// Define parser states.
	enum {
		kDelimiter,
		kSingleQuoted,
		kDoubleQuoted,
		kLiteral,
	} eState = kDelimiter;
	// Parse one character in each loop.
	for(;;){
		wchar_t wcToken = pwszRawString[uResultOffset];
		size_t uTokenLength = 1;
		switch(eState){
		case kDelimiter:
			switch(wcToken){
			case 0:
			case L'#':
				ulErrorCode = MakeParseResult(peResult, 0, MCFBUILD_kStringTemplateParseSuccess);
				goto jDone;
			case L' ':
			case L'\t':
				// eState = kDelimiter;
				break;
			case L'\'':
				eState = kSingleQuoted;
				break;
			case L'\"':
				eState = kDoubleQuoted;
				break;
			case L'\\':
				if(pwszRawString[uResultOffset + 1] == 0){
					ulErrorCode = MakeParseResult(peResult, 0, MCFBUILD_kStringTemplateParsePartial);
					goto jDone;
				}
				// Fallthrough
			default:
				ulErrorCode = ParseAndPushEscapeable(peResult, &uTokenLength, pTemplate, &uLastCapacityTaken, pwszRawString + uResultOffset);
				if(ulErrorCode != 0){
					goto jDone;
				}
				eState = kLiteral;
				break;
			}
			break;
		case kSingleQuoted:
			switch(wcToken){
			case 0:
				ulErrorCode = MakeParseResult(peResult, ERROR_HANDLE_EOF, MCFBUILD_kStringTemplateParseSingleQuoteUnclosed);
				goto jDone;
			case L'\'':
				eState = kLiteral;
				break;
			default:
				ulErrorCode = PushSegment(peResult, pTemplate, &uLastCapacityTaken, kSegmentLiteral, 0, &wcToken, sizeof(wcToken));
				if(ulErrorCode != 0){
					goto jDone;
				}
				// eState = kSingleQuoted;
				break;
			}
			break;
		case kDoubleQuoted:
			switch(wcToken){
			case 0:
				ulErrorCode = MakeParseResult(peResult, ERROR_HANDLE_EOF, MCFBUILD_kStringTemplateParseDoubleQuoteUnclosed);
				goto jDone;
			case L'\"':
				eState = kLiteral;
				break;
			case L'\\':
				if(pwszRawString[uResultOffset + 1] == 0){
					// Double quotes must be closed on the same line. If this is not the case, fail.
					ulErrorCode = MakeParseResult(peResult, ERROR_HANDLE_EOF, MCFBUILD_kStringTemplateParseDoubleQuoteUnclosed);
					goto jDone;
				}
				// Fallthrough
			default:
				ulErrorCode = ParseAndPushEscapeable(peResult, &uTokenLength, pTemplate, &uLastCapacityTaken, pwszRawString + uResultOffset);
				if(ulErrorCode != 0){
					goto jDone;
				}
				// eState = kDoubleQuoted;
				break;
			}
			break;
		case kLiteral:
			switch(wcToken){
			case 0:
			case L'#':
				ulErrorCode = PushSegment(peResult, pTemplate, &uLastCapacityTaken, kSegmentEndOfString, 0, 0, 0);
				if(ulErrorCode != 0){
					goto jDone;
				}
				ulErrorCode = MakeParseResult(peResult, 0, MCFBUILD_kStringTemplateParseSuccess);
				goto jDone;
			case L' ':
			case L'\t':
				ulErrorCode = PushSegment(peResult, pTemplate, &uLastCapacityTaken, kSegmentEndOfString, 0, 0, 0);
				if(ulErrorCode != 0){
					goto jDone;
				}
				eState = kDelimiter;
				break;
			case L'\'':
				eState = kSingleQuoted;
				break;
			case L'\"':
				eState = kDoubleQuoted;
				break;
			case L'\\':
				if(pwszRawString[uResultOffset + 1] == 0){
					ulErrorCode = MakeParseResult(peResult, 0, MCFBUILD_kStringTemplateParsePartial);
					goto jDone;
				}
				// Fallthrough
			default:
				ulErrorCode = ParseAndPushEscapeable(peResult, &uTokenLength, pTemplate, &uLastCapacityTaken, pwszRawString + uResultOffset);
				if(ulErrorCode != 0){
					goto jDone;
				}
				// eState = kLiteral;
				break;
			}
			break;
		default:
			__builtin_trap();
		}
		uResultOffset += uTokenLength;
	}
jDone:
	*puResultOffset = uResultOffset;
	if(ulErrorCode != 0){
		pTemplate->uOffsetEnd = uOffsetEndOld;
		MCFBUILD_SetLastError(ulErrorCode);
		return false;
	}
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
