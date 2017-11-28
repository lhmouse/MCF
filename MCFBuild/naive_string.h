// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_NAIVE_STRING_H_
#define MCFBUILD_NAIVE_STRING_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef struct MCFBUILD_tagNaiveString {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uSize;
} MCFBUILD_NaiveString;

extern void MCFBUILD_NaiveStringInitialize(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_NaiveStringUninitialize(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;

// This function returns a pointer to a null-terminated, immutable string.
// This function will never return a null pointer.
extern const wchar_t *MCFBUILD_NaiveStringGetNullTerminated(const MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
// This function returns a pointer to a potentially unterminated, mutable string.
// This function may return a null pointer if `*pString` is empty.
extern wchar_t *MCFBUILD_NaiveStringGetData(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern MCFBUILD_STD size_t MCFBUILD_NaiveStringGetLength(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_NaiveStringClear(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringReserve(wchar_t **MCFBUILD_RESTRICT ppwcCaret, MCFBUILD_NaiveString *pString, MCFBUILD_STD size_t uInsertAt, MCFBUILD_STD size_t uLengthToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringRemove(MCFBUILD_NaiveString *pString, MCFBUILD_STD size_t uRemoveFrom, MCFBUILD_STD size_t uLengthToRemove) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
