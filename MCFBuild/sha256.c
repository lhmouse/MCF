// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "sha256.h"
#include "endian.h"

// https://en.wikipedia.org/wiki/SHA-2

static inline uint32_t rotr(uint32_t val, unsigned bits){
	return (val << (32 - bits)) | (val >> bits);
}

static inline void sha256_round(uint32_t *restrict r, int a, int b, int c, int d, int e, int f, int g, int h, uint32_t k, uint32_t x){
	uint32_t S0 = rotr(r[a], 2) ^ rotr(r[a], 13) ^ rotr(r[a], 22);
	uint32_t maj = (r[a] & r[b]) | (r[c] & (r[a] ^ r[b]));
	uint32_t t2 = S0 + maj;
	uint32_t S1 = rotr(r[e], 6) ^ rotr(r[e], 11) ^ rotr(r[e], 25);
	uint32_t ch = r[g] ^ (r[e] & (r[f] ^ r[g]));
	uint32_t t1 = r[h] + S1 + ch + k + x;
	r[d] += t1;
	r[h] = t1 + t2;
}
static void sha256_chunk(uint32_t *restrict regs, const unsigned char *restrict chunk){
	uint32_t w[64];
	for(unsigned i = 0; i < 16; ++i){
		w[i] = MCFBUILD_load_be_uint32((const uint32_t *)chunk + i);
	}
	for(unsigned i = 16; i < 64; ++i){
		uint32_t s0 = rotr(w[i - 15],  7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >>  3);
		uint32_t s1 = rotr(w[i -  2], 17) ^ rotr(w[i -  2], 19) ^ (w[i -  2] >> 10);
		w[i] = w[i - 16] + w[i - 7] + s0 + s1;
	}

	uint32_t r[8];
	for(unsigned i = 0; i < 8; ++i){
		r[i] = regs[i];
	}

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0x428A2F98, w[ 0]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0x71374491, w[ 1]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0xB5C0FBCF, w[ 2]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0xE9B5DBA5, w[ 3]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0x3956C25B, w[ 4]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0x59F111F1, w[ 5]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0x923F82A4, w[ 6]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0xAB1C5ED5, w[ 7]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0xD807AA98, w[ 8]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0x12835B01, w[ 9]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0x243185BE, w[10]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0x550C7DC3, w[11]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0x72BE5D74, w[12]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0x80DEB1FE, w[13]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0x9BDC06A7, w[14]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0xC19BF174, w[15]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0xE49B69C1, w[16]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0xEFBE4786, w[17]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0x0FC19DC6, w[18]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0x240CA1CC, w[19]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0x2DE92C6F, w[20]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0x4A7484AA, w[21]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0x5CB0A9DC, w[22]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0x76F988DA, w[23]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0x983E5152, w[24]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0xA831C66D, w[25]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0xB00327C8, w[26]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0xBF597FC7, w[27]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0xC6E00BF3, w[28]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0xD5A79147, w[29]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0x06CA6351, w[30]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0x14292967, w[31]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0x27B70A85, w[32]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0x2E1B2138, w[33]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0x4D2C6DFC, w[34]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0x53380D13, w[35]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0x650A7354, w[36]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0x766A0ABB, w[37]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0x81C2C92E, w[38]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0x92722C85, w[39]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0xA2BFE8A1, w[40]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0xA81A664B, w[41]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0xC24B8B70, w[42]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0xC76C51A3, w[43]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0xD192E819, w[44]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0xD6990624, w[45]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0xF40E3585, w[46]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0x106AA070, w[47]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0x19A4C116, w[48]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0x1E376C08, w[49]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0x2748774C, w[50]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0x34B0BCB5, w[51]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0x391C0CB3, w[52]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0x4ED8AA4A, w[53]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0x5B9CCA4F, w[54]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0x682E6FF3, w[55]);

	sha256_round(r, 0, 1, 2, 3, 4, 5, 6, 7, 0x748F82EE, w[56]);
	sha256_round(r, 7, 0, 1, 2, 3, 4, 5, 6, 0x78A5636F, w[57]);
	sha256_round(r, 6, 7, 0, 1, 2, 3, 4, 5, 0x84C87814, w[58]);
	sha256_round(r, 5, 6, 7, 0, 1, 2, 3, 4, 0x8CC70208, w[59]);
	sha256_round(r, 4, 5, 6, 7, 0, 1, 2, 3, 0x90BEFFFA, w[60]);
	sha256_round(r, 3, 4, 5, 6, 7, 0, 1, 2, 0xA4506CEB, w[61]);
	sha256_round(r, 2, 3, 4, 5, 6, 7, 0, 1, 0xBEF9A3F7, w[62]);
	sha256_round(r, 1, 2, 3, 4, 5, 6, 7, 0, 0xC67178F2, w[63]);

	for(unsigned i = 0; i < 8; ++i){
		regs[i] += r[i];
	}
}

