// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_STRING_STACK_H_
#define MCFBUILD_STRING_STACK_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef struct MCFBUILD_tagStringStack {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uOffsetTop;
	MCFBUILD_STD size_t uOffsetEnd;
} MCFBUILD_StringStack;

extern void MCFBUILD_StringStackInitialize(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_StringStackUninitialize(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_StringStackClear(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackGetTop(const wchar_t **MCFBUILD_RESTRICT ppwszString, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, const MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackPush(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, const wchar_t *MCFBUILD_RESTRICT pwcString, MCFBUILD_STD size_t uLength) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackPushNullTerminated(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, const wchar_t *MCFBUILD_RESTRICT pwszString) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackPop(MCFBUILD_StringStack *pStack) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_StringStackSerialize(void **MCFBUILD_RESTRICT ppData, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puSize, const MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_StringStackFreeSerializedBuffer(void *pData) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_StringStackDeserialize(MCFBUILD_StringStack *MCFBUILD_RESTRICT pStack, void *MCFBUILD_RESTRICT pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
