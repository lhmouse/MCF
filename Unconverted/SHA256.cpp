// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "SHA256.hpp"
using namespace MCF;

// 静态成员函数。
__declspec(naked) void __fastcall SHA256Hasher::xHashChunk(DWORD (* /* pardwOutput */)[8], const BYTE (* /* pabyChunk */)[64]){
	// 参考文献：
	// 1. https://en.wikipedia.org/wiki/SHA-2
	// 2. http://download.intel.com/embedded/processor/whitepaper/327457.pdf

/*
	static const DWORD KVEC[64] = {
		0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
		0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
		0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
		0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
		0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
		0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
		0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
		0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
	};

	DWORD w[64];

	for(std::size_t i = 0; i < 16; ++i){
		// 先将 chunk 按照大端格式转换成 DWORD。
		w[i] = ::_byteswap_ulong(((const DWORD *)(*pabyChunk))[i]);
	}
	for(std::size_t i = 16; i < COUNTOF(w); ++i){
		//const DWORD s0 = ::_lrotr(w[i - 15], 7) ^ ::_lrotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
		const DWORD s0 = ::_lrotr((::_lrotr(w[i - 15], 11) ^ w[i - 15]), 7) ^ (w[i - 15] >> 3);
		//const DWORD s1 = ::_lrotr(w[i - 2], 17) ^ ::_lrotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
		const DWORD s1 = ::_lrotr((::_lrotr(w[i - 2], 2) ^ w[i - 2]), 17) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + w[i - 7] + s0 + s1;
	}

	DWORD tmp[8];
	std::memcpy(tmp, *pardwOutput, sizeof(tmp));

	for(std::size_t i = 0; i < 64; ++i){
		//const DWORD s0 = ::_lrotr(tmp[0], 2) ^ ::_lrotr(tmp[0], 13) ^ ::_lrotr(tmp[0], 22);
		const DWORD s0 = ::_lrotr(::_lrotr(::_lrotr(tmp[0], 9) ^ tmp[0], 11) ^ tmp[0], 2);
		//const DWORD maj = (tmp[0] & tmp[1]) ^ (tmp[0] & tmp[2]) ^ (tmp[1] & tmp[2]);
		const DWORD maj = (tmp[0] & tmp[1]) | (tmp[2] & (tmp[0] ^ tmp[1]));
		const DWORD t2 = s0 + maj;
		//const DWORD s1 = ::_lrotr(tmp[4], 6) ^ ::_lrotr(tmp[4], 11) ^ ::_lrotr(tmp[4], 25);
		const DWORD s1 = ::_lrotr(::_lrotr(::_lrotr(tmp[4], 14) ^ tmp[4], 5) ^ tmp[4], 6);
		//const DWORD ch = (tmp[4] & tmp[5]) ^ (~tmp[4] & tmp[6]);
		const DWORD ch = tmp[6] ^ (tmp[4] & (tmp[5] ^ tmp[6]));
		const DWORD t1 = tmp[7] + s1 + ch + KVEC[i] + w[i];

		tmp[7] = tmp[6];
		tmp[6] = tmp[5];
		tmp[5] = tmp[4];
		tmp[4] = tmp[3] + t1;
		tmp[3] = tmp[2];
		tmp[2] = tmp[1];
		tmp[1] = tmp[0];
		tmp[0] = t1 + t2;
	}

	for(std::size_t i = 0; i < COUNTOF(*pardwOutput); ++i){
		(*pardwOutput)[i] += tmp[i];
	}
*/

	__asm {
		push ebp
		mov ebp, esp
		and esp, -10h
		push edx
		push ecx
		sub esp, 68h

		push ebx
		push ebp
		push esi
		push edi
	}
	// esp 现在对齐到 16 字节，任何与 esp 偏移量为 16 字节整数倍的地址也是对齐到 16 字节的。

	// esp	->		<old edi>
	//		+  4h	<old esi>
	//		+  8h	<old ebp>
	//		+ 0Ch	<old ebx>
	//		+ 10h	w[0]
	//		+ 14h	w[1]
	//		 ...	 ...
	//		+ 4Ch	w[15]
	//		+ 50h	tmp[0]
	//		+ 54h	tmp[1]
	//		 ...	 ...
	//		+ 6Ch	tmp[7]
	//		+ 70h	<padding>
	//		+ 74h	<padding>
	//		+ 78h	pDigest
	//		+ 7Ch	pabyChunk
	//		+ 80h	<ret addr>

#define LPARDWOUTPUT				(esp + 78h)
#define LPARBYCHUNK					(esp + 7Ch)

#define xK_0_0		0428A2F98h
#define xK_0_1		071374491h
#define xK_0_2		0B5C0FBCFh
#define xK_0_3		0E9B5DBA5h
#define xK_4_0		03956C25Bh
#define xK_4_1		059F111F1h
#define xK_4_2		0923F82A4h
#define xK_4_3		0AB1C5ED5h
#define xK_8_0		0D807AA98h
#define xK_8_1		012835B01h
#define xK_8_2		0243185BEh
#define xK_8_3		0550C7DC3h
#define xK_12_0		072BE5D74h
#define xK_12_1		080DEB1FEh
#define xK_12_2		09BDC06A7h
#define xK_12_3		0C19BF174h
#define xK_16_0		0E49B69C1h
#define xK_16_1		0EFBE4786h
#define xK_16_2		00FC19DC6h
#define xK_16_3		0240CA1CCh
#define xK_20_0		02DE92C6Fh
#define xK_20_1		04A7484AAh
#define xK_20_2		05CB0A9DCh
#define xK_20_3		076F988DAh
#define xK_24_0		0983E5152h
#define xK_24_1		0A831C66Dh
#define xK_24_2		0B00327C8h
#define xK_24_3		0BF597FC7h
#define xK_28_0		0C6E00BF3h
#define xK_28_1		0D5A79147h
#define xK_28_2		006CA6351h
#define xK_28_3		014292967h
#define xK_32_0		027B70A85h
#define xK_32_1		02E1B2138h
#define xK_32_2		04D2C6DFCh
#define xK_32_3		053380D13h
#define xK_36_0		0650A7354h
#define xK_36_1		0766A0ABBh
#define xK_36_2		081C2C92Eh
#define xK_36_3		092722C85h
#define xK_40_0		0A2BFE8A1h
#define xK_40_1		0A81A664Bh
#define xK_40_2		0C24B8B70h
#define xK_40_3		0C76C51A3h
#define xK_44_0		0D192E819h
#define xK_44_1		0D6990624h
#define xK_44_2		0F40E3585h
#define xK_44_3		0106AA070h
#define xK_48_0		019A4C116h
#define xK_48_1		01E376C08h
#define xK_48_2		02748774Ch
#define xK_48_3		034B0BCB5h
#define xK_52_0		0391C0CB3h
#define xK_52_1		04ED8AA4Ah
#define xK_52_2		05B9CCA4Fh
#define xK_52_3		0682E6FF3h
#define xK_56_0		0748F82EEh
#define xK_56_1		078A5636Fh
#define xK_56_2		084C87814h
#define xK_56_3		08CC70208h
#define xK_60_0		090BEFFFAh
#define xK_60_1		0A4506CEBh
#define xK_60_2		0BEF9A3F7h
#define xK_60_3		0C67178F2h

#define K(base, offset)				xK_##base##_##offset
#define W(index)					(esp + 10h + ((index) % 16) * 4)
#define TMP(index)					(esp + 50h + ((index) % 8) * 4)

#define BSWAP_COPY(dst, regtmp, src)				\
	__asm mov regtmp, dword ptr[src]				\
	__asm bswap regtmp								\
	__asm mov dword ptr[dst], regtmp

	BSWAP_COPY(W( 0), eax, edx)
	BSWAP_COPY(W( 1), eax, edx +  4h)
	BSWAP_COPY(W( 2), eax, edx +  8h)
	BSWAP_COPY(W( 3), eax, edx + 0Ch)
	BSWAP_COPY(W( 4), eax, edx + 10h)
	BSWAP_COPY(W( 5), eax, edx + 14h)
	BSWAP_COPY(W( 6), eax, edx + 18h)
	BSWAP_COPY(W( 7), eax, edx + 1Ch)
	BSWAP_COPY(W( 8), eax, edx + 20h)
	BSWAP_COPY(W( 9), eax, edx + 24h)
	BSWAP_COPY(W(10), eax, edx + 28h)
	BSWAP_COPY(W(11), eax, edx + 2Ch)
	BSWAP_COPY(W(12), eax, edx + 30h)
	BSWAP_COPY(W(13), eax, edx + 34h)
	BSWAP_COPY(W(14), eax, edx + 38h)
	BSWAP_COPY(W(15), eax, edx + 3Ch)

#define DWORD_COPY(dst, regtmp, src)				\
	__asm mov regtmp, dword ptr[src]				\
	__asm mov dword ptr[dst], regtmp

	DWORD_COPY(TMP( 0), eax, ecx)
	DWORD_COPY(TMP( 1), eax, ecx +  4h)
	DWORD_COPY(TMP( 2), eax, ecx +  8h)
	DWORD_COPY(TMP( 3), eax, ecx + 0Ch)
	DWORD_COPY(TMP( 4), eax, ecx + 10h)
	DWORD_COPY(TMP( 5), eax, ecx + 14h)
	DWORD_COPY(TMP( 6), eax, ecx + 18h)
	DWORD_COPY(TMP( 7), eax, ecx + 1Ch)

#define NO_MIXEDIN_0(...)
#define NO_MIXEDIN_1(...)
#define NO_MIXEDIN_2(...)
#define NO_MIXEDIN_3(...)
#define NO_MIXEDIN_4(...)
#define NO_MIXEDIN_5(...)

// 注意这个宏有一个出入参和一个出参。outaddr 指向的内存地址必须是对齐到段的。
// 宏执行完毕后 outaddr 指向的缓冲区和 src_inout 会得到同样的结果。
//#define CALC_W_QUAD(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)
#define CALC_W_QUAD_MIXEDIN_A_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm movdqa xmm6, rxmm3												\
	__asm pshufd xmm4, rxmm0_inout, 10100101b
#define CALC_W_QUAD_MIXEDIN_A_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm movdqa xmm5, xmm4													\
	__asm punpckhdq xmm6, xmm6												\
	__asm movdqa xmm7, xmm6
#define CALC_W_QUAD_MIXEDIN_A_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm psrlq xmm4, 11													\
	__asm psrlq xmm6, 2
#define CALC_W_QUAD_MIXEDIN_A_3(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pxor xmm4, xmm5													\
	__asm pxor xmm6, xmm7
#define CALC_W_QUAD_MIXEDIN_A_4(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm psrlq xmm4, 7														\
	__asm psrlq xmm6, 17
#define CALC_W_QUAD_MIXEDIN_A_5(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm psrld xmm5, 3														\
	__asm psrld xmm7, 10

#define CALC_W_QUAD_MIXEDIN_B_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pxor xmm4, xmm5													\
	__asm pxor xmm6, xmm7
#define CALC_W_QUAD_MIXEDIN_B_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pshufd xmm5, xmm4, 10001000b										\
	__asm movdqa xmm4, rxmm2
#define CALC_W_QUAD_MIXEDIN_B_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pshufd xmm7, xmm6, 10001000b										\
	__asm psrldq xmm4, 4
#define CALC_W_QUAD_MIXEDIN_B_3(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm paddd xmm7, xmm5													\
	__asm paddd xmm4, rxmm0_inout											\
	__asm paddd xmm7, xmm4
#define CALC_W_QUAD_MIXEDIN_B_4(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pshufd xmm4, rxmm0_inout, 11111111b								\
	__asm pshufd xmm5, rxmm1, 00000000b										\
	__asm movdqa xmmword ptr[memout], xmm7
#define CALC_W_QUAD_MIXEDIN_B_5(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm punpckhqdq xmm4, xmm5												\
	__asm punpckldq xmm7, xmm7

#define CALC_W_QUAD_MIXEDIN_C_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm movdqa xmm5, xmm4													\
	__asm movdqa xmm6, xmm7
#define CALC_W_QUAD_MIXEDIN_C_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm psrlq xmm4, 11													\
	__asm psrlq xmm6, 2
#define CALC_W_QUAD_MIXEDIN_C_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pxor xmm4, xmm5													\
	__asm psrlq xmm4, 7
#define CALC_W_QUAD_MIXEDIN_C_3(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pxor xmm6, xmm7													\
	__asm psrlq xmm6, 17
#define CALC_W_QUAD_MIXEDIN_C_4(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm psrld xmm5, 3														\
	__asm psrld xmm7, 10
#define CALC_W_QUAD_MIXEDIN_C_5(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pxor xmm4, xmm5													\
	__asm pxor xmm6, xmm7													\
	__asm psrldq rxmm0_inout, 8

#define CALC_W_QUAD_MIXEDIN_D_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pshufd xmm5, xmm4, 10001000b										\
	__asm movdqa xmm4, rxmm2
#define CALC_W_QUAD_MIXEDIN_D_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pshufd xmm7, xmm6, 10001000b										\
	__asm psrldq xmm4, 12													\
	__asm movdqa xmm6, rxmm3
#define CALC_W_QUAD_MIXEDIN_D_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm paddd xmm7, xmm5													\
	__asm paddd xmm4, rxmm0_inout
#define CALC_W_QUAD_MIXEDIN_D_3(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm pslldq xmm6, 4													\
	__asm movq rxmm0_inout, qword ptr[memout]
#define CALC_W_QUAD_MIXEDIN_D_4(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm paddd xmm7, xmm6													\
	__asm paddd xmm7, xmm4
#define CALC_W_QUAD_MIXEDIN_D_5(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)	\
	__asm punpcklqdq rxmm0_inout, xmm7										\
	__asm movdqa xmmword ptr[memout], rxmm0_inout

#define RXMM0_FOR_STEP0		xmm0
#define RXMM1_FOR_STEP0		xmm1
#define RXMM2_FOR_STEP0		xmm2
#define RXMM3_FOR_STEP0		xmm3

#define RXMM0_FOR_STEP1		RXMM1_FOR_STEP0
#define RXMM1_FOR_STEP1		RXMM2_FOR_STEP0
#define RXMM2_FOR_STEP1		RXMM3_FOR_STEP0
#define RXMM3_FOR_STEP1		RXMM0_FOR_STEP0

#define RXMM0_FOR_STEP2		RXMM2_FOR_STEP0
#define RXMM1_FOR_STEP2		RXMM3_FOR_STEP0
#define RXMM2_FOR_STEP2		RXMM0_FOR_STEP0
#define RXMM3_FOR_STEP2		RXMM1_FOR_STEP0

#define RXMM0_FOR_STEP3		RXMM3_FOR_STEP0
#define RXMM1_FOR_STEP3		RXMM0_FOR_STEP0
#define RXMM2_FOR_STEP3		RXMM1_FOR_STEP0
#define RXMM3_FOR_STEP3		RXMM2_FOR_STEP0

#define RXMM0(index)		RXMM0_FOR_STEP##index
#define RXMM1(index)		RXMM1_FOR_STEP##index
#define RXMM2(index)		RXMM2_FOR_STEP##index
#define RXMM3(index)		RXMM3_FOR_STEP##index

// 首次使用之前需要令 eax = tmp0，ebp = tmp4。
// 此后只要没有改变这两个寄存器的值，就可以直接再次使用 SHA256_FAST_STEP，省去一些读内存的操作。
// *** 重要：在你调整这里的指令顺序之后请仔细测试一下运行速度，不要想当然。 ***
#define SHA256_FAST_STEP(tmp0, tmp1, tmp2, tmp3_inout, tmp4, tmp5, tmp6, tmp7_inout, k, w, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)	\
	__asm mov esi, ebp																															\
	__asm mov ecx, eax																															\
	mixedin_simd_prefix##_0(arg0, arg1, arg2, arg3, arg4)																						\
	__asm mov ebx, dword ptr[tmp1]																												\
	__asm ror ebp, 14																															\
	__asm mov edx, eax																															\
	__asm xor ebp, esi																															\
	__asm mov edi, dword ptr[tmp6]																												\
	mixedin_simd_prefix##_1(arg0, arg1, arg2, arg3, arg4)																						\
	__asm ror eax, 9																															\
	__asm xor eax, ecx																															\
	__asm ror eax, 11																															\
	__asm xor eax, ecx																															\
	mixedin_simd_prefix##_2(arg0, arg1, arg2, arg3, arg4)																						\
	__asm xor edx, ebx																															\
	__asm and edx, dword ptr[tmp2]																												\
	__asm and ecx, ebx																															\
	__asm or ecx, edx																															\
	__asm ror ebp, 5																															\
	__asm xor ebp, esi																															\
	__asm ror ebp, 6																															\
	__asm mov ebx, dword ptr[tmp5]																												\
	mixedin_simd_prefix##_3(arg0, arg1, arg2, arg3, arg4)																						\
	__asm mov edx, dword ptr[tmp7_inout]																										\
	__asm add edx, dword ptr[w]																													\
	__asm add edx, k																															\
	__asm ror eax, 2																															\
	__asm add eax, ecx																															\
	__asm xor ebx, edi																															\
	__asm and ebx, esi																															\
	__asm xor ebx, edi																															\
	mixedin_simd_prefix##_4(arg0, arg1, arg2, arg3, arg4)																						\
	__asm add edx, ebp																															\
	__asm add edx, ebx																															\
	__asm add eax, edx																															\
	__asm mov ebp, dword ptr[tmp3_inout]																										\
	__asm mov edi, esi																															\
	mixedin_simd_prefix##_5(arg0, arg1, arg2, arg3, arg4)																						\
	__asm mov dword ptr[tmp7_inout], eax																										\
	__asm add ebp, edx																															\
	__asm mov dword ptr[tmp3_inout], ebp

#define SHA256_FAST_STEP_QUAD(n)																																\
	SHA256_FAST_STEP(TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), TMP(n + 5), TMP(n + 6), TMP(n + 7), K(n, 0), W(n    ), NO_MIXEDIN, , , , , )	\
	SHA256_FAST_STEP(TMP(n + 7), TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), TMP(n + 5), TMP(n + 6), K(n, 1), W(n + 1), NO_MIXEDIN, , , , , )	\
	SHA256_FAST_STEP(TMP(n + 6), TMP(n + 7), TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), TMP(n + 5), K(n, 2), W(n + 2), NO_MIXEDIN, , , , , )	\
	SHA256_FAST_STEP(TMP(n + 5), TMP(n + 6), TMP(n + 7), TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), K(n, 3), W(n + 3), NO_MIXEDIN, , , , , )

