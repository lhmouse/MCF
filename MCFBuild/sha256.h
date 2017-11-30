// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_SHA256_H_
#define MCFBUILD_SHA256_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef MCFBUILD_STD uint8_t MCFBUILD_Sha256[32];

typedef struct MCFBUILD_tagSha256Context {
	MCFBUILD_STD uint32_t au32Regs[8];
	MCFBUILD_STD uint8_t au8Chunk[64];
	MCFBUILD_STD size_t uChunkOffset;
	MCFBUILD_STD uint64_t u64BitsTotal;
} MCFBUILD_Sha256Context;

// These functions calculate the SHA-256 checksum of arbitrary data.
extern void MCFBUILD_Sha256Initialize(MCFBUILD_Sha256Context *pContext) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_Sha256Update(MCFBUILD_Sha256Context *MCFBUILD_RESTRICT pContext, const void *MCFBUILD_RESTRICT pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_Sha256Finalize(MCFBUILD_Sha256 *MCFBUILD_RESTRICT pau8Sha256, MCFBUILD_Sha256Context *MCFBUILD_RESTRICT pContext) MCFBUILD_NOEXCEPT;

// This function works everything out in a single call.
extern void MCFBUILD_Sha256Simple(MCFBUILD_Sha256 *pau8Sha256, const void *pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;

// This function converts an SHA-256 checksum to a hexadecimal string and returns the number of characters written, not including the null terminator, if any.
// If the buffer is smaller than 64 characters, the string is truncated and no null terminator is appended.
// If the buffer is larger than 64 characters, a null terminator is appended to the string.
extern MCFBUILD_STD size_t MCFBUILD_Sha256Print(wchar_t *MCFBUILD_RESTRICT pwcBuffer, MCFBUILD_STD size_t uBufferLength, const MCFBUILD_Sha256 *MCFBUILD_RESTRICT pau8Sha256, bool bUpperCase) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
