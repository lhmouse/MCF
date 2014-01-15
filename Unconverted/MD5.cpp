// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "MD5.hpp"
using namespace MCF;

// 静态成员函数。
__declspec(naked) void __stdcall MD5Hasher::xHashChunk(DWORD (* /* pardwOutput */)[4], const BYTE (* /* pabyChunk */)[64]){
	// 参考文献：
	// https://en.wikipedia.org/wiki/MD5

/*
	static const BYTE RVEC[64] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};
	static const DWORD KVEC[64] = {
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

	const DWORD *const w = (const DWORD *)*pabyChunk;

	DWORD a = (*pardwOutput)[0];
	DWORD b = (*pardwOutput)[1];
	DWORD c = (*pardwOutput)[2];
	DWORD d = (*pardwOutput)[3];

	for(std::size_t i = 0; i < 64; ++i){
		DWORD f, g;

		switch(i / 16){
			case 0:
				//f = (b & c) | (~b & d);
				f = d ^ (b & (c ^ d));
				g = i;
				break;
			case 1:
				//f = (d & b) | (~d & c);
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

		const DWORD temp = d;
		d = c;
		c = b;
		b = b + ::_lrotl((a + f + KVEC[i] + w[g]) , RVEC[i]);
		a = temp;
	}

	(*pardwOutput)[0] += a;
	(*pardwOutput)[1] += b;
	(*pardwOutput)[2] += c;
	(*pardwOutput)[3] += d;
*/

	__asm {
		push ebx
		push ebp
		push esi
		push edi
	}
	// esp	->		<old edi>
	//		+  4h	<old esi>
	//		+  8h	<old ebp>
	//		+ 0Ch	<old ebx>
	//		+ 10h	<ret addr>
	//		+ 14h	pdwDigest
	//		+ 18h	pabyChunk

#define LPDIGEST	(esp + 14h)
#define LPBYCHUNK	(esp + 18h)

#define REG_A		esi
#define REG_B		ebx
#define REG_C		ecx
#define REG_D		edx

#define REG_F		ebp
#define REG_TMP		eax
#define REG_W		edi

	__asm {
		mov REG_TMP, dword ptr[LPDIGEST]
		mov REG_A, dword ptr[REG_TMP]
		mov REG_B, dword ptr[REG_TMP + 4h]
		mov REG_C, dword ptr[REG_TMP + 8h]
		mov REG_D, dword ptr[REG_TMP + 0Ch]

		mov REG_W, dword ptr[LPBYCHUNK]
	}

#define xR_0h		7
#define xR_1h		12
#define xR_2h		17
#define xR_3h		22
#define xR_4h		7
#define xR_5h		12
#define xR_6h		17
#define xR_7h		22
#define xR_8h		7
#define xR_9h		12
#define xR_0Ah		17
#define xR_0Bh		22
#define xR_0Ch		7
#define xR_0Dh		12
#define xR_0Eh		17
#define xR_0Fh		22
#define xR_10h		5
#define xR_11h		9
#define xR_12h		14
#define xR_13h		20
#define xR_14h		5
#define xR_15h		9
#define xR_16h		14
#define xR_17h		20
#define xR_18h		5
#define xR_19h		9
#define xR_1Ah		14
#define xR_1Bh		20
#define xR_1Ch		5
#define xR_1Dh		9
#define xR_1Eh		14
#define xR_1Fh		20
#define xR_20h		4
#define xR_21h		11
#define xR_22h		16
#define xR_23h		23
#define xR_24h		4
#define xR_25h		11
#define xR_26h		16
#define xR_27h		23
#define xR_28h		4
#define xR_29h		11
#define xR_2Ah		16
#define xR_2Bh		23
#define xR_2Ch		4
#define xR_2Dh		11
#define xR_2Eh		16
#define xR_2Fh		23
#define xR_30h		6
#define xR_31h		10
#define xR_32h		15
#define xR_33h		21
#define xR_34h		6
#define xR_35h		10
#define xR_36h		15
#define xR_37h		21
#define xR_38h		6
#define xR_39h		10
#define xR_3Ah		15
#define xR_3Bh		21
#define xR_3Ch		6
#define xR_3Dh		10
#define xR_3Eh		15
#define xR_3Fh		21

#define xK_0h		0xD76AA478
#define xK_1h		0xE8C7B756
#define xK_2h		0x242070DB
#define xK_3h		0xC1BDCEEE
#define xK_4h		0xF57C0FAF
#define xK_5h		0x4787C62A
#define xK_6h		0xA8304613
#define xK_7h		0xFD469501
#define xK_8h		0x698098D8
#define xK_9h		0x8B44F7AF
#define xK_0Ah		0xFFFF5BB1
#define xK_0Bh		0x895CD7BE
#define xK_0Ch		0x6B901122
#define xK_0Dh		0xFD987193
#define xK_0Eh		0xA679438E
#define xK_0Fh		0x49B40821
#define xK_10h		0xF61E2562
#define xK_11h		0xC040B340
#define xK_12h		0x265E5A51
#define xK_13h		0xE9B6C7AA
#define xK_14h		0xD62F105D
#define xK_15h		0x02441453
#define xK_16h		0xD8A1E681
#define xK_17h		0xE7D3FBC8
#define xK_18h		0x21E1CDE6
#define xK_19h		0xC33707D6
#define xK_1Ah		0xF4D50D87
#define xK_1Bh		0x455A14ED
#define xK_1Ch		0xA9E3E905
#define xK_1Dh		0xFCEFA3F8
#define xK_1Eh		0x676F02D9
#define xK_1Fh		0x8D2A4C8A
#define xK_20h		0xFFFA3942
#define xK_21h		0x8771F681
#define xK_22h		0x6D9D6122
#define xK_23h		0xFDE5380C
#define xK_24h		0xA4BEEA44
#define xK_25h		0x4BDECFA9
#define xK_26h		0xF6BB4B60
#define xK_27h		0xBEBFBC70
#define xK_28h		0x289B7EC6
#define xK_29h		0xEAA127FA
#define xK_2Ah		0xD4EF3085
#define xK_2Bh		0x04881D05
#define xK_2Ch		0xD9D4D039
#define xK_2Dh		0xE6DB99E5
#define xK_2Eh		0x1FA27CF8
#define xK_2Fh		0xC4AC5665
#define xK_30h		0xF4292244
#define xK_31h		0x432AFF97
#define xK_32h		0xAB9423A7
#define xK_33h		0xFC93A039
#define xK_34h		0x655B59C3
#define xK_35h		0x8F0CCC92
#define xK_36h		0xFFEFF47D
#define xK_37h		0x85845DD1
#define xK_38h		0x6FA87E4F
#define xK_39h		0xFE2CE6E0
#define xK_3Ah		0xA3014314
#define xK_3Bh		0x4E0811A1
#define xK_3Ch		0xF7537E82
#define xK_3Dh		0xBD3AF235
#define xK_3Eh		0x2AD7D2BB
#define xK_3Fh		0xEB86D391

#define R(index)	xR_##index
#define K(index)	xK_##index

#define xACCUMULATE(index, rega, regb, regc, regd, regf, g, regtmp, regw)					\
	__asm add rega, dword ptr[regw + (g) * 4]												\
	__asm add rega, K(index)																\
	__asm add rega, regf																	\
	__asm rol rega, R(index)																\
	__asm add rega, regb

#define MD5_STEP_0(index, rega, regb, regc, regd, regf, regtmp, regw)						\
	__asm mov regf, regd																	\
	__asm xor regf, regc																	\
	__asm and regf, regb																	\
	__asm xor regf, regd																	\
	xACCUMULATE(index, rega, regb, regc, regd, regf, index, regtmp, regw)

#define MD5_STEP_1(index, rega, regb, regc, regd, regf, regtmp, regw)						\
	__asm mov regf, regc																	\
	__asm xor regf, regb																	\
	__asm and regf, regd																	\
	__asm xor regf, regc																	\
	xACCUMULATE(index, rega, regb, regc, regd, regf, (index * 5 + 1) % 16, regtmp, regw)

#define MD5_STEP_2(index, rega, regb, regc, regd, regf, regtmp, regw)						\
	__asm mov regf, regb																	\
	__asm xor regf, regc																	\
	__asm xor regf, regd																	\
	xACCUMULATE(index, rega, regb, regc, regd, regf, (index * 3 + 5) % 16, regtmp, regw)

#define MD5_STEP_3(index, rega, regb, regc, regd, regf, regtmp, regw)						\
	__asm mov regf, regd																	\
	__asm not regf																			\
	__asm or regf, regb																		\
	__asm xor regf, regc																	\
	xACCUMULATE(index, rega, regb, regc, regd, regf, (index * 7) % 16, regtmp, regw)

	MD5_STEP_0( 0h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 1h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 2h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 3h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 4h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 5h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 6h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 7h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 8h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_0( 9h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_0(0Ah, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_0(0Bh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_0(0Ch, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_0(0Dh, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_0(0Eh, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_0(0Fh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)

	MD5_STEP_1(10h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(11h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(12h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(13h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(14h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(15h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(16h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(17h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(18h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(19h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(1Ah, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(1Bh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(1Ch, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(1Dh, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(1Eh, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_1(1Fh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)

	MD5_STEP_2(20h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(21h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(22h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(23h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(24h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(25h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(26h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(27h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(28h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(29h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(2Ah, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(2Bh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(2Ch, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(2Dh, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(2Eh, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_2(2Fh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)

	MD5_STEP_3(30h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(31h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(32h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(33h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(34h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(35h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(36h, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(37h, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(38h, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(39h, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(3Ah, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(3Bh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(3Ch, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(3Dh, REG_D, REG_A, REG_B, REG_C, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(3Eh, REG_C, REG_D, REG_A, REG_B, REG_F, REG_TMP, REG_W)
	MD5_STEP_3(3Fh, REG_B, REG_C, REG_D, REG_A, REG_F, REG_TMP, REG_W)

	__asm {
		mov REG_TMP, dword ptr[LPDIGEST]
		add dword ptr[REG_TMP], REG_A
		add dword ptr[REG_TMP + 4h], REG_B
		add dword ptr[REG_TMP + 8h], REG_C
		add dword ptr[REG_TMP + 0Ch], REG_D

		pop edi
		pop esi
		pop ebp
		pop ebx
		ret 8			// naked, __stdcall
	}
}

// 其他非静态成员函数。
void MD5Hasher::xInitialize(void *pOutput){
	DWORD (&adwDigest)[4] = *(DWORD (*)[4])pOutput;

	adwDigest[0] = 0x67452301;
	adwDigest[1] = 0xEFCDAB89;
	adwDigest[2] = 0x98BADCFE;
	adwDigest[3] = 0x10325476;

	xm_uBytesInLastChunk = 0;
	xm_uBytesTotal = 0;
}
void MD5Hasher::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	DWORD (&adwDigest)[4] = *(DWORD (*)[4])pOutput;

	const BYTE *pbyCur = pbyData;
	std::size_t uBytesRemaining = uSize;
	const std::size_t uBytesFree = sizeof(xm_abyLastChunk) - xm_uBytesInLastChunk;
	if(uBytesRemaining >= uBytesFree){
		if(xm_uBytesInLastChunk != 0){
			std::memcpy(xm_abyLastChunk + (std::ptrdiff_t)xm_uBytesInLastChunk, pbyCur, uBytesFree);
			xm_uBytesInLastChunk = 0;

			xHashChunk(&adwDigest, (const BYTE (*)[64])xm_abyLastChunk);
			pbyCur += (std::ptrdiff_t)uBytesFree;
			uBytesRemaining -= uBytesFree;
		}
		while(uBytesRemaining >= sizeof(xm_abyLastChunk)){
			xHashChunk(&adwDigest, (const BYTE (*)[64])pbyCur);
			pbyCur += (std::ptrdiff_t)sizeof(xm_abyLastChunk);
			uBytesRemaining -= sizeof(xm_abyLastChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(xm_abyLastChunk + (std::ptrdiff_t)xm_uBytesInLastChunk, pbyCur, uBytesRemaining);
		xm_uBytesInLastChunk = uBytesRemaining;
	}

	xm_uBytesTotal += uSize;
}
void MD5Hasher::xFinalize(void *pOutput){
	DWORD (&adwDigest)[4] = *(DWORD (*)[4])pOutput;

	xm_abyLastChunk[xm_uBytesInLastChunk++] = 0x80;
	if(xm_uBytesInLastChunk == sizeof(xm_abyLastChunk)){
		xm_uBytesInLastChunk = 0;
	} else if(xm_uBytesInLastChunk > sizeof(xm_abyLastChunk) - sizeof(std::uint64_t)){
		std::memset(xm_abyLastChunk + (std::ptrdiff_t)xm_uBytesInLastChunk, 0, sizeof(xm_abyLastChunk) - xm_uBytesInLastChunk);
		xHashChunk(&adwDigest, &xm_abyLastChunk);
		xm_uBytesInLastChunk = 0;
	}
	if(xm_uBytesInLastChunk < sizeof(xm_abyLastChunk) - sizeof(std::uint64_t)){
		std::memset(xm_abyLastChunk + (std::ptrdiff_t)xm_uBytesInLastChunk, 0, sizeof(xm_abyLastChunk) - sizeof(std::uint64_t) - xm_uBytesInLastChunk);
	}
	*(std::uint64_t *)(xm_abyLastChunk + (sizeof(xm_abyLastChunk) - sizeof(std::uint64_t))) = xm_uBytesTotal * CHAR_BIT;
	xHashChunk(&adwDigest, &xm_abyLastChunk);
}

void MD5Hasher::Transform(BYTE (*pardwOutput)[16], const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	AbstractTransformerBaseClass::xTransform((DWORD (*)[4])pardwOutput, pbyData, uSize, bIsEndOfStream);
}