#define SHA256_FAST_STEP_QUAD_WITH_CALC_W(n, w_n)																																											\
	SHA256_FAST_STEP(TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), TMP(n + 5), TMP(n + 6), TMP(n + 7), K(n, 0), W(n    ), CALC_W_QUAD_MIXEDIN_A, RXMM0(w_n), RXMM1(w_n), RXMM2(w_n), RXMM3(w_n), W(w_n * 4))	\
	SHA256_FAST_STEP(TMP(n + 7), TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), TMP(n + 5), TMP(n + 6), K(n, 1), W(n + 1), CALC_W_QUAD_MIXEDIN_B, RXMM0(w_n), RXMM1(w_n), RXMM2(w_n), RXMM3(w_n), W(w_n * 4))	\
	SHA256_FAST_STEP(TMP(n + 6), TMP(n + 7), TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), TMP(n + 5), K(n, 2), W(n + 2), CALC_W_QUAD_MIXEDIN_C, RXMM0(w_n), RXMM1(w_n), RXMM2(w_n), RXMM3(w_n), W(w_n * 4))	\
	SHA256_FAST_STEP(TMP(n + 5), TMP(n + 6), TMP(n + 7), TMP(n    ), TMP(n + 1), TMP(n + 2), TMP(n + 3), TMP(n + 4), K(n, 3), W(n + 3), CALC_W_QUAD_MIXEDIN_D, RXMM0(w_n), RXMM1(w_n), RXMM2(w_n), RXMM3(w_n), W(w_n * 4))

	__asm {
		// xmm0 =  w[3] :  w[2] :  w[1] :  w[0]
		// xmm1 =  w[7] :  w[6] :  w[5] :  w[4]
		// xmm2 = w[11] : w[10] :  w[9] :  w[8]
		// xmm3 = w[15] : w[14] : w[13] : w[12]
		movdqa xmm0, xmmword ptr[W(0)]
		movdqa xmm1, xmmword ptr[W(4)]
		movdqa xmm2, xmmword ptr[W(8)]
		movdqa xmm3, xmmword ptr[W(12)]

		// 参见宏 SHA256_FAST_STEP() 上面的注释。
		mov eax, dword ptr[TMP(0)]
		mov ebp, dword ptr[TMP(4)]
	}

	SHA256_FAST_STEP_QUAD				( 0)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	( 4, 0)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	( 8, 1)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(12, 2)

	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(16, 3)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(20, 0)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(24, 1)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(28, 2)

	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(32, 3)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(36, 0)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(40, 1)
	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(44, 2)

	SHA256_FAST_STEP_QUAD_WITH_CALC_W	(48, 3)
	SHA256_FAST_STEP_QUAD				(52)
	SHA256_FAST_STEP_QUAD				(56)
	SHA256_FAST_STEP_QUAD				(60)

