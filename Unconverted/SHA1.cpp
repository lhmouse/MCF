// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "SHA1.hpp"
using namespace MCF;

// 静态成员函数。
__declspec(naked) void __fastcall SHA1Hasher::xHashChunk(DWORD (* /* pardwOutput */)[5], const BYTE (* /* pabyChunk */)[64]){
	// 参考文献：
	// https://en.wikipedia.org/wiki/SHA1

/*
	DWORD w[80];

	for(std::size_t i = 0; i < 16; ++i){
		// 先将 chunk 按照大端格式转换成 DWORD。
		w[i] = ::_byteswap_ulong(((const DWORD *)(*pabyChunk))[i]);
	}
	for(std::size_t i = 16; i < COUNTOF(w); ++i){
		w[i] = ::_lrotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	}

	DWORD a = (*pardwOutput)[0];
	DWORD b = (*pardwOutput)[1];
	DWORD c = (*pardwOutput)[2];
	DWORD d = (*pardwOutput)[3];
	DWORD e = (*pardwOutput)[4];

	for(std::size_t i = 0; i < 80; ++i){
		DWORD f, k;

		switch(i / 20){
			case 0:
				//f = (b & c) | (~b & d);
				f = d ^ (b & (c ^ d));
				k = 0x5A827999;
				break;
			case 1:
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
				break;
			case 2:
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
				break;
			case 3:
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
				break;
		}

		const DWORD temp = ::_lrotl(a, 5) + f + e + k + w[i];
		e = d;
		d = c;
		c = ::_lrotl(b, 30);
		b = a;
		a = temp;
	}

	(*pardwOutput)[0] += a;
	(*pardwOutput)[1] += b;
	(*pardwOutput)[2] += c;
	(*pardwOutput)[3] += d;
	(*pardwOutput)[4] += e;
*/

	__asm {
		push ebp
		mov ebp, esp
		and esp, -10h
		push edx
		push ecx
		sub esp, 48h

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
	//		+ 50h	<padding>
	//		+ 54h	<padding>
	//		+ 58h	pDigest
	//		+ 5Ch	pabyChunk
	//		+ 60h	<ret addr>

#define LPDIGEST					(esp + 58h)
#define LPBYCHUNK					(esp + 5Ch)

#define REG_A						esi
#define REG_B						ebx
#define REG_C						ecx
#define REG_D						edx
#define REG_E						edi

#define REG_F						ebp
#define REG_TMP						eax

#define REG_XMM0					xmm0
#define REG_XMM1					xmm1
#define REG_XMM2					xmm2
#define REG_XMM3					xmm3

#define W(index)					(esp + 10h + ((index) % 16) * 4)

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

	__asm {
		mov REG_TMP, ecx
		mov REG_A, dword ptr[REG_TMP]
		mov REG_B, dword ptr[REG_TMP + 4h]
		mov REG_C, dword ptr[REG_TMP + 8h]
		mov REG_D, dword ptr[REG_TMP + 0Ch]
		mov REG_E, dword ptr[REG_TMP + 10h]
	}

#define NO_MIXEDIN_0(...)
#define NO_MIXEDIN_1(...)
#define NO_MIXEDIN_2(...)

// 注意这个宏有一个出入参和一个出参。outaddr 指向的内存地址必须是对齐到段的。
// 宏执行完毕后 outaddr 指向的缓冲区和 src_inout 会得到同样的结果。
//#define CALC_W_QUAD(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)
#define CALC_W_QUAD_MIXEDIN_A_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pshufd xmm4, rxmm3, 10100101b											\
	__asm pshufd xmm5, rxmm2, 01010000b
#define CALC_W_QUAD_MIXEDIN_A_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pshufd xmm6, rxmm0_inout, 11111010b									\
	__asm pshufd xmm7, rxmm0_inout, 01010000b
#define CALC_W_QUAD_MIXEDIN_A_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pxor xmm4, xmm5														\
	__asm pxor xmm4, xmm6
#define CALC_W_QUAD_MIXEDIN_B_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pxor xmm4, xmm7														\
	__asm psllq xmm4, 1
#define CALC_W_QUAD_MIXEDIN_B_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pshufd xmm4, xmm4, 11011101b											\
	__asm pshufd xmm6, xmm4, 00000000b
#define CALC_W_QUAD_MIXEDIN_B_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pshufd xmm5, rxmm3, 11111111b											\
	__asm pshufd xmm7, rxmm1, 01010000b
#define CALC_W_QUAD_MIXEDIN_C_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm punpcklqdq xmm5, xmm6													\
	__asm pshufd xmm6, rxmm2, 11111010b
#define CALC_W_QUAD_MIXEDIN_C_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pxor xmm5, xmm6														\
	__asm pxor xmm5, xmm7
#define CALC_W_QUAD_MIXEDIN_C_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm pshufd xmm7, rxmm0_inout, 11111010b									\
	__asm pxor xmm5, xmm7
#define CALC_W_QUAD_MIXEDIN_D_0(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm psllq xmm5, 1															\
	__asm pshufd xmm5, xmm5, 11011101b
#define CALC_W_QUAD_MIXEDIN_D_1(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm punpcklqdq xmm4, xmm5													\
	__asm movdqa xmmword ptr[memout], xmm4
#define CALC_W_QUAD_MIXEDIN_D_2(rxmm0_inout, rxmm1, rxmm2, rxmm3, memout)		\
	__asm movdqa rxmm0_inout, xmm4

#define ACCUMULATE(index, rega, regb, regc, regd, rege, regf, k, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)			\
	__asm mov regtmp, rega																												\
	__asm rol regtmp, 5																													\
	mixedin_simd_prefix##_1(arg0, arg1, arg2, arg3, arg4)																				\
	__asm add rege, dword ptr[W(index)]																									\
	__asm add rege, (k)																													\
	__asm add rege, regtmp																												\
	__asm add rege, regf																												\
	mixedin_simd_prefix##_2(arg0, arg1, arg2, arg3, arg4)																				\
	__asm rol regb, 30

#define SHA1_STEP_0(index, rega, regb, regc, regd, rege, regf, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)				\
	mixedin_simd_prefix##_0(arg0, arg1, arg2, arg3, arg4)																				\
	__asm mov regf, regd																												\
	__asm xor regf, regc																												\
	__asm and regf, regb																												\
	__asm xor regf, regd																												\
	ACCUMULATE(index, rega, regb, regc, regd, rege, regf, 5A827999h, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)

#define SHA1_STEP_1(index, rega, regb, regc, regd, rege, regf, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)				\
	mixedin_simd_prefix##_0(arg0, arg1, arg2, arg3, arg4)																				\
	__asm mov regf, regb																												\
	__asm xor regf, regc																												\
	__asm xor regf, regd																												\
	ACCUMULATE(index, rega, regb, regc, regd, rege, regf, 6ED9EBA1h, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)

#define SHA1_STEP_2(index, rega, regb, regc, regd, rege, regf, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)				\
	mixedin_simd_prefix##_0(arg0, arg1, arg2, arg3, arg4)																				\
	__asm mov regf, regb																												\
	__asm and regf, regc																												\
	__asm mov regtmp, regb																												\
	__asm and regtmp, regd																												\
	__asm or regf, regtmp																												\
	__asm mov regtmp, regc																												\
	__asm and regtmp, regd																												\
	__asm or regf, regtmp																												\
	ACCUMULATE(index, rega, regb, regc, regd, rege, regf, 8F1BBCDCh, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)

#define SHA1_STEP_3(index, rega, regb, regc, regd, rege, regf, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)				\
	mixedin_simd_prefix##_0(arg0, arg1, arg2, arg3, arg4)																				\
	__asm mov regf, regb																												\
	__asm xor regf, regc																												\
	__asm xor regf, regd																												\
	ACCUMULATE(index, rega, regb, regc, regd, rege, regf, 0CA62C1D6h, regtmp, mixedin_simd_prefix, arg0, arg1, arg2, arg3, arg4)

	__asm {
		// xmm0 =  w[3] :  w[2] :  w[1] :  w[0]
		// xmm1 =  w[7] :  w[6] :  w[5] :  w[4]
		// xmm2 = w[11] : w[10] :  w[9] :  w[8]
		// xmm3 = w[15] : w[14] : w[13] : w[12]
		movdqa REG_XMM0, xmmword ptr[W(0)]
		movdqa REG_XMM1, xmmword ptr[W(4)]
		movdqa REG_XMM2, xmmword ptr[W(8)]
		movdqa REG_XMM3, xmmword ptr[W(12)]
	}

	SHA1_STEP_0( 0, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_0( 1, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_0( 2, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_0( 3, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_0( 4, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(16))
	SHA1_STEP_0( 5, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(16))
	SHA1_STEP_0( 6, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(16))
	SHA1_STEP_0( 7, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(16))
	SHA1_STEP_0( 8, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(20))
	SHA1_STEP_0( 9, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(20))
	SHA1_STEP_0(10, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(20))
	SHA1_STEP_0(11, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(20))
	SHA1_STEP_0(12, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(24))
	SHA1_STEP_0(13, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(24))
	SHA1_STEP_0(14, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(24))
	SHA1_STEP_0(15, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(24))
	SHA1_STEP_0(16, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(28))
	SHA1_STEP_0(17, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(28))
	SHA1_STEP_0(18, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(28))
	SHA1_STEP_0(19, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(28))

	SHA1_STEP_1(20, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(32))
	SHA1_STEP_1(21, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(32))
	SHA1_STEP_1(22, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(32))
	SHA1_STEP_1(23, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(32))
	SHA1_STEP_1(24, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(36))
	SHA1_STEP_1(25, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(36))
	SHA1_STEP_1(26, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(36))
	SHA1_STEP_1(27, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(36))
	SHA1_STEP_1(28, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(40))
	SHA1_STEP_1(29, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(40))
	SHA1_STEP_1(30, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(40))
	SHA1_STEP_1(31, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(40))
	SHA1_STEP_1(32, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(44))
	SHA1_STEP_1(33, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(44))
	SHA1_STEP_1(34, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(44))
	SHA1_STEP_1(35, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(44))
	SHA1_STEP_1(36, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(48))
	SHA1_STEP_1(37, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(48))
	SHA1_STEP_1(38, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(48))
	SHA1_STEP_1(39, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(48))

	SHA1_STEP_2(40, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(52))
	SHA1_STEP_2(41, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(52))
	SHA1_STEP_2(42, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(52))
	SHA1_STEP_2(43, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(52))
	SHA1_STEP_2(44, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(56))
	SHA1_STEP_2(45, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(56))
	SHA1_STEP_2(46, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(56))
	SHA1_STEP_2(47, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(56))
	SHA1_STEP_2(48, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(60))
	SHA1_STEP_2(49, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(60))
	SHA1_STEP_2(50, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(60))
	SHA1_STEP_2(51, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(60))
	SHA1_STEP_2(52, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(64))
	SHA1_STEP_2(53, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(64))
	SHA1_STEP_2(54, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(64))
	SHA1_STEP_2(55, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, W(64))
	SHA1_STEP_2(56, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(68))
	SHA1_STEP_2(57, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(68))
	SHA1_STEP_2(58, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(68))
	SHA1_STEP_2(59, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM0, W(68))

	SHA1_STEP_3(60, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(72))
	SHA1_STEP_3(61, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(72))
	SHA1_STEP_3(62, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(72))
	SHA1_STEP_3(63, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM2, REG_XMM3, REG_XMM0, REG_XMM1, W(72))
	SHA1_STEP_3(64, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_A, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(76))
	SHA1_STEP_3(65, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_B, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(76))
	SHA1_STEP_3(66, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_C, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(76))
	SHA1_STEP_3(67, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, CALC_W_QUAD_MIXEDIN_D, REG_XMM3, REG_XMM0, REG_XMM1, REG_XMM2, W(76))
	SHA1_STEP_3(68, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(69, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(70, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(71, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(72, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(73, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(74, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(75, REG_A, REG_B, REG_C, REG_D, REG_E, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(76, REG_E, REG_A, REG_B, REG_C, REG_D, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(77, REG_D, REG_E, REG_A, REG_B, REG_C, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(78, REG_C, REG_D, REG_E, REG_A, REG_B, REG_F, REG_TMP, NO_MIXEDIN, , , , , )
	SHA1_STEP_3(79, REG_B, REG_C, REG_D, REG_E, REG_A, REG_F, REG_TMP, NO_MIXEDIN, , , , , )

	__asm {
		mov REG_TMP, dword ptr[LPDIGEST]
		add dword ptr[REG_TMP], REG_A
		add dword ptr[REG_TMP + 4h], REG_B
		add dword ptr[REG_TMP + 8h], REG_C
		add dword ptr[REG_TMP + 0Ch], REG_D
		add dword ptr[REG_TMP + 10h], REG_E

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
void SHA1Hasher::xInitialize(void *pOutput){
	DWORD (&adwDigest)[5] = *(DWORD (*)[5])pOutput;

	adwDigest[0] = 0x67452301;
	adwDigest[1] = 0xEFCDAB89;
	adwDigest[2] = 0x98BADCFE;
	adwDigest[3] = 0x10325476,
	adwDigest[4] = 0xC3D2E1F0;

	xm_uBytesInLastChunk = 0;
	xm_uBytesTotal = 0;
}
void SHA1Hasher::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	DWORD (&adwDigest)[5] = *(DWORD (*)[5])pOutput;

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
void SHA1Hasher::xFinalize(void *pOutput){
	DWORD (&adwDigest)[5] = *(DWORD (*)[5])pOutput;

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

void SHA1Hasher::Transform(BYTE (*pardwOutput)[20], const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	AbstractTransformerBaseClass::xTransform((DWORD (*)[5])pardwOutput, pbyData, uSize, bIsEndOfStream);
}
