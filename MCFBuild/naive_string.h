// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_NAIVE_STRING_H_
#define MCFBUILD_NAIVE_STRING_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef struct MCFBUILD_tagSha256 MCFBUILD_Sha256;

typedef struct MCFBUILD_tagNaiveString {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uSize;
} MCFBUILD_NaiveString;

extern const MCFBUILD_NaiveString *MCFBUILD_NaiveStringGetEmpty(void) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_NaiveStringConstruct(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_NaiveStringDestruct(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_NaiveStringMove(MCFBUILD_NaiveString *MCFBUILD_RESTRICT pString, MCFBUILD_NaiveString *MCFBUILD_RESTRICT pSource) MCFBUILD_NOEXCEPT;

// This function returns a pointer to a null-terminated, immutable string.
// This function will never return a null pointer.
extern const wchar_t *MCFBUILD_NaiveStringGetNullTerminated(const MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
// This function returns a pointer to a potentially unterminated, mutable string.
// This function may return a null pointer if `*pString` is empty.
extern wchar_t *MCFBUILD_NaiveStringGetData(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern MCFBUILD_STD size_t MCFBUILD_NaiveStringGetLength(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_NaiveStringIsEmpty(const MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
// Fundamental modifiers.
extern void MCFBUILD_NaiveStringClear(MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringReserve(wchar_t **MCFBUILD_RESTRICT ppwcCaret, MCFBUILD_NaiveString *pString, MCFBUILD_STD size_t uInsertAt, MCFBUILD_STD size_t uLengthToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringRemove(MCFBUILD_NaiveString *pString, MCFBUILD_STD size_t uRemoveFrom, MCFBUILD_STD size_t uLengthToRemove) MCFBUILD_NOEXCEPT;

// Hash support.
extern void MCFBUILD_NaiveStringGetSha256(MCFBUILD_Sha256 *pau8Sha256, const MCFBUILD_NaiveString *pString) MCFBUILD_NOEXCEPT;

// Wrappers for convenience.
extern bool MCFBUILD_NaiveStringInsert(MCFBUILD_NaiveString *MCFBUILD_RESTRICT pString, MCFBUILD_STD size_t uInsertAt, const wchar_t *MCFBUILD_RESTRICT pwcStringToInsert, MCFBUILD_STD size_t uLengthToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringInsertNullTerminated(MCFBUILD_NaiveString *MCFBUILD_RESTRICT pString, MCFBUILD_STD size_t uInsertAt, const wchar_t *MCFBUILD_RESTRICT pwszStringToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringInsertRepeated(MCFBUILD_NaiveString *MCFBUILD_RESTRICT pString, MCFBUILD_STD size_t uInsertAt, wchar_t wcCharacterToInsert, MCFBUILD_STD size_t uLengthToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringAppend(MCFBUILD_NaiveString *MCFBUILD_RESTRICT pString, const wchar_t *MCFBUILD_RESTRICT pwcStringToInsert, MCFBUILD_STD size_t uLengthToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringAppendNullTerminated(MCFBUILD_NaiveString *MCFBUILD_RESTRICT pString, const wchar_t *MCFBUILD_RESTRICT pwszStringToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringPush(MCFBUILD_NaiveString *pString, wchar_t wcCharacterToInsert, MCFBUILD_STD size_t uLengthToInsert) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_NaiveStringPop(MCFBUILD_NaiveString *pString, MCFBUILD_STD size_t uLengthToRemove) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
