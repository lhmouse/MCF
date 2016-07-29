// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "Sha1OutputStream.hpp"
#include "../Core/Array.hpp"
#include "../Utilities/Endian.hpp"

namespace MCF {

// https://en.wikipedia.org/wiki/SHA-1

Sha1OutputStream::~Sha1OutputStream(){
}

void Sha1OutputStream::X_Initialize() noexcept {
	x_au32Reg       = { 0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u };
	x_u64BytesTotal = 0;
}
void Sha1OutputStream::X_Update(const std::uint8_t (&abyChunk)[64]) noexcept {
/*
	std::uint32_t w[80];

	for(std::size_t i = 0; i < 16; ++i){
		w[i] = LoadBe(((const std::uint32_t *)abyChunk)[i]);
	}
	for(std::size_t i = 16; i < 32; ++i){
		w[i] = ::_rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	}
	for(std::size_t i = 32; i < CountOf(w); ++i){
		w[i] = ::_rotl(w[i - 6] ^ w[i - 16] ^ w[i - 28] ^ w[i - 32], 2);
	}

	std::uint32_t a = x_au32Reg[0];
	std::uint32_t b = x_au32Reg[1];
	std::uint32_t c = x_au32Reg[2];
	std::uint32_t d = x_au32Reg[3];
	std::uint32_t e = x_au32Reg[4];

	for(std::size_t i = 0; i < 80; ++i){
		std::uint32_t f, k;

		switch(i / 20){
		case 0:
			// f = (b & c) | (~b & d);
			f = d ^ (b & (c ^ d));
			k = 0x5A827999;
			break;

		case 1:
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;
			break;

		case 2:
			// f = (b & c) | (b & d) | (c & d);
			f = (b & (c | d)) | (c & d);
			k = 0x8F1BBCDC;
			break;

		case 3:
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
			break;
		}

		const std::uint32_t temp = ::_rotl(a, 5) + f + e + k + w[i];
		e = d;
		d = c;
		c = ::_rotl(b, 30);
		b = a;
		a = temp;
	}

	x_au32Reg[0] += a;
	x_au32Reg[1] += b;
	x_au32Reg[2] += c;
	x_au32Reg[3] += d;
	x_au32Reg[4] += e;
*/

	alignas(16) std::uint32_t w[80];

	const auto pu32Words = reinterpret_cast<const std::uint32_t *>(abyChunk);
	for(std::size_t i = 0; i < 16; ++i){
		w[i] = LoadBe(pu32Words[i]);
	}
	for(std::size_t i = 16; i < 32; ++i){
		w[i] = ::_rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	}
/*
	for(std::size_t i = 32; i < CountOf(w); ++i){
		w[i] = ::_rotl(w[i - 6] ^ w[i - 16] ^ w[i - 28] ^ w[i - 32], 2);
	}
*/

	auto a = x_au32Reg[0];
	auto b = x_au32Reg[1];
	auto c = x_au32Reg[2];
	auto d = x_au32Reg[3];
	auto e = x_au32Reg[4];

	__asm__ volatile (
		"movdqa xmm4, xmmword ptr[%5 + 28 * 4] \n"
		"movdqa xmm3, xmmword ptr[%5 + 24 * 4] \n"
		"movdqa xmm1, xmmword ptr[%5 + 16 * 4] \n"
		"movdqa xmm6, xmmword ptr[%5] \n"
		"movdqa xmm2, xmmword ptr[%5 + 20 * 4] \n"

#define GEN_W_0(i_, xr_out_, xrm4_, xrm8_, xrm16_, xrm28_out_, xrm32_)	\
		"movdqa " xr_out_ ", " xrm8_ " \n"	\
		"movdqa " xrm28_out_ ", xmmword ptr[%5 + ((" #i_ ") - 7 * 4) * 4] \n"

#define GEN_W_1(i_, xr_out_, xrm4_, xrm8_, xrm16_, xrm28_out_, xrm32_)	\
		"punpckhqdq " xr_out_ ", " xr_out_ " \n"	\
		"punpcklqdq " xr_out_ ", " xrm4_ " \n"

#define GEN_W_2(i_, xr_out_, xrm4_, xrm8_, xrm16_, xrm28_out_, xrm32_)	\
		"pxor " xr_out_ ", " xrm16_ " \n"	\
		"pxor " xr_out_ ", " xrm28_out_ " \n"	\
		"pxor " xr_out_ ", " xrm32_ " \n"

#define GEN_W_3(i_, xr_out_, xrm4_, xrm8_, xrm16_, xrm28_out_, xrm32_)	\
		"movdqa xmm7, " xr_out_ " \n"	\
		"psrld xmm7, 30 \n"	\
		"pslld " xr_out_ ", 2 \n"

#define GEN_W_4(i_, xr_out_, xrm4_, xrm8_, xrm16_, xrm28_out_, xrm32_)	\
		"por " xr_out_ ", xmm7 \n"	\
		"movdqa xmmword ptr[%5 + (" #i_ ") * 4], " xr_out_ " \n"

#define STEP_0(i_, ra_, rb_, rc_, rd_, re_)	\
		"mov edi, " ra_ " \n"	\
		"rol edi, 5 \n"	\
		"add edi, dword ptr[%5 + (" #i_ ") * 4] \n"	\
		"add edi, 0x5A827999 \n"	\
		"add " re_ ", edi \n"	\
		"mov edi, " rc_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"and edi, " rb_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"add " re_ ", edi \n"	\
		"rol " rb_ ", 30 \n"

#define STEP_1(i_, ra_, rb_, rc_, rd_, re_)	\
		"mov edi, " ra_ " \n"	\
		"rol edi, 5 \n"	\
		"add edi, dword ptr[%5 + (" #i_ ") * 4] \n"	\
		"add edi, 0x6ED9EBA1 \n"	\
		"add " re_ ", edi \n"	\
		"mov edi, " rb_ " \n"	\
		"xor edi, " rc_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"add " re_ ", edi \n"	\
		"rol " rb_ ", 30 \n"

#ifdef _WIN64
#	define EDI_OREQ_RC_AND_RD(rc_, rd_)	\
		"mov r8d, " rc_ " \n"	\
		"and r8d, " rd_ " \n"	\
		"or edi, r8d \n"
#else
#	define EDI_OREQ_RC_AND_RD(rc_, rd_)	\
		"push " rc_ " \n"	\
		"and " rc_ ", " rd_ " \n"	\
		"or edi, " rc_ " \n"	\
		"pop " rc_ " \n"
#endif

#define STEP_2(i_, ra_, rb_, rc_, rd_, re_)	\
		"mov edi, " ra_ " \n"	\
		"rol edi, 5 \n"	\
		"add edi, dword ptr[%5 + (" #i_ ") * 4] \n"	\
		"add edi, 0x8F1BBCDC \n"	\
		"add " re_ ", edi \n"	\
		"mov edi, " rc_ " \n"	\
		"or edi, " rd_ " \n"	\
		"and edi, " rb_ " \n"	\
		EDI_OREQ_RC_AND_RD(rc_, rd_)	\
		"add " re_ ", edi \n"	\
		"rol " rb_ ", 30 \n"

#define STEP_3(i_, ra_, rb_, rc_, rd_, re_)	\
		"mov edi, " ra_ " \n"	\
		"rol edi, 5 \n"	\
		"add edi, dword ptr[%5 + (" #i_ ") * 4] \n"	\
		"add edi, 0xCA62C1D6 \n"	\
		"add " re_ ", edi \n"	\
		"mov edi, " rb_ " \n"	\
		"xor edi, " rc_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"add " re_ ", edi \n"	\
		"rol " rb_ ", 30 \n"

		STEP_0( 0, "%0", "%1", "%2", "%3", "%4") GEN_W_0(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_0( 1, "%4", "%0", "%1", "%2", "%3") GEN_W_1(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_0( 2, "%3", "%4", "%0", "%1", "%2") GEN_W_2(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_0( 3, "%2", "%3", "%4", "%0", "%1") GEN_W_3(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_0( 4, "%1", "%2", "%3", "%4", "%0") GEN_W_4(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_0( 5, "%0", "%1", "%2", "%3", "%4") GEN_W_0(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_0( 6, "%4", "%0", "%1", "%2", "%3") GEN_W_1(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_0( 7, "%3", "%4", "%0", "%1", "%2") GEN_W_2(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_0( 8, "%2", "%3", "%4", "%0", "%1") GEN_W_3(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_0( 9, "%1", "%2", "%3", "%4", "%0") GEN_W_4(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_0(10, "%0", "%1", "%2", "%3", "%4") GEN_W_0(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
		STEP_0(11, "%4", "%0", "%1", "%2", "%3") GEN_W_1(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
		STEP_0(12, "%3", "%4", "%0", "%1", "%2") GEN_W_2(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
		STEP_0(13, "%2", "%3", "%4", "%0", "%1") GEN_W_3(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
		STEP_0(14, "%1", "%2", "%3", "%4", "%0") GEN_W_4(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
		STEP_0(15, "%0", "%1", "%2", "%3", "%4") GEN_W_0(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
		STEP_0(16, "%4", "%0", "%1", "%2", "%3") GEN_W_1(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
		STEP_0(17, "%3", "%4", "%0", "%1", "%2") GEN_W_2(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
		STEP_0(18, "%2", "%3", "%4", "%0", "%1") GEN_W_3(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
		STEP_0(19, "%1", "%2", "%3", "%4", "%0") GEN_W_4(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")

		STEP_1(20, "%0", "%1", "%2", "%3", "%4") GEN_W_0(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
		STEP_1(21, "%4", "%0", "%1", "%2", "%3") GEN_W_1(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
		STEP_1(22, "%3", "%4", "%0", "%1", "%2") GEN_W_2(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
		STEP_1(23, "%2", "%3", "%4", "%0", "%1") GEN_W_3(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
		STEP_1(24, "%1", "%2", "%3", "%4", "%0") GEN_W_4(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
		STEP_1(25, "%0", "%1", "%2", "%3", "%4") GEN_W_0(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
		STEP_1(26, "%4", "%0", "%1", "%2", "%3") GEN_W_1(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
		STEP_1(27, "%3", "%4", "%0", "%1", "%2") GEN_W_2(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
		STEP_1(28, "%2", "%3", "%4", "%0", "%1") GEN_W_3(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
		STEP_1(29, "%1", "%2", "%3", "%4", "%0") GEN_W_4(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
		STEP_1(30, "%0", "%1", "%2", "%3", "%4") GEN_W_0(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
		STEP_1(31, "%4", "%0", "%1", "%2", "%3") GEN_W_1(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
		STEP_1(32, "%3", "%4", "%0", "%1", "%2") GEN_W_2(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
		STEP_1(33, "%2", "%3", "%4", "%0", "%1") GEN_W_3(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
		STEP_1(34, "%1", "%2", "%3", "%4", "%0") GEN_W_4(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
		STEP_1(35, "%0", "%1", "%2", "%3", "%4") GEN_W_0(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
		STEP_1(36, "%4", "%0", "%1", "%2", "%3") GEN_W_1(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
		STEP_1(37, "%3", "%4", "%0", "%1", "%2") GEN_W_2(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
		STEP_1(38, "%2", "%3", "%4", "%0", "%1") GEN_W_3(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
		STEP_1(39, "%1", "%2", "%3", "%4", "%0") GEN_W_4(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")

		STEP_2(40, "%0", "%1", "%2", "%3", "%4") GEN_W_0(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
		STEP_2(41, "%4", "%0", "%1", "%2", "%3") GEN_W_1(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
		STEP_2(42, "%3", "%4", "%0", "%1", "%2") GEN_W_2(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
		STEP_2(43, "%2", "%3", "%4", "%0", "%1") GEN_W_3(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
		STEP_2(44, "%1", "%2", "%3", "%4", "%0") GEN_W_4(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
		STEP_2(45, "%0", "%1", "%2", "%3", "%4") GEN_W_0(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
		STEP_2(46, "%4", "%0", "%1", "%2", "%3") GEN_W_1(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
		STEP_2(47, "%3", "%4", "%0", "%1", "%2") GEN_W_2(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
		STEP_2(48, "%2", "%3", "%4", "%0", "%1") GEN_W_3(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
		STEP_2(49, "%1", "%2", "%3", "%4", "%0") GEN_W_4(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
		STEP_2(50, "%0", "%1", "%2", "%3", "%4") GEN_W_0(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_2(51, "%4", "%0", "%1", "%2", "%3") GEN_W_1(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_2(52, "%3", "%4", "%0", "%1", "%2") GEN_W_2(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_2(53, "%2", "%3", "%4", "%0", "%1") GEN_W_3(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_2(54, "%1", "%2", "%3", "%4", "%0") GEN_W_4(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
		STEP_2(55, "%0", "%1", "%2", "%3", "%4") GEN_W_0(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_2(56, "%4", "%0", "%1", "%2", "%3") GEN_W_1(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_2(57, "%3", "%4", "%0", "%1", "%2") GEN_W_2(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_2(58, "%2", "%3", "%4", "%0", "%1") GEN_W_3(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
		STEP_2(59, "%1", "%2", "%3", "%4", "%0") GEN_W_4(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")

		STEP_3(60, "%0", "%1", "%2", "%3", "%4")
		STEP_3(61, "%4", "%0", "%1", "%2", "%3")
		STEP_3(62, "%3", "%4", "%0", "%1", "%2")
		STEP_3(63, "%2", "%3", "%4", "%0", "%1")
		STEP_3(64, "%1", "%2", "%3", "%4", "%0")
		STEP_3(65, "%0", "%1", "%2", "%3", "%4")
		STEP_3(66, "%4", "%0", "%1", "%2", "%3")
		STEP_3(67, "%3", "%4", "%0", "%1", "%2")
		STEP_3(68, "%2", "%3", "%4", "%0", "%1")
		STEP_3(69, "%1", "%2", "%3", "%4", "%0")
		STEP_3(70, "%0", "%1", "%2", "%3", "%4")
		STEP_3(71, "%4", "%0", "%1", "%2", "%3")
		STEP_3(72, "%3", "%4", "%0", "%1", "%2")
		STEP_3(73, "%2", "%3", "%4", "%0", "%1")
		STEP_3(74, "%1", "%2", "%3", "%4", "%0")
		STEP_3(75, "%0", "%1", "%2", "%3", "%4")
		STEP_3(76, "%4", "%0", "%1", "%2", "%3")
		STEP_3(77, "%3", "%4", "%0", "%1", "%2")
		STEP_3(78, "%2", "%3", "%4", "%0", "%1")
		STEP_3(79, "%1", "%2", "%3", "%4", "%0")

		: "=r"(a), "=r"(b), "=r"(c), "=r"(d), "=r"(e), "=m"(w)
		: "0"(a), "1"(b), "2"(c), "3"(d), "4"(e), "m"(w)
		: "di"
#ifdef _WIN64
			, "r8", "xmm6", "xmm7"
#endif
	);

	x_au32Reg[0] += a;
	x_au32Reg[1] += b;
	x_au32Reg[2] += c;
	x_au32Reg[3] += d;
	x_au32Reg[4] += e;
}
void Sha1OutputStream::X_Finalize(std::uint8_t (&abyChunk)[64], unsigned uBytesInChunk) noexcept {
	abyChunk[uBytesInChunk] = 0x80;
	++uBytesInChunk;
	if(uBytesInChunk > 56){
		std::memset(abyChunk + uBytesInChunk, 0, 64 - uBytesInChunk);
		X_Update(abyChunk);
		uBytesInChunk = 0;
	}
	if(uBytesInChunk < 56){
		std::memset(abyChunk + uBytesInChunk, 0, 56 - uBytesInChunk);
	}
	std::uint64_t u64BitsTotal;
	StoreBe(u64BitsTotal, x_u64BytesTotal * 8);
	std::memcpy(abyChunk + 56, &u64BitsTotal, 8);
	X_Update(abyChunk);
}

void Sha1OutputStream::Put(unsigned char byData){
	Put(&byData, 1);
}
void Sha1OutputStream::Put(const void *pData, std::size_t uSize){
	if(x_nChunkOffset < 0){
		X_Initialize();
		x_nChunkOffset = 0;
	}

	auto pbyRead = static_cast<const unsigned char *>(pData);
	auto uBytesRemaining = uSize;
	const auto uChunkAvail = sizeof(x_abyChunk) - static_cast<unsigned>(x_nChunkOffset);
	if(uBytesRemaining >= uChunkAvail){
		if(x_nChunkOffset != 0){
			std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uChunkAvail);
			pbyRead += uChunkAvail;
			uBytesRemaining -= uChunkAvail;
			X_Update(x_abyChunk);
			x_nChunkOffset = 0;
		}
		while(uBytesRemaining >= sizeof(x_abyChunk)){
			X_Update(reinterpret_cast<const decltype(x_abyChunk) *>(pbyRead)[0]);
			pbyRead += sizeof(x_abyChunk);
			uBytesRemaining -= sizeof(x_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uBytesRemaining);
		x_nChunkOffset += static_cast<int>(uBytesRemaining);
	}
	x_u64BytesTotal += uSize;
}
void Sha1OutputStream::Flush(bool bHard){
	(void)bHard;
}

void Sha1OutputStream::Reset() noexcept {
	x_nChunkOffset = -1;
}
Array<std::uint8_t, 20> Sha1OutputStream::Finalize() noexcept {
	if(x_nChunkOffset >= 0){
		X_Finalize(x_abyChunk, static_cast<unsigned>(x_nChunkOffset));
	} else {
		X_Initialize();
		X_Finalize(x_abyChunk, 0);
	}
	x_nChunkOffset = -1;

	Array<std::uint8_t, 20> abyRet;
	const auto pu32RetWords = reinterpret_cast<std::uint32_t *>(abyRet.GetData());
	for(unsigned i = 0; i < 5; ++i){
		StoreBe(pu32RetWords[i], x_au32Reg[i]);
	}
	return abyRet;
}

}
