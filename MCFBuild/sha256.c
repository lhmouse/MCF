// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "sha256.h"
#include <x86intrin.h>

// https://en.wikipedia.org/wiki/SHA-2

static void EatChunk(MCFBUILD_Sha256Context *restrict pContext, const unsigned char *restrict pbyChunk){
	uint32_t s0, maj, t2, s1, ch, t1;

	uint32_t w[64];
	for(unsigned i = 0; i < 16; ++i){
		w[i] = __builtin_bswap32(((const uint32_t *)pbyChunk)[i]);
	}
	for(unsigned i = 16; i < 64; ++i){
		s0 = __rord(w[i - 15],  7) ^ __rord(w[i - 15], 18) ^ (w[i - 15] >>  3);
		s1 = __rord(w[i -  2], 17) ^ __rord(w[i -  2], 19) ^ (w[i -  2] >> 10);
		w[i] = w[i - 16] + w[i - 7] + s0 + s1;
	}

	uint32_t r[8];
	for(unsigned i = 0; i < 8; ++i){
		r[i] = pContext->au32Regs[i];
	}

#define SHA256_STEP(i_, a_, b_, c_, d_, e_, f_, g_, h_, k_)	\
	s0 = __rord(a_, 2) ^ __rord(a_, 13) ^ __rord(a_, 22);	\
	maj = (a_ & b_) | (c_ & (a_ ^ b_));	\
	t2 = s0 + maj;	\
	s1 = __rord(e_, 6) ^ __rord(e_, 11) ^ __rord(e_, 25);	\
	ch = g_ ^ (e_ & (f_ ^ g_));	\
	t1 = h_ + s1 + ch + k_ + w[i_];	\
	d_ += t1;	\
	h_ = t1 + t2;

	SHA256_STEP( 0, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0x428A2F98)
	SHA256_STEP( 1, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0x71374491)
	SHA256_STEP( 2, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0xB5C0FBCF)
	SHA256_STEP( 3, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0xE9B5DBA5)
	SHA256_STEP( 4, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0x3956C25B)
	SHA256_STEP( 5, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0x59F111F1)
	SHA256_STEP( 6, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0x923F82A4)
	SHA256_STEP( 7, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0xAB1C5ED5)

	SHA256_STEP( 8, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0xD807AA98)
	SHA256_STEP( 9, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0x12835B01)
	SHA256_STEP(10, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0x243185BE)
	SHA256_STEP(11, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0x550C7DC3)
	SHA256_STEP(12, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0x72BE5D74)
	SHA256_STEP(13, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0x80DEB1FE)
	SHA256_STEP(14, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0x9BDC06A7)
	SHA256_STEP(15, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0xC19BF174)

	SHA256_STEP(16, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0xE49B69C1)
	SHA256_STEP(17, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0xEFBE4786)
	SHA256_STEP(18, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0x0FC19DC6)
	SHA256_STEP(19, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0x240CA1CC)
	SHA256_STEP(20, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0x2DE92C6F)
	SHA256_STEP(21, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0x4A7484AA)
	SHA256_STEP(22, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0x5CB0A9DC)
	SHA256_STEP(23, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0x76F988DA)

	SHA256_STEP(24, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0x983E5152)
	SHA256_STEP(25, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0xA831C66D)
	SHA256_STEP(26, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0xB00327C8)
	SHA256_STEP(27, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0xBF597FC7)
	SHA256_STEP(28, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0xC6E00BF3)
	SHA256_STEP(29, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0xD5A79147)
	SHA256_STEP(30, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0x06CA6351)
	SHA256_STEP(31, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0x14292967)

	SHA256_STEP(32, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0x27B70A85)
	SHA256_STEP(33, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0x2E1B2138)
	SHA256_STEP(34, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0x4D2C6DFC)
	SHA256_STEP(35, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0x53380D13)
	SHA256_STEP(36, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0x650A7354)
	SHA256_STEP(37, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0x766A0ABB)
	SHA256_STEP(38, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0x81C2C92E)
	SHA256_STEP(39, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0x92722C85)

	SHA256_STEP(40, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0xA2BFE8A1)
	SHA256_STEP(41, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0xA81A664B)
	SHA256_STEP(42, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0xC24B8B70)
	SHA256_STEP(43, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0xC76C51A3)
	SHA256_STEP(44, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0xD192E819)
	SHA256_STEP(45, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0xD6990624)
	SHA256_STEP(46, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0xF40E3585)
	SHA256_STEP(47, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0x106AA070)

	SHA256_STEP(48, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0x19A4C116)
	SHA256_STEP(49, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0x1E376C08)
	SHA256_STEP(50, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0x2748774C)
	SHA256_STEP(51, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0x34B0BCB5)
	SHA256_STEP(52, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0x391C0CB3)
	SHA256_STEP(53, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0x4ED8AA4A)
	SHA256_STEP(54, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0x5B9CCA4F)
	SHA256_STEP(55, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0x682E6FF3)

	SHA256_STEP(56, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], 0x748F82EE)
	SHA256_STEP(57, r[7], r[0], r[1], r[2], r[3], r[4], r[5], r[6], 0x78A5636F)
	SHA256_STEP(58, r[6], r[7], r[0], r[1], r[2], r[3], r[4], r[5], 0x84C87814)
	SHA256_STEP(59, r[5], r[6], r[7], r[0], r[1], r[2], r[3], r[4], 0x8CC70208)
	SHA256_STEP(60, r[4], r[5], r[6], r[7], r[0], r[1], r[2], r[3], 0x90BEFFFA)
	SHA256_STEP(61, r[3], r[4], r[5], r[6], r[7], r[0], r[1], r[2], 0xA4506CEB)
	SHA256_STEP(62, r[2], r[3], r[4], r[5], r[6], r[7], r[0], r[1], 0xBEF9A3F7)
	SHA256_STEP(63, r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[0], 0xC67178F2)

	for(unsigned i = 0; i < 8; ++i){
		pContext->au32Regs[i] += r[i];
	}
}

