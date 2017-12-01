// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_VARIABLE_MAP_H_
#define MCFBUILD_VARIABLE_MAP_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

extern bool MCFBUILD_VariableMapIsKeyValid(const wchar_t *pwszKey) MCFBUILD_NOEXCEPT;

typedef struct MCFBUILD_tagVariableMap {
	unsigned char *pbyStorage;
	MCFBUILD_STD size_t uCapacity;
	MCFBUILD_STD size_t uOffsetEnd;
} MCFBUILD_VariableMap;

extern void MCFBUILD_VariableMapConstruct(MCFBUILD_VariableMap *pMap) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_VariableMapDestruct(MCFBUILD_VariableMap *pMap) MCFBUILD_NOEXCEPT;

extern void MCFBUILD_VariableMapClear(MCFBUILD_VariableMap *pMap) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapGet(const wchar_t **MCFBUILD_RESTRICT ppwszValue, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *pwszKey) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapSet(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *MCFBUILD_RESTRICT pwszKey, const wchar_t *MCFBUILD_RESTRICT pwcValue, MCFBUILD_STD size_t uLength) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapSetNullTerminated(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *MCFBUILD_RESTRICT pwszKey, const wchar_t *MCFBUILD_RESTRICT ppwszValue) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapUnset(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const wchar_t *MCFBUILD_RESTRICT pwszKey) MCFBUILD_NOEXCEPT;

typedef struct MCFBUILD_tagVariableMapEnumerationCookie {
	MCFBUILD_STD size_t uOffsetNext;
} MCFBUILD_VariableMapEnumerationCookie;

// The user shall initialize `*pCookie` to `{ 0 }` before calling this function.
// Upon each call, this function alters `*pCookie` to designate the next element if any, sets up `*ppwszKey`, `*ppwszValue` and `*puLength` accordingly, then returns `true`.
// If there are no more elements, this function returns `false`.
extern bool MCFBUILD_VariableMapEnumerate(const wchar_t **MCFBUILD_RESTRICT ppwszKey, const wchar_t **MCFBUILD_RESTRICT ppwszValue, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puLength, MCFBUILD_VariableMapEnumerationCookie *MCFBUILD_RESTRICT pCookie, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap) MCFBUILD_NOEXCEPT;

extern bool MCFBUILD_VariableMapSerialize(void **MCFBUILD_RESTRICT ppData, MCFBUILD_STD size_t *MCFBUILD_RESTRICT puSize, const MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_VariableMapFreeSerializedBuffer(void *pData) MCFBUILD_NOEXCEPT;
extern bool MCFBUILD_VariableMapDeserialize(MCFBUILD_VariableMap *MCFBUILD_RESTRICT pMap, const void *MCFBUILD_RESTRICT pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
