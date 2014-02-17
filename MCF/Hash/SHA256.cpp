// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SHA256.hpp"
#include <cstring>
using namespace MCF;

namespace {
	void DoSHA256Chunk(std::uint32_t (&auResult)[8], const unsigned char *pbyChunk) noexcept {
		// https://en.wikipedia.org/wiki/SHA-2
		// http://download.intel.com/embedded/processor/whitepaper/327457.pdf
/*
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
			w[i] = __builtin_bswap32(((const std::uint32_t *)pbyChunk)[i]);
		}
		for(std::size_t i = 16; i < COUNT_OF(w); ++i){
			//const std::uint32_t s0 = ::_rotr(w[i - 15], 7) ^ ::_rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
			const std::uint32_t s0 = ::_rotr((::_rotr(w[i - 15], 11) ^ w[i - 15]), 7) ^ (w[i - 15] >> 3);
			//const std::uint32_t s1 = ::_rotr(w[i - 2], 17) ^ ::_rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
			const std::uint32_t s1 = ::_rotr((::_rotr(w[i - 2], 2) ^ w[i - 2]), 17) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + w[i - 7] + s0 + s1;
		}

		std::uint32_t a = auResult[0];
		std::uint32_t b = auResult[1];
		std::uint32_t c = auResult[2];
		std::uint32_t d = auResult[3];
		std::uint32_t e = auResult[4];
		std::uint32_t f = auResult[5];
		std::uint32_t g = auResult[6];
		std::uint32_t h = auResult[7];

		for(std::size_t i = 0; i < 64; ++i){
			//const std::uint32_t S0 = ::_rotr(a, 2) ^ ::_rotr(a, 13) ^ ::_rotr(a, 22);
			const std::uint32_t S0 = ::_rotr(::_rotr(::_rotr(a, 9) ^ a, 11) ^ a, 2);
			//const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
			const std::uint32_t maj = (a & b) | (c & (a ^ b));
			const std::uint32_t t2 = S0 + maj;
			//const std::uint32_t S1 = ::_rotr(e, 6) ^ ::_rotr(e, 11) ^ ::_rotr(e, 25);
			const std::uint32_t S1 = ::_rotr(::_rotr(::_rotr(e, 14) ^ e, 5) ^ e, 6);
			//const std::uint32_t ch = (e & f) ^ (~e & g);
			const std::uint32_t ch = g ^ (e & (f ^ g));
			const std::uint32_t t1 = h + S1 + ch + KVEC[i] + w[i];

			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		auResult[0] += a;
		auResult[1] += b;
		auResult[2] += c;
		auResult[3] += d;
		auResult[4] += e;
		auResult[5] += f;
		auResult[6] += g;
		auResult[7] += h;
*/

#define K_0		"0x428A2F98"
#define K_1		"0x71374491"
#define K_2		"0xB5C0FBCF"
#define K_3		"0xE9B5DBA5"
#define K_4		"0x3956C25B"
#define K_5		"0x59F111F1"
#define K_6		"0x923F82A4"
#define K_7		"0xAB1C5ED5"
#define K_8		"0xD807AA98"
#define K_9		"0x12835B01"
#define K_10	"0x243185BE"
#define K_11	"0x550C7DC3"
#define K_12	"0x72BE5D74"
#define K_13	"0x80DEB1FE"
#define K_14	"0x9BDC06A7"
#define K_15	"0xC19BF174"
#define K_16	"0xE49B69C1"
#define K_17	"0xEFBE4786"
#define K_18	"0x0FC19DC6"
#define K_19	"0x240CA1CC"
#define K_20	"0x2DE92C6F"
#define K_21	"0x4A7484AA"
#define K_22	"0x5CB0A9DC"
#define K_23	"0x76F988DA"
#define K_24	"0x983E5152"
#define K_25	"0xA831C66D"
#define K_26	"0xB00327C8"
#define K_27	"0xBF597FC7"
#define K_28	"0xC6E00BF3"
#define K_29	"0xD5A79147"
#define K_30	"0x06CA6351"
#define K_31	"0x14292967"
#define K_32	"0x27B70A85"
#define K_33	"0x2E1B2138"
#define K_34	"0x4D2C6DFC"
#define K_35	"0x53380D13"
#define K_36	"0x650A7354"
#define K_37	"0x766A0ABB"
#define K_38	"0x81C2C92E"
#define K_39	"0x92722C85"
#define K_40	"0xA2BFE8A1"
#define K_41	"0xA81A664B"
#define K_42	"0xC24B8B70"
#define K_43	"0xC76C51A3"
#define K_44	"0xD192E819"
#define K_45	"0xD6990624"
#define K_46	"0xF40E3585"
#define K_47	"0x106AA070"
#define K_48	"0x19A4C116"
#define K_49	"0x1E376C08"
#define K_50	"0x2748774C"
#define K_51	"0x34B0BCB5"
#define K_52	"0x391C0CB3"
#define K_53	"0x4ED8AA4A"
#define K_54	"0x5B9CCA4F"
#define K_55	"0x682E6FF3"
#define K_56	"0x748F82EE"
#define K_57	"0x78A5636F"
#define K_58	"0x84C87814"
#define K_59	"0x8CC70208"
#define K_60	"0x90BEFFFA"
#define K_61	"0xA4506CEB"
#define K_62	"0xBEF9A3F7"
#define K_63	"0xC67178F2"
#define K(i)	K_##i

		alignas(16) std::uint32_t w[64];
		for(std::size_t i = 0; i < 16; ++i){
			w[i] = __builtin_bswap32(((const std::uint32_t *)pbyChunk)[i]);
		}
/*
		for(std::size_t i = 16; i < COUNT_OF(w); ++i){
			//const std::uint32_t s0 = ::_rotr(w[i - 15], 7) ^ ::_rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
			const std::uint32_t s0 = ::_rotr((::_rotr(w[i - 15], 11) ^ w[i - 15]), 7) ^ (w[i - 15] >> 3);
			//const std::uint32_t s1 = ::_rotr(w[i - 2], 17) ^ ::_rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
			const std::uint32_t s1 = ::_rotr((::_rotr(w[i - 2], 2) ^ w[i - 2]), 17) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + w[i - 7] + s0 + s1;
		}
*/

#ifdef _WIN64
		register std::uint32_t a = auResult[0];
		register std::uint32_t b = auResult[1];
		register std::uint32_t c = auResult[2];
		register std::uint32_t d = auResult[3];
		register std::uint32_t e = auResult[4];
		register std::uint32_t f = auResult[5];
		register std::uint32_t g = auResult[6];
		register std::uint32_t h = auResult[7];
#else
		std::uint32_t a = auResult[0];
		std::uint32_t b = auResult[1];
		std::uint32_t c = auResult[2];
		std::uint32_t d = auResult[3];
		std::uint32_t e = auResult[4];
		std::uint32_t f = auResult[5];
		std::uint32_t g = auResult[6];
		std::uint32_t h = auResult[7];
#endif

		__asm__ __volatile__(
			"movdqa xmm1, xmmword ptr[%8 + 12 * 4] \n"
			"movdqa xmm2, xmmword ptr[%8 + 8 * 4] \n"
			"movdqa xmm3, xmmword ptr[%8 + 4 * 4] \n"
			"movdqa xmm4, xmmword ptr[%8] \n"

#define GEN_W_0(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"movdqa " xr_out ", " xrm16 " \n"	\
			"psrldq " xr_out ", 4 \n"	\
			"movdqa xmm5, " xrm12 " \n"	\
			"pslldq xmm5, 12 \n"

#define GEN_W_1(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"por " xr_out ", xmm5 \n"	\
			"movdqa xmm5, " xr_out " \n"	\
			"movdqa xmm6, " xr_out " \n"	\
			"psrld xmm6, 11 \n"

#define GEN_W_2(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"pslld " xr_out ", 21 \n"	\
			"por " xr_out ", xmm6 \n"	\
			"pxor " xr_out ", xmm5 \n"	\
			"psrld xmm5, 3 \n"

#define GEN_W_3(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"movdqa xmm6, " xr_out " \n"	\
			"psrld xmm6, 7 \n"	\
			"pslld " xr_out ", 25 \n"	\
			"por " xr_out ", xmm6 \n"

#define GEN_W_4(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"pxor " xr_out ", xmm5 \n"	\
			"movdqa xmm5, " xrm8 " \n"	\
			"psrldq xmm5, 4 \n"	\
			"movdqa xmm6, " xrm4 " \n"

#define GEN_W_5(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"pslldq xmm6, 12 \n"	\
			"por xmm5, xmm6 \n"	\
			"paddd " xr_out ", xmm5 \n"	\
			"paddd " xr_out ", " xrm16 " \n"

#define GEN_W_6(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"movdqa xmm5, " xrm4 " \n"	\
			"psrldq xmm5, 8 \n"	\
			"movdqa xmm6, xmm5 \n"

#define GEN_W_7(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"movdqa xmm7, xmm5 \n"	\
			"psrld xmm6, 2 \n"	\
			"pslld xmm5, 30 \n"

#define GEN_W_8(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"por xmm5, xmm6 \n"	\
			"pxor xmm5, xmm7 \n"	\
			"psrld xmm7, 10 \n"	\
			"movdqa xmm6, xmm5 \n"

#define GEN_W_9(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"psrld xmm6, 17 \n"	\
			"pslld xmm5, 15 \n"	\
			"por xmm5, xmm6 \n"	\

#define GEN_W_10(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"pxor xmm5, xmm7 \n"	\
			"pslldq xmm5, 8 \n"	\
			"psrldq xmm5, 8 \n"

#define GEN_W_11(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"paddd " xr_out ", xmm5 \n"	\
			"movdqa xmm5, " xr_out " \n"	\
			"movdqa xmm6, " xr_out " \n"

#define GEN_W_12(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"psrld xmm6, 2 \n"	\
			"pslld xmm5, 30 \n"	\
			"por xmm5, xmm6 \n"	\
			"pxor xmm5, " xr_out " \n"

#define GEN_W_13(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"movdqa xmm6, xmm5 \n"	\
			"psrld xmm6, 17 \n"	\
			"pslld xmm5, 15 \n"

#define GEN_W_14(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"por xmm5, xmm6 \n"	\
			"movdqa xmm7, " xr_out " \n"	\
			"psrld xmm7, 10 \n"	\
			"pxor xmm5, xmm7 \n"

#define GEN_W_15(i, xr_out, xrm4, xrm8, xrm12, xrm16)	\
			"pslldq xmm5, 8 \n"	\
			"paddd " xr_out ", xmm5 \n"	\
			"movdqa xmmword ptr[%8 + (" #i ") * 4], " xr_out " \n"

// 寄存器多就是好。
#ifdef _WIN64

#define X64_STEP_0(i, ra, rb, rc, rd, re, rf, rg, rh)	\
			"mov r10d, " ra " \n"	\
			"ror r10d, 9 \n"	\
			"xor r10d, " ra " \n"	\
			"ror r10d, 11 \n"	\
			"xor r10d, " ra " \n"	\
			"ror r10d, 2 \n"

#define X64_STEP_1(i, ra, rb, rc, rd, re, rf, rg, rh)	\
			"mov r11d, " ra " \n"	\
			"and r11d, " rb " \n"	\
			"mov r12d, " ra " \n"	\
			"xor r12d, " rb " \n"	\
			"and r12d, " rc " \n"	\
			"or r11d, r12d \n"	\
			"mov r12d, dword ptr[%8 + (" #i ") * 4] \n"	\
			"add r10d, r11d \n"

#define X64_STEP_2(i, ra, rb, rc, rd, re, rf, rg, rh)	\
			"mov r11d, " re " \n"	\
			"ror r11d, 14 \n"	\
			"xor r11d, " re " \n"	\
			"ror r11d, 5 \n"	\
			"xor r11d, " re " \n"	\
			"ror r11d, 6 \n"	\
			"add " rh ", r12d \n"	\
			"add " rh ", r11d \n"

#define X64_STEP_3(i, ra, rb, rc, rd, re, rf, rg, rh)	\
			"mov r11d, " rf " \n"	\
			"xor r11d, " rg " \n"	\
			"and r11d, " re " \n"	\
			"xor r11d, " rg " \n"	\
			"add " rh ", " K(i) " \n"	\
			"add " rh ", r11d \n"	\
			"add " rd ", " rh " \n"	\
			"add " rh ", r10d \n"

			X64_STEP_0( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_0 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_1 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_2 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_3 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_4 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_5 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_6 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_7 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_8 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_9 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_10(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_11(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_12(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_13(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_14(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_15(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_0 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_1 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_2 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_3 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_4 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_5 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_6 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_7 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_8 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_9 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_10(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_11(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_12(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_13(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_14(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_15(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_0 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_1 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_2 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_3 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_4 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_5 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_6 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_7 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_8 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_9 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_10(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_11(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_12(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_13(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_14(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_15(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_0 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_1 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_2 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_3 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_4 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_5 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_6 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_7 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_8 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_9 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_10(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_11(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_12(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_13(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_14(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_15(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")

			X64_STEP_0(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_0 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_1 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_2 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_3 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_4 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_5 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_6 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_7 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_8 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_9 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_10(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_11(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_12(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_13(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_14(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_15(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_0 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_1 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_2 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_3 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_4 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_5 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_6 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_7 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_8 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_9 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_10(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_11(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_12(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_13(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_14(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_15(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_0 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_1 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_2 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_3 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_4 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_5 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_6 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_7 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_8 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_9 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_10(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_11(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_12(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_13(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_14(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_15(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_0 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_1 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_2 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_3 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_4 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_5 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_6 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_7 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_8 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_9 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_10(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_11(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_0(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_12(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_1(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_13(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_2(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_14(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X64_STEP_3(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_15(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")

			X64_STEP_0(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_0 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_1 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_2 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_3 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_4 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_5 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_6 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_7 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_8 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_9 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_10(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_11(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_12(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_1(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_13(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_2(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_14(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_3(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_15(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X64_STEP_0(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_0 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_1 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_2 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_3 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_4 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_5 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_6 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_7 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_8 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_9 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_10(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_11(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_12(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_1(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_13(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_2(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_14(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_3(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_15(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X64_STEP_0(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_0 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_1 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_2 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")	GEN_W_3 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_4 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_5 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_6 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")	GEN_W_7 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_8 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_9 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_10(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")	GEN_W_11(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_12(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_1(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_13(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_2(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_14(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_3(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")	GEN_W_15(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X64_STEP_0(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_0 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_1 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_2 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")	GEN_W_3 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_4 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_5 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_6 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")	GEN_W_7 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_8 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_9 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_10(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")	GEN_W_11(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_0(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_12(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_1(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_13(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_2(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_14(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X64_STEP_3(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")	GEN_W_15(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")

			X64_STEP_0(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_1(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_2(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_3(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_0(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_1(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_2(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_3(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_0(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_1(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_2(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_3(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_0(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_1(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_2(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_3(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_0(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_1(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_2(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_3(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_0(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_1(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_2(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_3(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_0(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_1(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_2(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_3(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_0(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_1(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_2(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_3(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_0(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_1(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_2(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_3(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7")
			X64_STEP_0(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_1(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_2(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_3(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6")
			X64_STEP_0(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_1(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_2(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_3(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5")
			X64_STEP_0(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_1(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_2(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_3(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4")
			X64_STEP_0(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_1(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_2(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_3(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3")
			X64_STEP_0(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_1(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_2(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_3(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2")
			X64_STEP_0(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_1(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_2(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_3(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1")
			X64_STEP_0(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_1(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_2(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")
			X64_STEP_3(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0")

			: "=r"(a), "=r"(b), "=r"(c), "=r"(d), "=r"(e), "=r"(f), "=r"(g), "=r"(h), "=m"(w)
			: "0"(a), "1"(b), "2"(c), "3"(d), "4"(e), "5"(f), "6"(g), "7"(h), "m"(w)
			: "r10", "r11", "r12", "xmm6", "xmm7"

#else // _WIN64

			"mov ebx, dword ptr[%0] \n" // a
			"mov esi, dword ptr[%1] \n" // b
			"mov edi, dword ptr[%2] \n" // c

#define X86_STEP_0(i, ma, mb, mc, md, me, mf, mg, mh, ra, rb, rc_ra)	\
			"mov eax, " ra " \n"	\
			"ror eax, 9 \n"	\
			"xor eax, " ra " \n"	\
			"ror eax, 11 \n"	\
			"xor eax, " ra " \n"	\
			"ror eax, 2 \n"

#define X86_STEP_1(i, ma, mb, mc, md, me, mf, mg, mh, ra, rb, rc_ra)	\
			"mov ecx, " ra " \n"	\
			"and ecx, " rb " \n"	\
			"mov edx, " ra " \n"	\
			"xor edx, " rb " \n"	\
			"and edx, " rc_ra " \n"	\
			"or ecx, edx \n"	\
			"mov edx, dword ptr[%8 + (" #i ") * 4] \n"	\
			"add eax, ecx \n"

#define X86_STEP_2(i, ma, mb, mc, md, me, mf, mg, mh, ra, rb, rc_ra)	\
			"mov " rc_ra ", dword ptr[" me "] \n"	\
			"mov ecx, " rc_ra " \n"	\
			"ror ecx, 14 \n"	\
			"xor ecx, " rc_ra " \n"	\
			"ror ecx, 5 \n"	\
			"xor ecx, " rc_ra " \n"	\
			"mov " rc_ra ", dword ptr[" mh "] \n"	\
			"add " rc_ra ", edx \n"	\
			"ror ecx, 6 \n"	\
			"add " rc_ra ", ecx \n"

#define X86_STEP_3(i, ma, mb, mc, md, me, mf, mg, mh, ra, rb, rc_ra)	\
			"mov edx, dword ptr[" md "] \n"	\
			"mov ecx, " mf " \n"	\
			"xor ecx, " mg " \n"	\
			"and ecx, " me " \n"	\
			"xor ecx, " mg " \n"	\
			"add " rc_ra ", " K(i) " \n"	\
			"add " rc_ra ", ecx \n"	\
			"add edx, " rc_ra " \n"	\
			"mov dword ptr[" md "], edx \n"	\
			"add " rc_ra ", eax \n"	\
			"mov dword ptr[" mh "], " rc_ra " \n"

			X86_STEP_0( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_0 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_1 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_2 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3( 0, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_3 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_4 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_5 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_6 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3( 1, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_7 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_8 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_9 (16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_10(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3( 2, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_11(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_12(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_13(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_14(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3( 3, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_15(16, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_0 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_1 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_2 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3( 4, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_3 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_4 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_5 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_6 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3( 5, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_7 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_8 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_9 (20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_10(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3( 6, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_11(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_12(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_13(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_14(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3( 7, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_15(20, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_0 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_1 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_2 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3( 8, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_3 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_4 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_5 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_6 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3( 9, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_7 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_8 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_9 (24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_10(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3(10, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_11(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_12(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_13(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_14(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3(11, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_15(24, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_0 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_1 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_2 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(12, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_3 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_4 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_5 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_6 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(13, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_7 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_8 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_9 (28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_10(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(14, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_11(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_12(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_13(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_14(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(15, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_15(28, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")

			X86_STEP_0(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_0 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_1 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_2 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(16, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_3 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_4 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_5 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_6 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(17, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_7 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_8 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_9 (32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_10(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(18, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_11(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_12(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_13(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_14(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(19, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_15(32, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_0 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_1 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_2 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(20, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_3 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_4 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_5 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_6 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(21, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_7 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_8 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_9 (36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_10(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(22, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_11(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_12(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_13(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_14(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(23, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_15(36, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_0 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_1 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_2 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(24, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")	GEN_W_3 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_4 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_5 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_6 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(25, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")	GEN_W_7 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_8 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_9 (40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_10(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(26, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")	GEN_W_11(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_12(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_13(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_14(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(27, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")	GEN_W_15(40, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_0 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_1 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_2 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3(28, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")	GEN_W_3 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_4 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_5 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_6 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3(29, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")	GEN_W_7 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_8 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_9 (44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_10(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3(30, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")	GEN_W_11(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_0(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_12(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_1(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_13(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_2(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_14(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")
			X86_STEP_3(31, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")	GEN_W_15(44, "xmm3", "xmm4", "xmm0", "xmm1", "xmm2")

			X86_STEP_0(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_0 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_1 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_2 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(32, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")	GEN_W_3 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_4 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_5 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_6 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(33, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")	GEN_W_7 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_8 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_9 (48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_10(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(34, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")	GEN_W_11(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_12(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_1(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_13(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_2(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_14(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_3(35, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")	GEN_W_15(48, "xmm2", "xmm3", "xmm4", "xmm0", "xmm1")
			X86_STEP_0(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_0 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_1 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_2 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(36, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")	GEN_W_3 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_4 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_5 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_6 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(37, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")	GEN_W_7 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_8 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_9 (52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_10(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(38, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")	GEN_W_11(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_12(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_1(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_13(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_2(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_14(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_3(39, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")	GEN_W_15(52, "xmm1", "xmm2", "xmm3", "xmm4", "xmm0")
			X86_STEP_0(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_0 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_1 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_2 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(40, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "edi", "ebx", "esi")	GEN_W_3 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_4 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_5 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_6 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(41, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "esi", "edi", "ebx")	GEN_W_7 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_8 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_9 (56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_10(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(42, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "ebx", "esi", "edi")	GEN_W_11(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_12(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_1(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_13(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_2(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_14(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_3(43, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "edi", "ebx", "esi")	GEN_W_15(56, "xmm0", "xmm1", "xmm2", "xmm3", "xmm4")
			X86_STEP_0(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_0 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_1 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_2 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(44, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "esi", "edi", "ebx")	GEN_W_3 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_4 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_5 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_6 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(45, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "ebx", "esi", "edi")	GEN_W_7 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_8 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_9 (60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_10(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(46, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "edi", "ebx", "esi")	GEN_W_11(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_0(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_12(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_1(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_13(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_2(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_14(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")
			X86_STEP_3(47, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "esi", "edi", "ebx")	GEN_W_15(60, "xmm4", "xmm0", "xmm1", "xmm2", "xmm3")

			X86_STEP_0(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")
			X86_STEP_1(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")
			X86_STEP_2(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")
			X86_STEP_3(48, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "ebx", "esi", "edi")
			X86_STEP_0(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")
			X86_STEP_1(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")
			X86_STEP_2(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")
			X86_STEP_3(49, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "edi", "ebx", "esi")
			X86_STEP_0(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")
			X86_STEP_1(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")
			X86_STEP_2(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")
			X86_STEP_3(50, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "esi", "edi", "ebx")
			X86_STEP_0(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")
			X86_STEP_1(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")
			X86_STEP_2(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")
			X86_STEP_3(51, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "ebx", "esi", "edi")
			X86_STEP_0(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")
			X86_STEP_1(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")
			X86_STEP_2(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")
			X86_STEP_3(52, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "edi", "ebx", "esi")
			X86_STEP_0(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")
			X86_STEP_1(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")
			X86_STEP_2(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")
			X86_STEP_3(53, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "esi", "edi", "ebx")
			X86_STEP_0(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")
			X86_STEP_1(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")
			X86_STEP_2(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")
			X86_STEP_3(54, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "ebx", "esi", "edi")
			X86_STEP_0(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")
			X86_STEP_1(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")
			X86_STEP_2(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")
			X86_STEP_3(55, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "edi", "ebx", "esi")
			X86_STEP_0(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")
			X86_STEP_1(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")
			X86_STEP_2(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")
			X86_STEP_3(56, "%0", "%1", "%2", "%3", "%4", "%5", "%6", "%7", "esi", "edi", "ebx")
			X86_STEP_0(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")
			X86_STEP_1(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")
			X86_STEP_2(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")
			X86_STEP_3(57, "%7", "%0", "%1", "%2", "%3", "%4", "%5", "%6", "ebx", "esi", "edi")
			X86_STEP_0(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")
			X86_STEP_1(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")
			X86_STEP_2(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")
			X86_STEP_3(58, "%6", "%7", "%0", "%1", "%2", "%3", "%4", "%5", "edi", "ebx", "esi")
			X86_STEP_0(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")
			X86_STEP_1(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")
			X86_STEP_2(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")
			X86_STEP_3(59, "%5", "%6", "%7", "%0", "%1", "%2", "%3", "%4", "esi", "edi", "ebx")
			X86_STEP_0(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")
			X86_STEP_1(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")
			X86_STEP_2(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")
			X86_STEP_3(60, "%4", "%5", "%6", "%7", "%0", "%1", "%2", "%3", "ebx", "esi", "edi")
			X86_STEP_0(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")
			X86_STEP_1(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")
			X86_STEP_2(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")
			X86_STEP_3(61, "%3", "%4", "%5", "%6", "%7", "%0", "%1", "%2", "edi", "ebx", "esi")
			X86_STEP_0(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")
			X86_STEP_1(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")
			X86_STEP_2(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")
			X86_STEP_3(62, "%2", "%3", "%4", "%5", "%6", "%7", "%0", "%1", "esi", "edi", "ebx")
			X86_STEP_0(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")
			X86_STEP_1(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")
			X86_STEP_2(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")
			X86_STEP_3(63, "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%0", "ebx", "esi", "edi")

			: "=m"(a), "=m"(b), "=m"(c), "=m"(d), "=m"(e), "=m"(f), "=m"(g), "=m"(h), "=m"(w)
			: "m"(a), "m"(b), "m"(c), "m"(d), "m"(e), "m"(f), "m"(g), "m"(h), "m"(w)
			: "ax", "cx", "dx", "bx", "si", "di"

#endif // _WIN64

		);

		auResult[0] += a;
		auResult[1] += b;
		auResult[2] += c;
		auResult[3] += d;
		auResult[4] += e;
		auResult[5] += f;
		auResult[6] += g;
		auResult[7] += h;
	}
}

// 构造函数和析构函数。
SHA256::SHA256() noexcept
	: xm_bInited(false)
{
}

// 其他非静态成员函数。
void SHA256::Abort() noexcept {
	xm_bInited = false;
}
void SHA256::Update(const void *pData, std::size_t uSize) noexcept {
	if(!xm_bInited){
		xm_auResult[0] = 0x6A09E667u;
		xm_auResult[1] = 0xBB67AE85u;
		xm_auResult[2] = 0x3C6EF372u;
		xm_auResult[3] = 0xA54FF53Au;
		xm_auResult[4] = 0x510E527Fu;
		xm_auResult[5] = 0x9B05688Cu;
		xm_auResult[6] = 0x1F83D9ABu;
		xm_auResult[7] = 0x5BE0CD19u;

		xm_uBytesInChunk = 0;
		xm_u64BytesTotal = 0;

		xm_bInited = true;
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
	xm_u64BytesTotal += uSize;
}
void SHA256::Finalize(unsigned char (&abyOutput)[32]) noexcept {
	if(xm_bInited){
		xm_abyChunk[xm_uBytesInChunk++] = 0x80;
		if(xm_uBytesInChunk > sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyChunk) - xm_uBytesInChunk);
			DoSHA256Chunk(xm_auResult, xm_abyChunk);
			xm_uBytesInChunk = 0;
		}
		if(xm_uBytesInChunk < sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyFirstPart) - xm_uBytesInChunk);
		}
		xm_uBitsTotal = __builtin_bswap64(xm_u64BytesTotal * CHAR_BIT);
		DoSHA256Chunk(xm_auResult, xm_abyChunk);

		for(auto &u : xm_auResult){
			u = __builtin_bswap32(u);
		}

		xm_bInited = false;
	}
	__builtin_memcpy(abyOutput, xm_auResult, sizeof(xm_auResult));
}