void MCFBUILD_Sha256Initialize(MCFBUILD_Sha256Context *pContext){
	// Fill in hard-coded values.
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
	// If there are any data pending in the chunk, ...
	if(uRemaining >= uChunkAvail){
		// ... fill the chunk up, and eat it.
		if(pContext->uChunkOffset != 0){
			memcpy(pContext->au8Chunk + pContext->uChunkOffset, (const unsigned char *)pData + uSize - uRemaining, uChunkAvail);
			uRemaining -= uChunkAvail;
			pContext->uChunkOffset = 0;
			sha256_chunk(pContext->au32Regs, pContext->au8Chunk);
		}
		// Avoid copying data to the internal chunk over and over by reading from the input area directly.
		while(uRemaining >= 64){
			sha256_chunk(pContext->au32Regs, (const unsigned char *)pData + uSize - uRemaining);
			uRemaining -= 64;
		}
	}
	// The data remaining should fit into the internal chunk. Absorb them if any.
	if(uRemaining != 0){
		memcpy(pContext->au8Chunk + pContext->uChunkOffset, (const unsigned char *)pData + uSize - uRemaining, uRemaining);
		pContext->uChunkOffset += uRemaining;
	}
	// Add up the number of BITs.
	pContext->u64BitsTotal += (uint64_t)uSize * 8;
}
void MCFBUILD_Sha256Finalize(uint8_t (*restrict pau8Result)[32], MCFBUILD_Sha256Context *restrict pContext){
	// Append an `0x80` to the last chunk.
	// There is no need for overflow checks since the last chunk will never be full.
	pContext->au8Chunk[(pContext->uChunkOffset)++] = 0x80;
	size_t uChunkAvail = 64 - pContext->uChunkOffset;
	// Pad the last chunk until there are exactly 8 bytes remaining.
	// If there are fewer at this time, fill it up with zeroes, eat it, and prepare another chunk after it.
	if(uChunkAvail < 8){
		memset(pContext->au8Chunk + pContext->uChunkOffset, 0, uChunkAvail);
		sha256_chunk(pContext->au32Regs, pContext->au8Chunk);
		pContext->uChunkOffset = 0;
		uChunkAvail = 64;
	}
	// The last chunk should have no more than 56 bytes here. Pad it.
	if(uChunkAvail > 8){
		memset(pContext->au8Chunk + pContext->uChunkOffset, 0, uChunkAvail - 8);
		pContext->uChunkOffset = 56;
	}
	// Finish the last chunk.
	MCFBUILD_store_be_uint64((uint64_t *)pContext->au8Chunk + 7, pContext->u64BitsTotal);
	sha256_chunk(pContext->au32Regs, pContext->au8Chunk);
	// Copy the hash out.
	for(unsigned uIndex = 0; uIndex < 8; ++uIndex){
		MCFBUILD_store_be_uint32((uint32_t *)pau8Result + uIndex, pContext->au32Regs[uIndex]);
	}
}
