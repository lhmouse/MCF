// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SHA1.hpp"
#include <cstring>
using namespace MCF;

namespace {
	void DoSHA1Chunk(std::uint32_t (&auResult)[5], const unsigned char *pbyChunk){
		// https://en.wikipedia.org/wiki/SHA1
/*
		std::uint32_t w[80];

		for(std::size_t i = 0; i < 16; ++i){
			w[i] = __builtin_bswap32(((const std::uint32_t *)pbyChunk)[i]);
		}
		for(std::size_t i = 16; i < 32; ++i){
			w[i] = ::_rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		}
		for(std::size_t i = 32; i < COUNT_OF(w); ++i){
			w[i] = ::_rotl(w[i - 6] ^ w[i - 16] ^ w[i - 28] ^ w[i - 32], 2);
		}

		std::uint32_t a = auResult[0];
		std::uint32_t b = auResult[1];
		std::uint32_t c = auResult[2];
		std::uint32_t d = auResult[3];
		std::uint32_t e = auResult[4];

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

		auResult[0] += a;
		auResult[1] += b;
		auResult[2] += c;
		auResult[3] += d;
		auResult[4] += e;
*/

		__attribute__((aligned(16))) std::uint32_t w[80];

		for(std::size_t i = 0; i < 16; ++i){
			w[i] = __builtin_bswap32(((const std::uint32_t *)pbyChunk)[i]);
		}
		for(std::size_t i = 16; i < 32; ++i){
			w[i] = ::_rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		}
/*
		for(std::size_t i = 32; i < COUNT_OF(w); ++i){
			w[i] = ::_rotl(w[i - 6] ^ w[i - 16] ^ w[i - 28] ^ w[i - 32], 2);
		}
*/

		register std::uint32_t a __asm__("ax") = auResult[0];
		register std::uint32_t b __asm__("bx") = auResult[1];
		register std::uint32_t c __asm__("cx") = auResult[2];
		register std::uint32_t d __asm__("dx") = auResult[3];
		register std::uint32_t e __asm__("si") = auResult[4];

#ifdef _WIN64
#	define TMP			"r8d"
#	define SAVE_TMP
#	define RESTORE_TMP
#	define TMP_CLOB		, "r8"
#else
#	define TMP			"ebp"
#	define SAVE_TMP		"push ebp"
#	define RESTORE_TMP	"pop ebp"
#	define TMP_CLOB
#endif

		__asm__ __volatile__(
			"movdqa xmm4, xmmword ptr[%5 + 28 * 4] \n"
			"movdqa xmm3, xmmword ptr[%5 + 24 * 4] \n"
			"movdqa xmm1, xmmword ptr[%5 + 16 * 4] \n"
			"movdqa xmm6, xmmword ptr[%5] \n"
			"movdqa xmm2, xmmword ptr[%5 + 20 * 4] \n"

#define GEN_W_0(i, xr_out, xrm4, xrm8, xrm16, xrm28_out, xrm32)	\
			"movdqa " xr_out ", " xrm8 " \n"	\
			"movdqa " xrm28_out ", xmmword ptr[%5 + ((" #i ") - 7 * 4) * 4] \n"

#define GEN_W_1(i, xr_out, xrm4, xrm8, xrm16, xrm28_out, xrm32)	\
			"punpckhqdq " xr_out ", " xr_out " \n"	\
			"punpcklqdq " xr_out ", " xrm4 " \n"

#define GEN_W_2(i, xr_out, xrm4, xrm8, xrm16, xrm28_out, xrm32)	\
			"pxor " xr_out ", " xrm16 " \n"	\
			"pxor " xr_out ", " xrm28_out " \n"	\
			"pxor " xr_out ", " xrm32 " \n"

#define GEN_W_3(i, xr_out, xrm4, xrm8, xrm16, xrm28_out, xrm32)	\
			"movdqa xmm7, " xr_out " \n"	\
			"psrld xmm7, 30 \n"	\
			"pslld " xr_out ", 2 \n"

#define GEN_W_4(i, xr_out, xrm4, xrm8, xrm16, xrm28_out, xrm32)	\
			"por " xr_out ", xmm7 \n"	\
			"movdqa xmmword ptr[%5 + (" #i ") * 4], " xr_out " \n"

#define STEP_0(i, ra, rb, rc, rd, re)	\
			"mov edi, " ra " \n"	\
			"rol edi, 5 \n"	\
			"add edi, dword ptr[%5 + (" #i ") * 4] \n"	\
			"add edi, 0x5A827999 \n"	\
			"add " re ", edi \n"	\
			"mov edi, " rc " \n"	\
			"xor edi, " rd " \n"	\
			"and edi, " rb " \n"	\
			"xor edi, " rd " \n"	\
			"add " re ", edi \n"	\
			"rol " rb ", 30 \n"

#define STEP_1(i, ra, rb, rc, rd, re)	\
			"mov edi, " ra " \n"	\
			"rol edi, 5 \n"	\
			"add edi, dword ptr[%5 + (" #i ") * 4] \n"	\
			"add edi, 0x6ED9EBA1 \n"	\
			"add " re ", edi \n"	\
			"mov edi, " rb " \n"	\
			"xor edi, " rc " \n"	\
			"xor edi, " rd " \n"	\
			"add " re ", edi \n"	\
			"rol " rb ", 30 \n"

#define STEP_2(i, ra, rb, rc, rd, re)	\
			"mov edi, " ra " \n"	\
			"rol edi, 5 \n"	\
			"add edi, dword ptr[%5 + (" #i ") * 4] \n"	\
			"add edi, 0x8F1BBCDC \n"	\
			"add " re ", edi \n"	\
			"mov edi, " rc " \n"	\
			"or edi, " rd " \n"	\
			"and edi, " rb " \n"	\
			SAVE_TMP " \n"	\
			"mov " TMP ", " rc " \n"	\
			"and " TMP ", " rd " \n"	\
			"or edi, " TMP " \n"	\
			RESTORE_TMP " \n"	\
			"add " re ", edi \n"	\
			"rol " rb ", 30 \n"

#define STEP_3(i, ra, rb, rc, rd, re)	\
			"mov edi, " ra " \n"	\
			"rol edi, 5 \n"	\
			"add edi, dword ptr[%5 + (" #i ") * 4] \n"	\
			"add edi, 0xCA62C1D6 \n"	\
			"add " re ", edi \n"	\
			"mov edi, " rb " \n"	\
			"xor edi, " rc " \n"	\
			"xor edi, " rd " \n"	\
			"add " re ", edi \n"	\
			"rol " rb ", 30 \n"

			STEP_0(0 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_0(1 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_0(2 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_0(3 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_0(4 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(32, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_0(5 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_0(6 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_0(7 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_0(8 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_0(9 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(36, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_0(10 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
			STEP_0(11 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
			STEP_0(12 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
			STEP_0(13 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
			STEP_0(14 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(40, "xmm2", "xmm1", "xmm0", "xmm3", "xmm5", "xmm6")
			STEP_0(15 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
			STEP_0(16 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
			STEP_0(17 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
			STEP_0(18 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")
			STEP_0(19 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(44, "xmm3", "xmm2", "xmm1", "xmm4", "xmm6", "xmm5")

			STEP_1(20 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
			STEP_1(21 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
			STEP_1(22 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
			STEP_1(23 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
			STEP_1(24 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(48, "xmm4", "xmm3", "xmm2", "xmm0", "xmm5", "xmm6")
			STEP_1(25 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
			STEP_1(26 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
			STEP_1(27 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
			STEP_1(28 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
			STEP_1(29 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(52, "xmm0", "xmm4", "xmm3", "xmm1", "xmm6", "xmm5")
			STEP_1(30 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
			STEP_1(31 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
			STEP_1(32 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
			STEP_1(33 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
			STEP_1(34 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(56, "xmm1", "xmm0", "xmm4", "xmm2", "xmm5", "xmm6")
			STEP_1(35 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
			STEP_1(36 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
			STEP_1(37 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
			STEP_1(38 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")
			STEP_1(39 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(60, "xmm2", "xmm1", "xmm0", "xmm3", "xmm6", "xmm5")

			STEP_2(40 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
			STEP_2(41 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
			STEP_2(42 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
			STEP_2(43 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
			STEP_2(44 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(64, "xmm3", "xmm2", "xmm1", "xmm4", "xmm5", "xmm6")
			STEP_2(45 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
			STEP_2(46 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
			STEP_2(47 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
			STEP_2(48 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
			STEP_2(49 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(68, "xmm4", "xmm3", "xmm2", "xmm0", "xmm6", "xmm5")
			STEP_2(50 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_2(51 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_2(52 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_2(53 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_2(54 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(72, "xmm0", "xmm4", "xmm3", "xmm1", "xmm5", "xmm6")
			STEP_2(55 , "eax", "ebx", "ecx", "edx", "esi")	GEN_W_0(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_2(56 , "esi", "eax", "ebx", "ecx", "edx")	GEN_W_1(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_2(57 , "edx", "esi", "eax", "ebx", "ecx")	GEN_W_2(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_2(58 , "ecx", "edx", "esi", "eax", "ebx")	GEN_W_3(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")
			STEP_2(59 , "ebx", "ecx", "edx", "esi", "eax")	GEN_W_4(76, "xmm1", "xmm0", "xmm4", "xmm2", "xmm6", "xmm5")

			STEP_3(60 , "eax", "ebx", "ecx", "edx", "esi")
			STEP_3(61 , "esi", "eax", "ebx", "ecx", "edx")
			STEP_3(62 , "edx", "esi", "eax", "ebx", "ecx")
			STEP_3(63 , "ecx", "edx", "esi", "eax", "ebx")
			STEP_3(64 , "ebx", "ecx", "edx", "esi", "eax")
			STEP_3(65 , "eax", "ebx", "ecx", "edx", "esi")
			STEP_3(66 , "esi", "eax", "ebx", "ecx", "edx")
			STEP_3(67 , "edx", "esi", "eax", "ebx", "ecx")
			STEP_3(68 , "ecx", "edx", "esi", "eax", "ebx")
			STEP_3(69 , "ebx", "ecx", "edx", "esi", "eax")
			STEP_3(70 , "eax", "ebx", "ecx", "edx", "esi")
			STEP_3(71 , "esi", "eax", "ebx", "ecx", "edx")
			STEP_3(72 , "edx", "esi", "eax", "ebx", "ecx")
			STEP_3(73 , "ecx", "edx", "esi", "eax", "ebx")
			STEP_3(74 , "ebx", "ecx", "edx", "esi", "eax")
			STEP_3(75 , "eax", "ebx", "ecx", "edx", "esi")
			STEP_3(76 , "esi", "eax", "ebx", "ecx", "edx")
			STEP_3(77 , "edx", "esi", "eax", "ebx", "ecx")
			STEP_3(78 , "ecx", "edx", "esi", "eax", "ebx")
			STEP_3(79 , "ebx", "ecx", "edx", "esi", "eax")

			: "=a"(a), "=b"(b), "=c"(c), "=d"(d), "=S"(e)
			: "m"(w), "0"(a), "1"(b), "2"(c), "3"(d), "4"(e)
			: "di" TMP_CLOB
		);

		auResult[0] += a;
		auResult[1] += b;
		auResult[2] += c;
		auResult[3] += d;
		auResult[4] += e;
	}
}

// 构造函数和析构函数。
SHA1::SHA1() noexcept {
	xm_bInited = false;
}

// 其他非静态成员函数。
void SHA1::Update(const void *pData, std::size_t uSize) noexcept {
	if(!xm_bInited){
		xm_bInited = true;

		xm_auResult[0] = 0x67452301u;
		xm_auResult[1] = 0xEFCDAB89u;
		xm_auResult[2] = 0x98BADCFEu;
		xm_auResult[3] = 0x10325476u,
		xm_auResult[4] = 0xC3D2E1F0u;

		xm_uBytesInChunk = 0;
		xm_uBytesTotal = 0;
	}

	auto pbyRead = (const unsigned char *)pData;
	std::size_t uBytesRemaining = uSize;
	const std::size_t uBytesFree = sizeof(xm_abyChunk) - xm_uBytesInChunk;
	if(uBytesRemaining >= uBytesFree){
		if(xm_uBytesInChunk != 0){
			std::memcpy(xm_abyChunk + xm_uBytesInChunk, pbyRead, uBytesFree);
			DoSHA1Chunk(xm_auResult, xm_abyChunk);
			xm_uBytesInChunk = 0;
			pbyRead += uBytesFree;
			uBytesRemaining -= uBytesFree;
		}
		while(uBytesRemaining >= sizeof(xm_abyChunk)){
			DoSHA1Chunk(xm_auResult, pbyRead);
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
void SHA1::Finalize(unsigned char (&abyOutput)[20]) noexcept {
	if(xm_bInited){
		xm_bInited = false;

		xm_abyChunk[xm_uBytesInChunk++] = 0x80;
		if(xm_uBytesInChunk > sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyChunk) - xm_uBytesInChunk);
			DoSHA1Chunk(xm_auResult, xm_abyChunk);
			xm_uBytesInChunk = 0;
		}
		if(xm_uBytesInChunk < sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyFirstPart) - xm_uBytesInChunk);
		}
		xm_uBitsTotal = __builtin_bswap64(xm_uBytesTotal * CHAR_BIT);
		DoSHA1Chunk(xm_auResult, xm_abyChunk);

		for(auto &u : xm_auResult){
			u = __builtin_bswap32(u);
		}
	}
	__builtin_memcpy(abyOutput, xm_auResult, sizeof(xm_auResult));
}