void MCFBUILD_Sha256Initialize(MCFBUILD_Sha256Context *pContext){
	pContext->au32Regs[0] = 0x6A09E667;
	pContext->au32Regs[1] = 0xBB67AE85;
	pContext->au32Regs[2] = 0x3C6EF372;
	pContext->au32Regs[3] = 0xA54FF53A;
	pContext->au32Regs[4] = 0x510E527F;
	pContext->au32Regs[5] = 0x9B05688C;
	pContext->au32Regs[6] = 0x1F83D9AB;
	pContext->au32Regs[7] = 0x5BE0CD19;
	pContext->uChunkOffset = 0;
	pContext->u64BitsTotal = 0;
}
void MCFBUILD_Sha256Update(MCFBUILD_Sha256Context *restrict pContext, const void *restrict pData, size_t uSize){
	size_t uRemaining = uSize;
	size_t uChunkAvail = 64 - pContext->uChunkOffset;
	if(uRemaining >= uChunkAvail){
		if(pContext->uChunkOffset != 0){
			memcpy(pContext->au8Chunk + pContext->uChunkOffset, (const unsigned char *)pData + uSize - uRemaining, uChunkAvail);
			uRemaining -= uChunkAvail;
			pContext->uChunkOffset = 0;
			EatChunk(pContext, pContext->au8Chunk);
		}
		while(uRemaining >= 64){
			EatChunk(pContext, (const unsigned char *)pData + uSize - uRemaining);
			uRemaining -= 64;
		}
	}
	if(uRemaining != 0){
		memcpy(pContext->au8Chunk + pContext->uChunkOffset, (const unsigned char *)pData + uSize - uRemaining, uRemaining);
		pContext->uChunkOffset += uRemaining;
	}
	pContext->u64BitsTotal += (uint64_t)uSize * 8;
}
void MCFBUILD_Sha256Finalize(uint8_t (*restrict pau8Result)[32], MCFBUILD_Sha256Context *restrict pContext){
	pContext->au8Chunk[(pContext->uChunkOffset)++] = 0x80;
	size_t uChunkAvail = 64 - pContext->uChunkOffset;
	if(uChunkAvail < 8){
		memset(pContext->au8Chunk + pContext->uChunkOffset, 0, uChunkAvail);
		EatChunk(pContext, pContext->au8Chunk);
		pContext->uChunkOffset = 0;
		uChunkAvail = 64;
	}
	if(uChunkAvail > 8){
		memset(pContext->au8Chunk + pContext->uChunkOffset, 0, uChunkAvail - 8);
		pContext->uChunkOffset = 56;
	}
	((uint64_t *)pContext->au8Chunk)[7] = __builtin_bswap64(pContext->u64BitsTotal);
	EatChunk(pContext, pContext->au8Chunk);
	for(unsigned uIndex = 0; uIndex < 8; ++uIndex){
		((uint32_t *)pau8Result)[uIndex] = __builtin_bswap32(pContext->au32Regs[uIndex]);
	}
}
