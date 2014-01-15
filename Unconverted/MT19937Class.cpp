// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "MT19937Class.hpp"
using namespace MCF;

// 参考文献：
//   http://en.wikipedia.org/wiki/Mersenne_twister

// 静态成员变量。
__declspec(align(16)) const DWORD MT19937Class::xGEN_FLIP_MASK[4]	= { 0x9908B0DF, 0x9908B0DF, 0x9908B0DF, 0x9908B0DF };
__declspec(align(16)) const DWORD MT19937Class::xEXTRACT_MASK1[4]	= { 0x9D2C5680, 0x9D2C5680, 0x9D2C5680, 0x9D2C5680 };
__declspec(align(16)) const DWORD MT19937Class::xEXTRACT_MASK2[4]	= { 0xEFC60000, 0xEFC60000, 0xEFC60000, 0xEFC60000 };

// 构造函数和析构函数。
MT19937Class::MT19937Class(DWORD dwSeed){
	Init(dwSeed);
}
MT19937Class::~MT19937Class(){
}

// 其他非静态成员函数。
__declspec(naked) void __thiscall MT19937Class::xGenNumbers(){
/*
	std::size_t i = 0;
	std::size_t j = 397;
	DWORD dwResult;

	while(i < COUNTOF(xm_ardwMT) - 397){
		const DWORD y = (xm_ardwMT[i] & (DWORD)0x80000000) | (xm_ardwMT[i + 1] & (DWORD)0x7FFFFFFF);
		xm_ardwMT[i] = xm_ardwMT[j] ^ (y >> 1);
		if(y % 2 != 0){
			xm_ardwMT[i] ^= 0x9908B0DF;
		}
		dwResult = xm_ardwMT[i];
		dwResult ^= dwResult >> 11;
		dwResult ^= (dwResult << 7) & 0x9D2C5680;
		dwResult ^= (dwResult << 15) & 0xEFC60000;
		dwResult ^= dwResult >> 18;
		xm_ardwResults[i] = dwResult;

		++i;
		++j;
	}

	j = 0;
	while(i < COUNTOF(xm_ardwMT) - 1){
		const DWORD y = (xm_ardwMT[i] & (DWORD)0x80000000) | (xm_ardwMT[i + 1] & (DWORD)0x7FFFFFFF);
		xm_ardwMT[i] = xm_ardwMT[j] ^ (y >> 1);
		if(y % 2 != 0){
			xm_ardwMT[i] ^= 0x9908B0DF;
		}
		dwResult = xm_ardwMT[i];
		dwResult ^= dwResult >> 11;
		dwResult ^= (dwResult << 7) & 0x9D2C5680;
		dwResult ^= (dwResult << 15) & 0xEFC60000;
		dwResult ^= dwResult >> 18;
		xm_ardwResults[i] = dwResult;

		++i;
		++j;
	}

	const DWORD y = (xm_ardwMT[i] & (DWORD)0x80000000) | (xm_ardwMT[0] & (DWORD)0x7FFFFFFF);
	xm_ardwMT[i] = xm_ardwMT[396] ^ (y >> 1);
	if(y % 2 != 0){
		xm_ardwMT[i] ^= 0x9908B0DF;
	}
	dwResult = xm_ardwMT[i];
	dwResult ^= dwResult >> 11;
	dwResult ^= (dwResult << 7) & 0x9D2C5680;
	dwResult ^= (dwResult << 15) & 0xEFC60000;
	dwResult ^= dwResult >> 18;
	xm_ardwResults[i] = dwResult;
*/

	__asm {
				xor eax, eax		// i

				movdqa xmm0, xmmword ptr[xGEN_FLIP_MASK]
	}

#define GEN_NUMBER_QUAD(xmm_i_cur, xmm_i_next, xmm_j_cur, xmm_j_next, wr_offset)	\
	__asm movdqa xmm5, xmm_i_cur													\
	__asm movdqa xmm6, xmm_i_next													\
	__asm psrldq xmm5, 4															\
	__asm pslldq xmm6, 12															\
	__asm por xmm5, xmm6															\
	__asm pxor xmm5, xmm_i_cur														\
	__asm pslld xmm5, 1																\
	__asm psrld xmm5, 1																\
	__asm pxor xmm5, xmm_i_cur														\
	__asm movdqa xmm6, xmm_j_cur													\
	__asm movdqa xmm7, xmm_j_next													\
	__asm psrldq xmm6, 4															\
	__asm pslldq xmm7, 12															\
	__asm por xmm7, xmm6															\
	__asm movdqa xmm6, xmm5															\
	__asm psrld xmm5, 1																\
	__asm pslld xmm6, 31															\
	__asm pxor xmm5, xmm7															\
	__asm pxor xmm7, xmm7															\
	__asm pcmpgtd xmm7, xmm6														\
	__asm movdqa xmm6, xmmword ptr[xEXTRACT_MASK1]									\
	__asm pand xmm7, xmm0															\
	__asm pxor xmm5, xmm7															\
	__asm movdqa xmm7, xmm5															\
	__asm movdqa xmmword ptr[ecx + (wr_offset)].xm_ardwMT, xmm5						\
	__asm psrld xmm5, 11															\
	__asm pxor xmm5, xmm7															\
	__asm movdqa xmm7, xmm5															\
	__asm pslld xmm5, 7																\
	__asm pand xmm5, xmm6															\
	__asm movdqa xmm6, xmmword ptr[xEXTRACT_MASK2]									\
	__asm pxor xmm5, xmm7															\
	__asm movdqa xmm7, xmm5															\
	__asm pslld xmm5, 15															\
	__asm pand xmm5, xmm6															\
	__asm pxor xmm5, xmm7															\
	__asm movdqa xmm7, xmm5															\
	__asm psrld xmm5, 18															\
	__asm pxor xmm5, xmm7															\
	__asm movdqa xmmword ptr[ecx + (wr_offset)].xm_ardwResults, xmm5

	__asm {
				movdqa xmm1, xmmword ptr[ecx].xm_ardwMT
				movdqa xmm3, xmmword ptr[ecx + 10h * 99].xm_ardwMT
				jmp xloop1
				align 16

	xloop1:
	}
	__asm		movdqa xmm2, xmmword ptr[ecx + eax + 10h].xm_ardwMT
	__asm		movdqa xmm4, xmmword ptr[ecx + eax + 10h + 10h * 99].xm_ardwMT
				GEN_NUMBER_QUAD(xmm1, xmm2, xmm3, xmm4, eax)
	__asm 		movdqa xmm1, xmmword ptr[ecx + eax + 20h].xm_ardwMT
	__asm 		movdqa xmm3, xmmword ptr[ecx + eax + 20h + 10h * 99].xm_ardwMT
				GEN_NUMBER_QUAD(xmm2, xmm1, xmm4, xmm3, eax + 10h)
	__asm {
				add eax, 20h
				cmp eax, 10h * 56
				jb xloop1

				jmp xloop2
				align 16

	xloop2:
	}
	__asm		movdqa xmm2, xmmword ptr[ecx + eax + 10h].xm_ardwMT
	__asm		movdqa xmm4, xmmword ptr[ecx + eax + 10h + 10h * 99 - 10h * 156].xm_ardwMT
				GEN_NUMBER_QUAD(xmm1, xmm2, xmm3, xmm4, eax)
	__asm 		movdqa xmm1, xmmword ptr[ecx + eax + 20h].xm_ardwMT
	__asm		movdqa xmm3, xmmword ptr[ecx + eax + 20h + 10h * 99 - 10h * 156].xm_ardwMT
				GEN_NUMBER_QUAD(xmm2, xmm1, xmm4, xmm3, eax + 10h)
	__asm {
				add eax, 20h
				cmp eax, 10h * 154
				jb xloop2
	}

	__asm		movdqa xmm2, xmmword ptr[ecx + 10h * 155].xm_ardwMT
	__asm		movdqa xmm4, xmmword ptr[ecx + 10h * 98].xm_ardwMT
				GEN_NUMBER_QUAD(xmm1, xmm2, xmm3, xmm4, 10h * 154)
	__asm		movdqa xmm1, xmmword ptr[ecx].xm_ardwMT
	__asm		movdqa xmm3, xmmword ptr[ecx + 10h * 99].xm_ardwMT
				GEN_NUMBER_QUAD(xmm2, xmm1, xmm4, xmm3, 10h * 155)

	__asm 		ret
}

