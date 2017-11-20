// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_SHA256_H_
#define MCFBUILD_SHA256_H_

#include "common.h"

MCFBUILD_EXTERN_C_BEGIN

typedef struct MCFBUILD_tagSha256Context {
	MCFBUILD_STD uint32_t au32Regs[8];
	MCFBUILD_STD uint8_t au8Chunk[64];
	MCFBUILD_STD size_t uChunkOffset;
	MCFBUILD_STD uint64_t u64BitsTotal;
} MCFBUILD_Sha256Context;

extern void MCFBUILD_Sha256Initialize(MCFBUILD_Sha256Context *pContext) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_Sha256Update(MCFBUILD_Sha256Context *MCFBUILD_RESTRICT pContext, const void *MCFBUILD_RESTRICT pData, MCFBUILD_STD size_t uSize) MCFBUILD_NOEXCEPT;
extern void MCFBUILD_Sha256Finalize(MCFBUILD_STD uint8_t (*MCFBUILD_RESTRICT pau8Result)[32], MCFBUILD_Sha256Context *MCFBUILD_RESTRICT pContext) MCFBUILD_NOEXCEPT;

MCFBUILD_EXTERN_C_END

#endif
