// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "MD5.hpp"
#include <cstring>
using namespace MCF;

namespace {
	void DoMD5Chunk(std::uint32_t (&auResult)[4], const unsigned char *pbyChunk){
		// https://en.wikipedia.org/wiki/MD5
/*
		static const unsigned char RVEC[64] = {
			7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
			5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
			4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
			6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
		};
		static const std::uint32_t KVEC[64] = {
			0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
			0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
			0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
			0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
			0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
			0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
			0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
			0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
			0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
			0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
			0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
			0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
			0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
			0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
			0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
			0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
		};

		const auto w = (const std::uint32_t *)pbyChunk;

		std::uint32_t a = auResult[0];
		std::uint32_t b = auResult[1];
		std::uint32_t c = auResult[2];
		std::uint32_t d = auResult[3];

		for(std::size_t i = 0; i < 64; ++i){
			std::uint32_t f, g;

			switch(i / 16){
				case 0:
					// f = (b & c) | (~b & d);
					f = d ^ (b & (c ^ d));
					g = i;
					break;
				case 1:
					// f = (d & b) | (~d & c);
					f = c ^ (d & (b ^ c));
					g = (5 * i + 1) % 16;
					break;
				case 2:
					f = b ^ c ^ d;
					g = (3 * i + 5) % 16;
					break;
				default:
					f = c ^ (b | ~d);
					g = (7 * i) % 16;
					break;
			}

			const std::uint32_t temp = d;
			d = c;
			c = b;
			b += ::_rotl(a + f + KVEC[i] + w[g], RVEC[i]);
			a = temp;
		}

		auResult[0] += a;
		auResult[1] += b;
		auResult[2] += c;
		auResult[3] += d;
*/

#define R_0		"7"
#define R_1		"12"
#define R_2		"17"
#define R_3		"22"
#define R_4		"7"
#define R_5		"12"
#define R_6		"17"
#define R_7		"22"
#define R_8		"7"
#define R_9		"12"
#define R_10	"17"
#define R_11	"22"
#define R_12	"7"
#define R_13	"12"
#define R_14	"17"
#define R_15	"22"
#define R_16	"5"
#define R_17	"9"
#define R_18	"14"
#define R_19	"20"
#define R_20	"5"
#define R_21	"9"
#define R_22	"14"
#define R_23	"20"
#define R_24	"5"
#define R_25	"9"
#define R_26	"14"
#define R_27	"20"
#define R_28	"5"
#define R_29	"9"
#define R_30	"14"
#define R_31	"20"
#define R_32	"4"
#define R_33	"11"
#define R_34	"16"
#define R_35	"23"
#define R_36	"4"
#define R_37	"11"
#define R_38	"16"
#define R_39	"23"
#define R_40	"4"
#define R_41	"11"
#define R_42	"16"
#define R_43	"23"
#define R_44	"4"
#define R_45	"11"
#define R_46	"16"
#define R_47	"23"
#define R_48	"6"
#define R_49	"10"
#define R_50	"15"
#define R_51	"21"
#define R_52	"6"
#define R_53	"10"
#define R_54	"15"
#define R_55	"21"
#define R_56	"6"
#define R_57	"10"
#define R_58	"15"
#define R_59	"21"
#define R_60	"6"
#define R_61	"10"
#define R_62	"15"
#define R_63	"21"
#define R(i)	R_##i

#define K_0		"0xD76AA478"
#define K_1		"0xE8C7B756"
#define K_2		"0x242070DB"
#define K_3		"0xC1BDCEEE"
#define K_4		"0xF57C0FAF"
#define K_5		"0x4787C62A"
#define K_6		"0xA8304613"
#define K_7		"0xFD469501"
#define K_8		"0x698098D8"
#define K_9		"0x8B44F7AF"
#define K_10	"0xFFFF5BB1"
#define K_11	"0x895CD7BE"
#define K_12	"0x6B901122"
#define K_13	"0xFD987193"
#define K_14	"0xA679438E"
#define K_15	"0x49B40821"
#define K_16	"0xF61E2562"
#define K_17	"0xC040B340"
#define K_18	"0x265E5A51"
#define K_19	"0xE9B6C7AA"
#define K_20	"0xD62F105D"
#define K_21	"0x02441453"
#define K_22	"0xD8A1E681"
#define K_23	"0xE7D3FBC8"
#define K_24	"0x21E1CDE6"
#define K_25	"0xC33707D6"
#define K_26	"0xF4D50D87"
#define K_27	"0x455A14ED"
#define K_28	"0xA9E3E905"
#define K_29	"0xFCEFA3F8"
#define K_30	"0x676F02D9"
#define K_31	"0x8D2A4C8A"
#define K_32	"0xFFFA3942"
#define K_33	"0x8771F681"
#define K_34	"0x6D9D6122"
#define K_35	"0xFDE5380C"
#define K_36	"0xA4BEEA44"
#define K_37	"0x4BDECFA9"
#define K_38	"0xF6BB4B60"
#define K_39	"0xBEBFBC70"
#define K_40	"0x289B7EC6"
#define K_41	"0xEAA127FA"
#define K_42	"0xD4EF3085"
#define K_43	"0x04881D05"
#define K_44	"0xD9D4D039"
#define K_45	"0xE6DB99E5"
#define K_46	"0x1FA27CF8"
#define K_47	"0xC4AC5665"
#define K_48	"0xF4292244"
#define K_49	"0x432AFF97"
#define K_50	"0xAB9423A7"
#define K_51	"0xFC93A039"
#define K_52	"0x655B59C3"
#define K_53	"0x8F0CCC92"
#define K_54	"0xFFEFF47D"
#define K_55	"0x85845DD1"
#define K_56	"0x6FA87E4F"
#define K_57	"0xFE2CE6E0"
#define K_58	"0xA3014314"
#define K_59	"0x4E0811A1"
#define K_60	"0xF7537E82"
#define K_61	"0xBD3AF235"
#define K_62	"0x2AD7D2BB"
#define K_63	"0xEB86D391"
#define K(i)	K_##i

		register std::uint32_t a __asm__("ax") = auResult[0];
		register std::uint32_t b __asm__("bx") = auResult[1];
		register std::uint32_t c __asm__("cx") = auResult[2];
		register std::uint32_t d __asm__("dx") = auResult[3];

#ifdef _WIN64
#	define SI	"rsi"
#else
#	define SI	"esi"
#endif
		__asm__ __volatile__(

#define STEP_0(i, ra, rb, rc, rd)	\
			"add " ra ", dword ptr[" SI " + (" #i ") * 4] \n"	\
			"mov edi, " rc " \n"	\
			"xor edi, " rd " \n"	\
			"and edi, " rb " \n"	\
			"xor edi, " rd " \n"	\
			"add " ra ", edi \n"	\
			"add " ra ", " K(i) " \n"	\
			"rol " ra ", " R(i) " \n"	\
			"add " ra ", " rb " \n"

#define STEP_1(i, ra, rb, rc, rd)	\
			"add " ra ", dword ptr[" SI " + ((5 * (" #i ") + 1) %% 16) * 4] \n"	\
			"mov edi, " rb " \n"	\
			"xor edi, " rc " \n"	\
			"and edi, " rd " \n"	\
			"xor edi, " rc " \n"	\
			"add " ra ", edi \n"	\
			"add " ra ", " K(i) " \n"	\
			"rol " ra ", " R(i) " \n"	\
			"add " ra ", " rb " \n"

#define STEP_2(i, ra, rb, rc, rd)	\
			"add " ra ", dword ptr[" SI " + ((3 * (" #i ") + 5) %% 16) * 4] \n"	\
			"mov edi, " rb " \n"	\
			"xor edi, " rc " \n"	\
			"xor edi, " rd " \n"	\
			"add " ra ", edi \n"	\
			"add " ra ", " K(i) " \n"	\
			"rol " ra ", " R(i) " \n"	\
			"add " ra ", " rb " \n"

#define STEP_3(i, ra, rb, rc, rd)	\
			"add " ra ", dword ptr[" SI " + ((7 * (" #i ")) %% 16) * 4] \n"	\
			"mov edi, " rd " \n"	\
			"not edi \n"	\
			"or edi, " rb " \n"	\
			"xor edi, " rc " \n"	\
			"add " ra ", edi \n"	\
			"add " ra ", " K(i) " \n"	\
			"rol " ra ", " R(i) " \n"	\
			"add " ra ", " rb " \n"

			STEP_0(0 , "eax", "ebx", "ecx", "edx")
			STEP_0(1 , "edx", "eax", "ebx", "ecx")
			STEP_0(2 , "ecx", "edx", "eax", "ebx")
			STEP_0(3 , "ebx", "ecx", "edx", "eax")
			STEP_0(4 , "eax", "ebx", "ecx", "edx")
			STEP_0(5 , "edx", "eax", "ebx", "ecx")
			STEP_0(6 , "ecx", "edx", "eax", "ebx")
			STEP_0(7 , "ebx", "ecx", "edx", "eax")
			STEP_0(8 , "eax", "ebx", "ecx", "edx")
			STEP_0(9 , "edx", "eax", "ebx", "ecx")
			STEP_0(10, "ecx", "edx", "eax", "ebx")
			STEP_0(11, "ebx", "ecx", "edx", "eax")
			STEP_0(12, "eax", "ebx", "ecx", "edx")
			STEP_0(13, "edx", "eax", "ebx", "ecx")
			STEP_0(14, "ecx", "edx", "eax", "ebx")
			STEP_0(15, "ebx", "ecx", "edx", "eax")

			STEP_1(16, "eax", "ebx", "ecx", "edx")
			STEP_1(17, "edx", "eax", "ebx", "ecx")
			STEP_1(18, "ecx", "edx", "eax", "ebx")
			STEP_1(19, "ebx", "ecx", "edx", "eax")
			STEP_1(20, "eax", "ebx", "ecx", "edx")
			STEP_1(21, "edx", "eax", "ebx", "ecx")
			STEP_1(22, "ecx", "edx", "eax", "ebx")
			STEP_1(23, "ebx", "ecx", "edx", "eax")
			STEP_1(24, "eax", "ebx", "ecx", "edx")
			STEP_1(25, "edx", "eax", "ebx", "ecx")
			STEP_1(26, "ecx", "edx", "eax", "ebx")
			STEP_1(27, "ebx", "ecx", "edx", "eax")
			STEP_1(28, "eax", "ebx", "ecx", "edx")
			STEP_1(29, "edx", "eax", "ebx", "ecx")
			STEP_1(30, "ecx", "edx", "eax", "ebx")
			STEP_1(31, "ebx", "ecx", "edx", "eax")

			STEP_2(32, "eax", "ebx", "ecx", "edx")
			STEP_2(33, "edx", "eax", "ebx", "ecx")
			STEP_2(34, "ecx", "edx", "eax", "ebx")
			STEP_2(35, "ebx", "ecx", "edx", "eax")
			STEP_2(36, "eax", "ebx", "ecx", "edx")
			STEP_2(37, "edx", "eax", "ebx", "ecx")
			STEP_2(38, "ecx", "edx", "eax", "ebx")
			STEP_2(39, "ebx", "ecx", "edx", "eax")
			STEP_2(40, "eax", "ebx", "ecx", "edx")
			STEP_2(41, "edx", "eax", "ebx", "ecx")
			STEP_2(42, "ecx", "edx", "eax", "ebx")
			STEP_2(43, "ebx", "ecx", "edx", "eax")
			STEP_2(44, "eax", "ebx", "ecx", "edx")
			STEP_2(45, "edx", "eax", "ebx", "ecx")
			STEP_2(46, "ecx", "edx", "eax", "ebx")
			STEP_2(47, "ebx", "ecx", "edx", "eax")

			STEP_3(48, "eax", "ebx", "ecx", "edx")
			STEP_3(49, "edx", "eax", "ebx", "ecx")
			STEP_3(50, "ecx", "edx", "eax", "ebx")
			STEP_3(51, "ebx", "ecx", "edx", "eax")
			STEP_3(52, "eax", "ebx", "ecx", "edx")
			STEP_3(53, "edx", "eax", "ebx", "ecx")
			STEP_3(54, "ecx", "edx", "eax", "ebx")
			STEP_3(55, "ebx", "ecx", "edx", "eax")
			STEP_3(56, "eax", "ebx", "ecx", "edx")
			STEP_3(57, "edx", "eax", "ebx", "ecx")
			STEP_3(58, "ecx", "edx", "eax", "ebx")
			STEP_3(59, "ebx", "ecx", "edx", "eax")
			STEP_3(60, "eax", "ebx", "ecx", "edx")
			STEP_3(61, "edx", "eax", "ebx", "ecx")
			STEP_3(62, "ecx", "edx", "eax", "ebx")
			STEP_3(63, "ebx", "ecx", "edx", "eax")

			: "=a"(a), "=b"(b), "=c"(c), "=d"(d)
			: "0"(a), "1"(b), "2"(c), "3"(d), "S"(pbyChunk)
			: "di"
		);

		auResult[0] += a;
		auResult[1] += b;
		auResult[2] += c;
		auResult[3] += d;
	}
}

