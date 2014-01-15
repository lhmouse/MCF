// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include <map>
#include <memory>
#include "CRC32.hpp"
using namespace MCF;

// 静态成员函数。
__declspec(naked) void __fastcall CRC32Hasher::xBuildTable(DWORD (* /* pardwTable */)[0x100], DWORD /* dwDivisor */){
	// 参考文献：
	// http://www.relisoft.com/science/CrcOptim.html
	//   [注1] 原文提供的是正序（权较大位向权较小位方向）的 CRC 计算，而这里使用的是反序（权较小位向权较大位方向）。
	//   [注2] 原文的 CRC 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。

/*
	for(DWORD i = 0; i < 256; ++i){
		DWORD dwReg = i;
		for(std::size_t j = 0; j < 8; ++j){
			const bool bLowerBit = (dwReg & 1) != 0;
			dwReg >>= 1;
			if(bLowerBit){
				dwReg ^= dwDivisor;
			}
		}
		(*pardwTable)[i] = dwReg;
	}
*/

#define		CALC_ELEMENT_STEP(reg, divisor, tmp)	\
	__asm shr reg, 1								\
	__asm sbb tmp, tmp								\
	__asm and tmp, divisor							\
	__asm xor reg, tmp

	__asm {
		push ebx
		push esi
		push edi

		xor esi, esi
		mov ebx, -100h
		xor eax, eax
		jmp xfill_loop
		align 16

	xfill_loop:
	}

	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)
	CALC_ELEMENT_STEP(eax, edx, edi)

	__asm {
		inc ebx
		mov dword ptr[ecx + ebx * 4 - 4h + 400h], eax
		mov eax, esi
		mov al, bl
		jnz xfill_loop

		pop edi
		pop esi
		pop ebx
		ret				// __fastcall
	}
}

// 构造函数和析构函数。
CRC32Hasher::CRC32Hasher(DWORD dwDivisor){
	xBuildTable(&xm_ardwTable, dwDivisor);
}

// 其他非静态成员函数。
void CRC32Hasher::xInitialize(void *pOutput){
	*(DWORD *)pOutput = 0xFFFFFFFF;
}
void CRC32Hasher::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	DWORD dwReg = *(DWORD *)pOutput;

	const BYTE *pbyRead = pbyData;
	const BYTE *const pbyEnd = pbyRead + (std::ptrdiff_t)uSize;

	// 注意下面的汇编代码是一个 do ... while 循环，必须确保对于所有情况，第一个对齐点和结束点之间至少有一个 DWORD。
	if((std::size_t)(pbyEnd - pbyRead) >= sizeof(DWORD) * 2){
		const BYTE *const pbyFirstAlignedPoint = (const BYTE *)(((std::uintptr_t)(pbyRead + 3)) & -4);
		while(pbyRead < pbyFirstAlignedPoint){
			dwReg = xm_ardwTable[(std::size_t)((dwReg & 0xFF) ^ *(pbyRead++))] ^ (dwReg >> 8);
		}

		__asm {
			push ebx
			push esi
			push edi

			mov ebx, dword ptr[this]
			lea ebx, dword ptr[ebx].xm_ardwTable
			mov esi, dword ptr[pbyRead]
			mov edi, dword ptr[pbyEnd]
			and edi, -4
			mov edx, dword ptr[dwReg]
			jmp xcrc_loop
			align 16

		xcrc_loop:
			lodsd

			movzx ecx, al
			xor cl, dl
			shr edx, 8
			xor edx, dword ptr[ebx + ecx * 4]
			xor ecx, ecx
			movzx ecx, ah
			xor cl, dl
			shr edx, 8
			xor edx, dword ptr[ebx + ecx * 4]

			shr eax, 16

			movzx ecx, al
			mov cl, al
			xor cl, dl
			shr edx, 8
			xor edx, dword ptr[ebx + ecx * 4]
			movzx ecx, ah
			xor cl, dl
			shr edx, 8
			xor edx, dword ptr[ebx + ecx * 4]

			cmp esi, edi
			jnz xcrc_loop

			mov dword ptr[dwReg], edx
			mov dword ptr[pbyRead], esi

			pop edi
			pop esi
			pop ebx
		}
	}
	while(pbyRead < pbyEnd){
		dwReg = xm_ardwTable[(std::size_t)((dwReg & 0xFF) ^ *(pbyRead++))] ^ (dwReg >> 8);
	}

	*(DWORD *)pOutput = dwReg;
}
void CRC32Hasher::xFinalize(void *pOutput){
	*(DWORD *)pOutput = ~*(DWORD *)pOutput;
}

void CRC32Hasher::Transform(DWORD *pdwOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	AbstractTransformerBaseClass::xTransform(pdwOutput, pbyData, uSize, bIsEndOfStream);
}
