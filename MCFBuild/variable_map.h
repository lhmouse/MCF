// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_VARIABLE_MAP_H_
#define MCFBUILD_VARIABLE_MAP_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

extern bool MCFBUILD_VariableMapValidateKey(MCFBUILD_STD size_t *MCFBUILD_RESTRICT puInvalidIndex, const wchar_t *pwszKey) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapIsKeyValid(const wchar_t *pwszKey) MCFBUILD_NOEXCEPT;

typedef struct MCFBUILD_tagVariableMap {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uOffsetEnd;
} MCFBUILD_VariableMap;

typedef struct MCFBUILD_tagVariableMapEnumerationCookie {
	const MCFBUILD_VariableMap *pMap;
	MCFBUILD_STD size_t uOffsetNext;
} MCFBUILD_VariableMapEnumerationCookie;

extern const MCFBUILD_VariableMap *MCFBUILD_VariableMapGetEmpty(void) MCFBUILD_NOEXCEPT;

// This function constructs an empty `MCFBUILD_VariableMap`.
extern void MCFBUILD_VariableMapConstruct(MCFBUILD_VariableMap *pMap) MCFBUILD_NOEXCEPT;
// This function destructs a `MCFBUILD_VariableMap`, deallocating any storage it has allocated.
extern void MCFBUILD_VariableMapDestruct(MCFBUILD_VariableMap *pMap) MCFBUILD_NOEXCEPT;
// This function constructs a `MCFBUILD_VariableMap` using the contents from `*pSource` and destructs it thereafter.
extern void MCFBUILD_VariableMapMove(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, MCFBUILD_VariableMap *MCFBUILD_RESTRICT pSource) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_VariableMapClear(MCFBUILD_VariableMap *pMap) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapGet(const wchar_t **MCFBUILD_RESTRICT ppwszValue, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *pwszKey) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapSet(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *MCFBUILD_RESTRICT pwszKey, const wchar_t *MCFBUILD_RESTRICT pwcValue, MCFBUILD_STD size_t uLength) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapSetNullTerminated(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *MCFBUILD_RESTRICT pwszKey, const wchar_t *MCFBUILD_RESTRICT ppwszValue) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapUnset(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *MCFBUILD_RESTRICT pwszKey) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_VariableMapEnumerateBegin(MCFBUILD_VariableMapEnumerationCookie *MCFBUILD_RESTRICT pCookie, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapEnumerateNext(const wchar_t **MCFBUILD_RESTRICT ppwszKey, const wchar_t **MCFBUILD_RESTRICT ppwszValue, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, MCFBUILD_VariableMapEnumerationCookie *MCFBUILD_RESTRICT pCookie) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_VariableMapSerialize(void **MCFBUILD_RESTRICT ppData, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puSize, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_VariableMapFreeSerializedBuffer(void *pData) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapDeserialize(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const void *MCFBUILD_RESTRICT pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
