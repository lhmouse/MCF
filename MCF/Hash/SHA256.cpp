// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SHA256.hpp"
#include <cstring>
using namespace MCF;

namespace {
	void DoSHA256Chunk(std::uint32_t (&auResult)[8], const unsigned char *pbyChunk){
		// https://en.wikipedia.org/wiki/SHA-2
		// http://download.intel.com/embedded/processor/whitepaper/327457.pdf

		static const std::uint32_t KVEC[64] = {
			0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
			0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
			0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
			0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
			0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
			0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
			0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
			0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
		};

		std::uint32_t w[64];

		for(std::size_t i = 0; i < 16; ++i){
			// 先将 chunk 按照大端格式转换成 std::uint32_t。
			w[i] = __builtin_bswap32(((const std::uint32_t *)pbyChunk)[i]);
		}
		for(std::size_t i = 16; i < COUNT_OF(w); ++i){
			//const std::uint32_t s0 = ::_rotr(w[i - 15], 7) ^ ::_rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
			const std::uint32_t s0 = ::_rotr((::_rotr(w[i - 15], 11) ^ w[i - 15]), 7) ^ (w[i - 15] >> 3);
			//const std::uint32_t s1 = ::_rotr(w[i - 2], 17) ^ ::_rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
			const std::uint32_t s1 = ::_rotr((::_rotr(w[i - 2], 2) ^ w[i - 2]), 17) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + w[i - 7] + s0 + s1;
		}

		std::uint32_t tmp[8];
		__builtin_memcpy(tmp, auResult, sizeof(tmp));

		for(std::size_t i = 0; i < 64; ++i){
			//const std::uint32_t s0 = ::_rotr(tmp[0], 2) ^ ::_rotr(tmp[0], 13) ^ ::_rotr(tmp[0], 22);
			const std::uint32_t s0 = ::_rotr(::_rotr(::_rotr(tmp[0], 9) ^ tmp[0], 11) ^ tmp[0], 2);
			//const std::uint32_t maj = (tmp[0] & tmp[1]) ^ (tmp[0] & tmp[2]) ^ (tmp[1] & tmp[2]);
			const std::uint32_t maj = (tmp[0] & tmp[1]) | (tmp[2] & (tmp[0] ^ tmp[1]));
			const std::uint32_t t2 = s0 + maj;
			//const std::uint32_t s1 = ::_rotr(tmp[4], 6) ^ ::_rotr(tmp[4], 11) ^ ::_rotr(tmp[4], 25);
			const std::uint32_t s1 = ::_rotr(::_rotr(::_rotr(tmp[4], 14) ^ tmp[4], 5) ^ tmp[4], 6);
			//const std::uint32_t ch = (tmp[4] & tmp[5]) ^ (~tmp[4] & tmp[6]);
			const std::uint32_t ch = tmp[6] ^ (tmp[4] & (tmp[5] ^ tmp[6]));
			const std::uint32_t t1 = tmp[7] + s1 + ch + KVEC[i] + w[i];

			tmp[7] = tmp[6];
			tmp[6] = tmp[5];
			tmp[5] = tmp[4];
			tmp[4] = tmp[3] + t1;
			tmp[3] = tmp[2];
			tmp[2] = tmp[1];
			tmp[1] = tmp[0];
			tmp[0] = t1 + t2;
		}

		for(std::size_t i = 0; i < COUNT_OF(auResult); ++i){
			auResult[i] += tmp[i];
		}
	}
}

// 构造函数和析构函数。
SHA256::SHA256() noexcept {
	xm_bInited = false;
}

// 其他非静态成员函数。
void SHA256::Update(const void *pData, std::size_t uSize) noexcept {
	if(!xm_bInited){
		xm_bInited = true;

		xm_auResult[0] = 0x6A09E667u;
		xm_auResult[1] = 0xBB67AE85u;
		xm_auResult[2] = 0x3C6EF372u;
		xm_auResult[3] = 0xA54FF53Au;
		xm_auResult[4] = 0x510E527Fu;
		xm_auResult[5] = 0x9B05688Cu;
		xm_auResult[6] = 0x1F83D9ABu;
		xm_auResult[7] = 0x5BE0CD19u;

		xm_uBytesInChunk = 0;
		xm_uBytesTotal = 0;
	}

	auto pbyRead = (const unsigned char *)pData;
	std::size_t uBytesRemaining = uSize;
	const std::size_t uBytesFree = sizeof(xm_abyChunk) - xm_uBytesInChunk;
	if(uBytesRemaining >= uBytesFree){
		if(xm_uBytesInChunk != 0){
			std::memcpy(xm_abyChunk + xm_uBytesInChunk, pbyRead, uBytesFree);
			DoSHA256Chunk(xm_auResult, xm_abyChunk);
			xm_uBytesInChunk = 0;
			pbyRead += uBytesFree;
			uBytesRemaining -= uBytesFree;
		}
		while(uBytesRemaining >= sizeof(xm_abyChunk)){
			DoSHA256Chunk(xm_auResult, pbyRead);
			pbyRead += sizeof(xm_abyChunk);
			uBytesRemaining -= sizeof(xm_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(xm_abyChunk + xm_uBytesInChunk, pbyRead, uBytesRemaining);
		xm_uBytesInChunk += uBytesRemaining;
	}
	xm_uBytesTotal += uSize;
}
void SHA256::Finalize(unsigned char (&abyOutput)[32]) noexcept {
	if(xm_bInited){
		xm_bInited = false;

		xm_abyChunk[xm_uBytesInChunk++] = 0x80;
		if(xm_uBytesInChunk > sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyChunk) - xm_uBytesInChunk);
			DoSHA256Chunk(xm_auResult, xm_abyChunk);
			xm_uBytesInChunk = 0;
		}
		if(xm_uBytesInChunk < sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyFirstPart) - xm_uBytesInChunk);
		}
		xm_uBitsTotal = __builtin_bswap64(xm_uBytesTotal * CHAR_BIT);
		DoSHA256Chunk(xm_auResult, xm_abyChunk);

		for(auto &u : xm_auResult){
			u = __builtin_bswap32(u);
		}
	}
	__builtin_memcpy(abyOutput, xm_auResult, sizeof(xm_auResult));
}
