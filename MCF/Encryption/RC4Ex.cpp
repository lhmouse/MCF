// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "RC4Ex.hpp"
#include "../Hash/SHA256.hpp"
using namespace MCF;

namespace {
	// http://en.wikipedia.org/wiki/RC4
	// 1. 基本算法为 RC4；
	// 2. 输入 RC4 的密钥为原始密钥的散列值；
	// 3. 原文输入 RC4 前被循环移位；
	// 4. 加密输出的字节参与伪随机数生成。
	void GenInitBox(unsigned char (&abyOutput)[256], const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce) noexcept {
		SHA256 Hasher;
		union {
			struct {
				unsigned char abyNoncedKeyHash[32];
				unsigned char abyKeyHash[32];
			};
			unsigned char abyBytes[64];
		} RC4Key;

		Hasher.Update(&u64Nonce, sizeof(u64Nonce));
		Hasher.Update(pKey, uKeyLen);
		Hasher.Finalize(RC4Key.abyNoncedKeyHash);

		Hasher.Update(pKey, uKeyLen);
		Hasher.Finalize(RC4Key.abyKeyHash);

		for(std::size_t i = 0; i < 256; ++i){
			abyOutput[i] = (unsigned char)i;
		}
		unsigned char j = 0;
		for(std::size_t i = 0; i < 256; ++i){
			const auto b0 = abyOutput[i];
			j += b0 + RC4Key.abyBytes[i % sizeof(RC4Key.abyBytes)];
			const auto b1 = abyOutput[j];
			abyOutput[i] = b1;
			abyOutput[j] = b0;
		}
	}
/*
	inline unsigned char RotateByteLeft(unsigned char by, unsigned char shift) noexcept {
		register unsigned char ret;
		__asm__ __volatile__(
			"rol %b0, cl \n"
			: "=q"(ret)
			: "c"(shift), "0"(by)
		);
		return ret;
	}
	inline unsigned char RotateByteRight(unsigned char by, unsigned char shift) noexcept {
		register unsigned char ret;
		__asm__ __volatile__(
			"ror %b0, cl \n"
			: "=q"(ret)
			: "c"(shift), "0"(by)
		);
		return ret;
	}
*/
}

// ========== RC4ExEncoder ==========
// 构造函数和析构函数。
RC4ExEncoder::RC4ExEncoder(std::function<void *(std::size_t)> fnDataCallback, const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce) noexcept
	: xm_fnDataCallback(std::move(fnDataCallback))
{
	GenInitBox(xm_abyInitBox, pKey, uKeyLen, u64Nonce);
	xm_bInited = false;
}

// 其他非静态成员函数。
void RC4ExEncoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_bInited = true;
		__builtin_memcpy(xm_abyBox, xm_abyInitBox, sizeof(xm_abyBox));
		xm_byI = 0;
		xm_byJ = 0;
	}

	auto pbyRead = (const unsigned char *)pData;
	auto pbyWrite = (unsigned char *)xm_fnDataCallback(uSize);
/*
	auto i = xm_byI;
	auto j = xm_byJ;

	for(std::size_t k = 0; k < uSize; ++k){
		++i;
		const auto b0 = xm_abyBox[i];
		j += b0;
		const auto b1 = xm_abyBox[j];
		xm_abyBox[i] = b1;
		xm_abyBox[j] = b0;
		const unsigned char ch = RotateByteLeft(*(pbyRead++), b1 & 7) ^ (b0 + b1);
		*(pbyWrite++) = ch;
		j += ch;
	}

	xm_byI = i;
	xm_byJ = j;
*/
	std::uintptr_t unused;
	__asm__ __volatile__(
#ifdef _WIN64
		"test %4, %4 \n"
		"jz 1f \n"
		"	.align 16 \n"
		"	2: \n"
		"	inc rax \n"
		"	movzx rax, al \n"
		"	movzx rbx, byte ptr[%5 + rax] \n"
		"	movzx r8, byte ptr[rsi] \n"
		"	add rdx, rbx \n"
		"	movzx rdx, dl \n"
		"	inc rsi \n"
		"	movzx rcx, byte ptr[%5 + rdx] \n"
		"	mov byte ptr[%5 + rdx], bl \n"
		"	add rbx, rcx \n"
		"	mov byte ptr[%5 + rax], cl \n"
		"	and rcx, 7 \n"
		"	movzx r8, r8b \n"
		"	rol r8b, cl \n"
		"	lea rdi, dword ptr[rdi + 1] \n"
		"	xor r8b, bl \n"
 		"	mov byte ptr[rdi - 1], r8b \n"
		"	movzx rdx, dl \n"
		"	add dl, r8b \n"
		"	dec %4 \n"
		"	jnz 2b \n"
		"1: \n"
		: "=a"(xm_byI), "=d"(xm_byJ), "=S"(unused), "=D"(unused), "=r"(unused)
		: "r"(&xm_abyBox), "0"(xm_byI), "1"(xm_byJ), "2"(pbyRead), "3"(pbyWrite), "4"(uSize)
		: "cx", "bx", "r8"
#else
		"mov ecx, dword ptr[%4] \n"
		"test ecx, ecx \n"
		"jz 1f \n"
		"	push ebp \n"
		"	mov ebp, ecx \n"
		"	.align 16 \n"
		"	2: \n"
		"	inc al \n"
		"	movzx eax, al \n"
		"	movzx ecx, byte ptr[ebx + eax] \n"
		"	movzx edx, dl \n"
		"	add dl, cl \n"
		"	shl ecx, 8 \n"
		"	mov cl, byte ptr[ebx + edx] \n"
		"	mov byte ptr[ebx + edx], ch \n"
		"	add ch, cl \n"
		"	mov byte ptr[ebx + eax], cl \n"
		"	and cl, 7 \n"
		"	mov dh, byte ptr[esi] \n"
		"	inc esi \n"
		"	rol dh, cl \n"
		"	lea edi, dword ptr[edi + 1] \n"
		"	xor dh, ch \n"
		"	mov byte ptr[edi - 1], dh \n"
		"	add dl, dh \n"
		"	dec ebp \n"
		"	jnz 2b \n"
		"	pop ebp \n"
		"1: \n"
		: "=a"(xm_byI), "=d"(xm_byJ), "=S"(unused), "=D"(unused)
		: "m"(uSize), "b"(&xm_abyBox), "0"(xm_byI), "1"(xm_byJ), "2"(pbyRead), "3"(pbyWrite)
		: "cx"
#endif
	);
}
void RC4ExEncoder::Finalize(){
	if(xm_bInited){
		xm_bInited = false;
	}
}