__declspec(naked) void __fastcall MT19937Class::Init(DWORD /* dwSeed */){
/*
	DWORD dwLastMT = dwSeed;
	xm_ardwMT[0] = dwSeed;
	for(std::size_t i = 1; i < COUNTOF(xm_ardwMT); ++i){
		dwLastMT = (DWORD)(0x6C078965 * (dwLastMT ^ (dwLastMT >> 30))) + (DWORD)i;
		xm_ardwMT[i] = dwLastMT;
	}
	xm_uIndex = 0;
*/

	__asm {
				push ebx
				push esi
				push edi

				mov ebx, ecx				// this
				xor esi, esi				// i
				mov edi, 6C078965h

				mov ecx, 624 - 1			// loop count
				mov eax, edx				// dwLastMT
				mov dword ptr[ebx].xm_ardwMT, edx
				jmp xloop
				align 16

	xloop:		shr eax, 30
				xor eax, edx
				mul edi
				inc esi
				add eax, esi
				dec ecx
				mov dword ptr[ebx + esi * 4].xm_ardwMT, eax
				mov edx, eax
				jnz xloop

				mov dword ptr[ebx].xm_uIndex, ecx

				pop edi
				pop esi
				pop ebx
				ret							// __fastcall
	}
}
DWORD MT19937Class::Get(){
	if(xm_uIndex == 0){
		xGenNumbers();
	}

	const DWORD dwRet = xm_ardwResults[xm_uIndex];
	++xm_uIndex;
	xm_uIndex = (xm_uIndex < COUNTOF(xm_ardwMT)) ? xm_uIndex : 0;
	return dwRet;
}