#define ADD_MEMORY_DWORD(dst, regtmp, src)			\
	__asm mov regtmp, dword ptr[dst]				\
	__asm add regtmp, dword ptr[src]				\
	__asm mov dword ptr[dst], regtmp

	__asm mov esi, dword ptr[LPARDWOUTPUT]

	ADD_MEMORY_DWORD(esi, eax, TMP(0))
	ADD_MEMORY_DWORD(esi +  4h, eax, TMP(1))
	ADD_MEMORY_DWORD(esi +  8h, eax, TMP(2))
	ADD_MEMORY_DWORD(esi + 0Ch, eax, TMP(3))
	ADD_MEMORY_DWORD(esi + 10h, eax, TMP(4))
	ADD_MEMORY_DWORD(esi + 14h, eax, TMP(5))
	ADD_MEMORY_DWORD(esi + 18h, eax, TMP(6))
	ADD_MEMORY_DWORD(esi + 1Ch, eax, TMP(7))

	__asm {
		pop edi
		pop esi
		pop ebp
		pop ebx
		mov esp, ebp
		pop ebp
		ret				// naked, __fastcall
	}
}

// 其他非静态成员函数。
void SHA256Hasher::xInitialize(void *pOutput){
	DWORD (&adwDigest)[8] = *(DWORD (*)[8])pOutput;

	adwDigest[0] = 0x6A09E667u;
	adwDigest[1] = 0xBB67AE85u;
	adwDigest[2] = 0x3C6EF372u;
	adwDigest[3] = 0xA54FF53Au;
	adwDigest[4] = 0x510E527Fu;
	adwDigest[5] = 0x9B05688Cu;
	adwDigest[6] = 0x1F83D9ABu;
	adwDigest[7] = 0x5BE0CD19u;

	xm_uBytesInLastChunk = 0;
	xm_uBytesTotal = 0;
}
void SHA256Hasher::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	DWORD (&adwDigest)[8] = *(DWORD (*)[8])pOutput;

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
void SHA256Hasher::xFinalize(void *pOutput){
	DWORD (&adwDigest)[8] = *(DWORD (*)[8])pOutput;

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
	*(std::uint64_t *)(xm_abyLastChunk + (sizeof(xm_abyLastChunk) - sizeof(std::uint64_t))) = ::_byteswap_uint64(xm_uBytesTotal * CHAR_BIT);
	xHashChunk(&adwDigest, &xm_abyLastChunk);

	for(std::size_t i = 0; i < COUNTOF(adwDigest); ++i){
		adwDigest[i] = ::_byteswap_ulong(adwDigest[i]);
	}
}

void SHA256Hasher::Transform(BYTE (*pardwOutput)[32], const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	AbstractTransformerBaseClass::xTransform((DWORD (*)[8])pardwOutput, pbyData, uSize, bIsEndOfStream);
}