// ========== RC4ExDecoder ==========
// 构造函数和析构函数。
RC4ExDecoder::RC4ExDecoder(std::function<void *(std::size_t)> fnDataCallback, const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce) noexcept
	: xm_fnDataCallback(std::move(fnDataCallback))
{
	GenInitBox(xm_abyInitBox, pKey, uKeyLen, u64Nonce);
	xm_bInited = false;
}

// 其他非静态成员函数。
void RC4ExDecoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_bInited = true;
		__builtin_memcpy(xm_abyBox, xm_abyInitBox, sizeof(xm_abyBox));
		xm_byI = 0;
		xm_byJ = 0;
	}

	auto pbyRead = (const unsigned char *)pData;
	auto pbyWrite = (unsigned char *)xm_fnDataCallback(uSize);
/*
	auto i = xm_byI;
	auto j = xm_byJ;

	for(std::size_t k = 0; k < uSize; ++k){
		++i;
		const auto b0 = xm_abyBox[i];
		j += b0;
		const auto b1 = xm_abyBox[j];
		xm_abyBox[i] = b1;
		xm_abyBox[j] = b0;
		const unsigned char ch = *(pbyRead++);
		*(pbyWrite++) = RotateByteRight(ch ^ (b0 + b1), b1 & 7);
		j += ch;
	}

	xm_byI = i;
	xm_byJ = j;
*/
	std::uintptr_t unused;
	__asm__ __volatile__(
#ifdef _WIN64
		"test %4, %4 \n"
		"jz 1f \n"
		"	.align 16 \n"
		"	2: \n"
		"	inc rax \n"
		"	movzx rax, al \n"
		"	movzx rbx, byte ptr[%5 + rax] \n"
		"	movzx r8, byte ptr[rsi] \n"
		"	add rdx, rbx \n"
		"	movzx rdx, dl \n"
		"	inc rsi \n"
		"	movzx rcx, byte ptr[%5 + rdx] \n"
		"	mov byte ptr[%5 + rdx], bl \n"
		"	add rbx, rcx \n"
		"	mov byte ptr[%5 + rax], cl \n"
		"	and rcx, 7 \n"
		"	movzx r9, r8b \n"
		"	xor r9b, bl \n"
		"	lea rdi, dword ptr[rdi + 1] \n"
		"	ror r9b, cl \n"
		"	mov byte ptr[rdi - 1], r9b \n"
		"	lea rdx, dword ptr[rdx + r8] \n"
		"	dec %4 \n"
		"	jnz 2b \n"
		"1: \n"
		: "=a"(xm_byI), "=d"(xm_byJ), "=S"(unused), "=D"(unused), "=r"(unused)
		: "r"(&xm_abyBox), "0"(xm_byI), "1"(xm_byJ), "2"(pbyRead), "3"(pbyWrite), "4"(uSize)
		: "cx", "bx", "r8", "r9"
#else
		"mov ecx, dword ptr[%4] \n"
		"test ecx, ecx \n"
		"jz 1f \n"
		"	push ebp \n"
		"	mov ebp, ecx \n"
		"	.align 16 \n"
		"	2: \n"
		"	inc al \n"
		"	movzx eax, al \n"
		"	movzx ecx, byte ptr[ebx + eax] \n"
		"	movzx edx, dl \n"
		"	add dl, cl \n"
		"	shl ecx, 8 \n"
		"	mov cl, byte ptr[ebx + edx] \n"
		"	mov byte ptr[ebx + edx], ch \n"
		"	add ch, cl \n"
		"	mov byte ptr[ebx + eax], cl \n"
		"	and cl, 7 \n"
		"	mov dh, byte ptr[esi] \n"
		"	inc esi \n"
		"	add dl, dh \n"
		"	xor dh, ch \n"
		"	lea edi, dword ptr[edi + 1] \n"
		"	ror dh, cl \n"
		"	mov byte ptr[edi - 1], dh \n"
		"	dec ebp \n"
		"	jnz 2b \n"
		"	pop ebp \n"
		"1: \n"
		: "=a"(xm_byI), "=d"(xm_byJ), "=S"(unused), "=D"(unused)
		: "m"(uSize), "b"(&xm_abyBox), "0"(xm_byI), "1"(xm_byJ), "2"(pbyRead), "3"(pbyWrite)
		: "cx"
#endif
	);
}
void RC4ExDecoder::Finalize(){
	if(xm_bInited){
		xm_bInited = false;
	}
}
