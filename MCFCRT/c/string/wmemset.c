// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_stringasm.h"

__MCF_CRT_EXTERN wchar_t *wmemset(wchar_t *dst, wchar_t ch, size_t cnt){
	uintptr_t unused;
	__asm__ __volatile__(
		"mov "__CX__", %5 \n"
		"cmp "__CX__", 8 \n"
		"jb 1f  \n"
		"test "__DI__", 1 \n"
		"jz 4f \n"
		"	mov byte ptr["__DI__"], al \n"
		"	xchg al, ah \n"
		"	mov byte ptr["__DI__" + %5 * 2 - 1], al \n"
		"	inc "__DI__" \n"
		"	dec %5 \n"
		"4: \n"
#ifdef __amd64__
		"test "__DI__", 7 \n"
#else
		"test "__DI__", 3 \n"
#endif
		"jz 2f \n"
		"	3: \n"
		"	stosw \n"
		"	dec %5 \n"
#ifdef __amd64__
		"	test "__DI__", 7 \n"
#else
		"	test "__DI__", 3 \n"
#endif
		"	jnz 3b \n"
		"2: \n"
		"movzx "__CX__", ax \n"
		"shl "__CX__", 16 \n"
		"or "__AX__", "__CX__" \n"
#ifdef __amd64__
		"shl rcx, 32 \n"
		"or rax, rcx \n"
#endif
		"mov "__CX__", %5 \n"
#ifdef __amd64__
		"shr rcx, 2 \n"
		"rep stosq \n"
		"mov rcx, %5 \n"
		"and rcx, 3 \n"
#else
		"shr ecx, 1 \n"
		"rep stosd \n"
		"mov ecx, %5 \n"
		"and ecx, 1 \n"
#endif
		"1: \n"
		"rep stosw \n"
		: "=D"(unused), "=a"(unused), "=r"(unused)
		: "0"(dst), "1"(ch), "2"(cnt)
		: __CX__
	);
	return dst;
}
