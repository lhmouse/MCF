// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "Md5OutputStream.hpp"
#include "../Core/Array.hpp"
#include "../Core/Endian.hpp"

namespace MCF {

// https://en.wikipedia.org/wiki/MD5

Md5OutputStream::~Md5OutputStream(){
}

void Md5OutputStream::X_Initialize() noexcept {
	x_au32Reg       = { 0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u };
	x_u64BytesTotal = 0;
}
void Md5OutputStream::X_Update(const std::uint8_t (&abyChunk)[64]) noexcept {
/*
	static constexpr unsigned char RVEC[64] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};
	static constexpr std::uint32_t KVEC[64] = {
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

	const auto w = (const std::uint32_t *)abyChunk;

	std::uint32_t a = x_au32Reg[0];
	std::uint32_t b = x_au32Reg[1];
	std::uint32_t c = x_au32Reg[2];
	std::uint32_t d = x_au32Reg[3];

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
		b += ::_rotl(a + f + KVEC[i] + LoadLe(w[g]), RVEC[i]);
		a = temp;
	}

	x_au32Reg[0] += a;
	x_au32Reg[1] += b;
	x_au32Reg[2] += c;
	x_au32Reg[3] += d;
*/

#define R_0     "7"
#define R_1     "12"
#define R_2     "17"
#define R_3     "22"
#define R_4     "7"
#define R_5     "12"
#define R_6     "17"
#define R_7     "22"
#define R_8     "7"
#define R_9     "12"
#define R_10    "17"
#define R_11    "22"
#define R_12    "7"
#define R_13    "12"
#define R_14    "17"
#define R_15    "22"
#define R_16    "5"
#define R_17    "9"
#define R_18    "14"
#define R_19    "20"
#define R_20    "5"
#define R_21    "9"
#define R_22    "14"
#define R_23    "20"
#define R_24    "5"
#define R_25    "9"
#define R_26    "14"
#define R_27    "20"
#define R_28    "5"
#define R_29    "9"
#define R_30    "14"
#define R_31    "20"
#define R_32    "4"
#define R_33    "11"
#define R_34    "16"
#define R_35    "23"
#define R_36    "4"
#define R_37    "11"
#define R_38    "16"
#define R_39    "23"
#define R_40    "4"
#define R_41    "11"
#define R_42    "16"
#define R_43    "23"
#define R_44    "4"
#define R_45    "11"
#define R_46    "16"
#define R_47    "23"
#define R_48    "6"
#define R_49    "10"
#define R_50    "15"
#define R_51    "21"
#define R_52    "6"
#define R_53    "10"
#define R_54    "15"
#define R_55    "21"
#define R_56    "6"
#define R_57    "10"
#define R_58    "15"
#define R_59    "21"
#define R_60    "6"
#define R_61    "10"
#define R_62    "15"
#define R_63    "21"
#define R(i_)   R_ ## i_

#define K_0     "0xD76AA478"
#define K_1     "0xE8C7B756"
#define K_2     "0x242070DB"
#define K_3     "0xC1BDCEEE"
#define K_4     "0xF57C0FAF"
#define K_5     "0x4787C62A"
#define K_6     "0xA8304613"
#define K_7     "0xFD469501"
#define K_8     "0x698098D8"
#define K_9     "0x8B44F7AF"
#define K_10    "0xFFFF5BB1"
#define K_11    "0x895CD7BE"
#define K_12    "0x6B901122"
#define K_13    "0xFD987193"
#define K_14    "0xA679438E"
#define K_15    "0x49B40821"
#define K_16    "0xF61E2562"
#define K_17    "0xC040B340"
#define K_18    "0x265E5A51"
#define K_19    "0xE9B6C7AA"
#define K_20    "0xD62F105D"
#define K_21    "0x02441453"
#define K_22    "0xD8A1E681"
#define K_23    "0xE7D3FBC8"
#define K_24    "0x21E1CDE6"
#define K_25    "0xC33707D6"
#define K_26    "0xF4D50D87"
#define K_27    "0x455A14ED"
#define K_28    "0xA9E3E905"
#define K_29    "0xFCEFA3F8"
#define K_30    "0x676F02D9"
#define K_31    "0x8D2A4C8A"
#define K_32    "0xFFFA3942"
#define K_33    "0x8771F681"
#define K_34    "0x6D9D6122"
#define K_35    "0xFDE5380C"
#define K_36    "0xA4BEEA44"
#define K_37    "0x4BDECFA9"
#define K_38    "0xF6BB4B60"
#define K_39    "0xBEBFBC70"
#define K_40    "0x289B7EC6"
#define K_41    "0xEAA127FA"
#define K_42    "0xD4EF3085"
#define K_43    "0x04881D05"
#define K_44    "0xD9D4D039"
#define K_45    "0xE6DB99E5"
#define K_46    "0x1FA27CF8"
#define K_47    "0xC4AC5665"
#define K_48    "0xF4292244"
#define K_49    "0x432AFF97"
#define K_50    "0xAB9423A7"
#define K_51    "0xFC93A039"
#define K_52    "0x655B59C3"
#define K_53    "0x8F0CCC92"
#define K_54    "0xFFEFF47D"
#define K_55    "0x85845DD1"
#define K_56    "0x6FA87E4F"
#define K_57    "0xFE2CE6E0"
#define K_58    "0xA3014314"
#define K_59    "0x4E0811A1"
#define K_60    "0xF7537E82"
#define K_61    "0xBD3AF235"
#define K_62    "0x2AD7D2BB"
#define K_63    "0xEB86D391"
#define K(i_)   K_ ## i_

	auto a = x_au32Reg[0];
	auto b = x_au32Reg[1];
	auto c = x_au32Reg[2];
	auto d = x_au32Reg[3];

	__asm__ volatile (

#define STEP_A(i_, ra_, rb_, rc_, rd_)	\
		"add " ra_ ", dword ptr[%4 + (" #i_ ") * 4] \n"	\
		"mov edi, " rc_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"and edi, " rb_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"add " ra_ ", edi \n"	\
		"add " ra_ ", " K(i_) " \n"	\
		"rol " ra_ ", " R(i_) " \n"	\
		"add " ra_ ", " rb_ " \n"

#define STEP_B(i_, ra_, rb_, rc_, rd_)	\
		"add " ra_ ", dword ptr[%4 + ((5 * (" #i_ ") + 1) %% 16) * 4] \n"	\
		"mov edi, " rb_ " \n"	\
		"xor edi, " rc_ " \n"	\
		"and edi, " rd_ " \n"	\
		"xor edi, " rc_ " \n"	\
		"add " ra_ ", edi \n"	\
		"add " ra_ ", " K(i_) " \n"	\
		"rol " ra_ ", " R(i_) " \n"	\
		"add " ra_ ", " rb_ " \n"

#define STEP_C(i_, ra_, rb_, rc_, rd_)	\
		"add " ra_ ", dword ptr[%4 + ((3 * (" #i_ ") + 5) %% 16) * 4] \n"	\
		"mov edi, " rb_ " \n"	\
		"xor edi, " rc_ " \n"	\
		"xor edi, " rd_ " \n"	\
		"add " ra_ ", edi \n"	\
		"add " ra_ ", " K(i_) " \n"	\
		"rol " ra_ ", " R(i_) " \n"	\
		"add " ra_ ", " rb_ " \n"

#define STEP_D(i_, ra_, rb_, rc_, rd_)	\
		"add " ra_ ", dword ptr[%4 + ((7 * (" #i_ ")) %% 16) * 4] \n"	\
		"mov edi, " rd_ " \n"	\
		"not edi \n"	\
		"or edi, " rb_ " \n"	\
		"xor edi, " rc_ " \n"	\
		"add " ra_ ", edi \n"	\
		"add " ra_ ", " K(i_) " \n"	\
		"rol " ra_ ", " R(i_) " \n"	\
		"add " ra_ ", " rb_ " \n"

		STEP_A( 0, "%0", "%1", "%2", "%3")
		STEP_A( 1, "%3", "%0", "%1", "%2")
		STEP_A( 2, "%2", "%3", "%0", "%1")
		STEP_A( 3, "%1", "%2", "%3", "%0")
		STEP_A( 4, "%0", "%1", "%2", "%3")
		STEP_A( 5, "%3", "%0", "%1", "%2")
		STEP_A( 6, "%2", "%3", "%0", "%1")
		STEP_A( 7, "%1", "%2", "%3", "%0")
		STEP_A( 8, "%0", "%1", "%2", "%3")
		STEP_A( 9, "%3", "%0", "%1", "%2")
		STEP_A(10, "%2", "%3", "%0", "%1")
		STEP_A(11, "%1", "%2", "%3", "%0")
		STEP_A(12, "%0", "%1", "%2", "%3")
		STEP_A(13, "%3", "%0", "%1", "%2")
		STEP_A(14, "%2", "%3", "%0", "%1")
		STEP_A(15, "%1", "%2", "%3", "%0")

		STEP_B(16, "%0", "%1", "%2", "%3")
		STEP_B(17, "%3", "%0", "%1", "%2")
		STEP_B(18, "%2", "%3", "%0", "%1")
		STEP_B(19, "%1", "%2", "%3", "%0")
		STEP_B(20, "%0", "%1", "%2", "%3")
		STEP_B(21, "%3", "%0", "%1", "%2")
		STEP_B(22, "%2", "%3", "%0", "%1")
		STEP_B(23, "%1", "%2", "%3", "%0")
		STEP_B(24, "%0", "%1", "%2", "%3")
		STEP_B(25, "%3", "%0", "%1", "%2")
		STEP_B(26, "%2", "%3", "%0", "%1")
		STEP_B(27, "%1", "%2", "%3", "%0")
		STEP_B(28, "%0", "%1", "%2", "%3")
		STEP_B(29, "%3", "%0", "%1", "%2")
		STEP_B(30, "%2", "%3", "%0", "%1")
		STEP_B(31, "%1", "%2", "%3", "%0")

		STEP_C(32, "%0", "%1", "%2", "%3")
		STEP_C(33, "%3", "%0", "%1", "%2")
		STEP_C(34, "%2", "%3", "%0", "%1")
		STEP_C(35, "%1", "%2", "%3", "%0")
		STEP_C(36, "%0", "%1", "%2", "%3")
		STEP_C(37, "%3", "%0", "%1", "%2")
		STEP_C(38, "%2", "%3", "%0", "%1")
		STEP_C(39, "%1", "%2", "%3", "%0")
		STEP_C(40, "%0", "%1", "%2", "%3")
		STEP_C(41, "%3", "%0", "%1", "%2")
		STEP_C(42, "%2", "%3", "%0", "%1")
		STEP_C(43, "%1", "%2", "%3", "%0")
		STEP_C(44, "%0", "%1", "%2", "%3")
		STEP_C(45, "%3", "%0", "%1", "%2")
		STEP_C(46, "%2", "%3", "%0", "%1")
		STEP_C(47, "%1", "%2", "%3", "%0")

		STEP_D(48, "%0", "%1", "%2", "%3")
		STEP_D(49, "%3", "%0", "%1", "%2")
		STEP_D(50, "%2", "%3", "%0", "%1")
		STEP_D(51, "%1", "%2", "%3", "%0")
		STEP_D(52, "%0", "%1", "%2", "%3")
		STEP_D(53, "%3", "%0", "%1", "%2")
		STEP_D(54, "%2", "%3", "%0", "%1")
		STEP_D(55, "%1", "%2", "%3", "%0")
		STEP_D(56, "%0", "%1", "%2", "%3")
		STEP_D(57, "%3", "%0", "%1", "%2")
		STEP_D(58, "%2", "%3", "%0", "%1")
		STEP_D(59, "%1", "%2", "%3", "%0")
		STEP_D(60, "%0", "%1", "%2", "%3")
		STEP_D(61, "%3", "%0", "%1", "%2")
		STEP_D(62, "%2", "%3", "%0", "%1")
		STEP_D(63, "%1", "%2", "%3", "%0")

		: "=r"(a), "=r"(b), "=r"(c), "=r"(d)
		: "r"(abyChunk), "0"(a), "1"(b), "2"(c), "3"(d)
		: "di"
	);

	x_au32Reg[0] += a;
	x_au32Reg[1] += b;
	x_au32Reg[2] += c;
	x_au32Reg[3] += d;
}
void Md5OutputStream::X_Finalize(std::uint8_t (&abyChunk)[64], unsigned uBytesInChunk) noexcept {
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
	StoreLe(u64BitsTotal, x_u64BytesTotal * 8);
	std::memcpy(abyChunk + 56, &u64BitsTotal, 8);
	X_Update(abyChunk);
}

void Md5OutputStream::Put(unsigned char byData) noexcept {
	Put(&byData, 1);
}
void Md5OutputStream::Put(const void *pData, std::size_t uSize) noexcept {
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
void Md5OutputStream::Flush(bool bHard) noexcept {
	(void)bHard;
}

void Md5OutputStream::Reset() noexcept {
	x_nChunkOffset = -1;
}
Array<std::uint8_t, 16> Md5OutputStream::Finalize() noexcept {
	if(x_nChunkOffset >= 0){
		X_Finalize(x_abyChunk, static_cast<unsigned>(x_nChunkOffset));
	} else {
		X_Initialize();
		X_Finalize(x_abyChunk, 0);
	}
	x_nChunkOffset = -1;

	Array<std::uint8_t, 16> abyRet;
	const auto pu32RetWords = reinterpret_cast<std::uint32_t *>(abyRet.GetData());
	for(unsigned i = 0; i < 4; ++i){
		StoreLe(pu32RetWords[i], x_au32Reg[i]);
	}
	return abyRet;
}

}
