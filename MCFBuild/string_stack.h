// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_STRING_STACK_H_
#define MCFBUILD_STRING_STACK_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef struct MCFBUILD_tagStringStack {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uOffsetBegin;
} MCFBUILD_StringStack;

typedef struct MCFBUILD_tagStringStackEnumerationCookie {
	const MCFBUILD_StringStack *pStack;
	MCFBUILD_STD size_t uOffsetNext;
} MCFBUILD_StringStackEnumerationCookie;

extern const MCFBUILD_StringStack *MCFBUILD_StringStackGetEmpty(void) MCFBUILD_NOEXCEPT;

// This function constructs an empty `MCFBUILD_StringStack`.
extern void MCFBUILD_StringStackConstruct(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;
// This function destructs a `MCFBUILD_StringStack`, deallocating any storage it has allocated.
extern void MCFBUILD_StringStackDestruct(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;
// This function constructs a `MCFBUILD_StringStack` using the contents from `*pSource` and destructs it thereafter.
extern void MCFBUILD_StringStackMove(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, MCFBUILD_StringStack *MCFBUILD_RESTRICT pSource) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_StringStackIsEmpty(const MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackGetTop(const wchar_t **MCFBUILD_RESTRICT ppwszString, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, const MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, const wchar_t *MCFBUILD_RESTRICT pwcString, MCFBUILD_STD size_t uLength) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, const wchar_t *MCFBUILD_RESTRICT pwszString) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_StringStackEnumerateBegin(MCFBUILD_StringStackEnumerationCookie *MCFBUILD_RESTRICT pCookie, const MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackEnumerateNext(const wchar_t **MCFBUILD_RESTRICT ppwszString, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, MCFBUILD_StringStackEnumerationCookie *MCFBUILD_RESTRICT pCookie) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_StringStackSerialize(void **MCFBUILD_RESTRICT ppData, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puSize, const MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_StringStackFreeSerializedBuffer(void *pData) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackDeserialize(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, const void *MCFBUILD_RESTRICT pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
