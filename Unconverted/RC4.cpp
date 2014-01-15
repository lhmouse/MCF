// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "RC4.hpp"
using namespace MCF;

// 参考文献：
// http://en.wikipedia.org/wiki/RC4

// 其他非静态成员函数。
void RC4Encoder::xInitialize(void *pOutput){
	UNREF_PARAM(pOutput);

	for(std::size_t i = 0; i < 256; ++i){
		xm_abyBox[i] = (BYTE)(i & 0xFF);
	}

	std::size_t j = 0;
	std::size_t rem = 0;
	for(std::size_t i = 0; i < 256; ++i){
		// j = (j + xm_abyBox[i] + xm_pbyKey[i % xm_uKeyLen]) & 0xFF;
		j = (j + xm_abyBox[i] + xm_pbyKey[rem]) & 0xFF;
		if(++rem >= xm_uKeyLen){
			rem = 0;
		}
		std::swap(xm_abyBox[i], xm_abyBox[j]);
	}

	xm_uContextI = 0;
	xm_uContextJ = 0;
}

void RC4Encoder::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	xm_vecbyTemp.resize(uSize);
	BYTE *pbyWrite = xm_vecbyTemp.data();

/*
	const BYTE *const pbyEnd = pbyData + (std::ptrdiff_t)uSize;
	for(const BYTE *pbyCur = pbyData; pbyCur < pbyEnd; ++pbyCur){
		xm_uContextI = (xm_uContextI + 1) & 0xFF;
		xm_uContextJ = (xm_uContextJ + xm_abyBox[xm_uContextI]) & 0xFF;
		std::swap(xm_abyBox[xm_uContextI], xm_abyBox[xm_uContextJ]);

		*(pbyWrite++) = (BYTE)((*pbyCur ^ xm_abyBox[(xm_abyBox[xm_uContextI] + xm_abyBox[xm_uContextJ]) & 0xFF]) & 0xFF);
	}
*/

	__asm {
		mov eax, dword ptr[uSize]
		test eax, eax
		jz xno_data

		push ebx
		push ebp
		push esi
		push edi

		mov ebx, dword ptr[this]
		mov esi, dword ptr[pbyData]
		mov edi, dword ptr[pbyWrite]
		mov ebp, eax

		mov eax, dword ptr[ebx].xm_uContextI
		mov ecx, dword ptr[ebx].xm_uContextJ
		jmp xencode_loop
		align 16

	xencode_loop:
		inc eax
		and eax, 0FFh
		movzx edx, byte ptr[ebx + eax].xm_abyBox
		add ecx, edx
		and ecx, 0FFh

		mov dh, byte ptr[ebx + ecx].xm_abyBox
		inc esi
		mov byte ptr[ebx + eax].xm_abyBox, dh
		inc edi
		mov byte ptr[ebx + ecx].xm_abyBox, dl

		add dl, dh
		and edx, 0FFh
		movzx edx, byte ptr[ebx + edx].xm_abyBox
		xor dl, byte ptr[esi - 1]
		dec ebp
		mov byte ptr[edi - 1], dl
		jnz xencode_loop

		mov dword ptr[ebx].xm_uContextI, eax
		mov dword ptr[ebx].xm_uContextJ, ecx

		pop edi
		pop esi
		pop ebp
		pop ebx

	xno_data:
	}

	((std::vector<BYTE> *)pOutput)->insert(((std::vector<BYTE> *)pOutput)->end(), xm_vecbyTemp.begin(), xm_vecbyTemp.end());
}

void RC4Encoder::xFinalize(void *pOutput){
	UNREF_PARAM(pOutput);
}

void RC4Encoder::Transform(std::vector<BYTE> *pvecbyOutput, const BYTE *pbyKey, std::size_t uKeyLen, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	xm_pbyKey	= pbyKey;
	xm_uKeyLen	= uKeyLen;

	AbstractTransformerBaseClass::xTransform(pvecbyOutput, pbyData, uSize, bIsEndOfStream);
}