// 构造函数和析构函数。
MD5::MD5() noexcept {
	xm_bInited = false;
}

// 其他非静态成员函数。
void MD5::Update(const void *pData, std::size_t uSize) noexcept {
	if(!xm_bInited){
		xm_bInited = true;

		xm_auResult[0] = 0x67452301;
		xm_auResult[1] = 0xEFCDAB89;
		xm_auResult[2] = 0x98BADCFE;
		xm_auResult[3] = 0x10325476;

		xm_uBytesInChunk = 0;
		xm_uBytesTotal = 0;
	}

	register auto pbyRead = (const unsigned char *)pData;
	std::size_t uBytesRemaining = uSize;
	const std::size_t uBytesFree = sizeof(xm_abyChunk) - xm_uBytesInChunk;
	if(uBytesRemaining >= uBytesFree){
		if(xm_uBytesInChunk != 0){
			std::memcpy(xm_abyChunk + xm_uBytesInChunk, pbyRead, uBytesFree);
			xm_uBytesInChunk = 0;

			DoMD5Chunk(xm_auResult, xm_abyChunk);
			pbyRead += uBytesFree;
			uBytesRemaining -= uBytesFree;
		}
		while(uBytesRemaining >= sizeof(xm_abyChunk)){
			DoMD5Chunk(xm_auResult, pbyRead);
			pbyRead += sizeof(xm_abyChunk);
			uBytesRemaining -= sizeof(xm_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(xm_abyChunk + xm_uBytesInChunk, pbyRead, uBytesRemaining);
		xm_uBytesInChunk = uBytesRemaining;
	}
	xm_uBytesTotal += uSize;
}
void MD5::Finalize(unsigned char (&abyOutput)[16]) noexcept {
	if(xm_bInited){
		xm_bInited = false;

		xm_abyChunk[xm_uBytesInChunk++] = 0x80;
		if(xm_uBytesInChunk > sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyChunk) - xm_uBytesInChunk);
			DoMD5Chunk(xm_auResult, xm_abyChunk);
			xm_uBytesInChunk = 0;
		}
		if(xm_uBytesInChunk < sizeof(xm_abyFirstPart)){
			std::memset(xm_abyChunk + xm_uBytesInChunk, 0, sizeof(xm_abyFirstPart) - xm_uBytesInChunk);
		}
		xm_uBitsTotal = xm_uBytesTotal * CHAR_BIT;
		DoMD5Chunk(xm_auResult, xm_abyChunk);
	}
	__builtin_memcpy(abyOutput, xm_auResult, sizeof(xm_auResult));
}
