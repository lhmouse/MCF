// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../string/_stringasm.h"

wchar_t *wmemset(wchar_t *dst, wchar_t ch, size_t cnt){
	uintptr_t unused;
	__asm__ __volatile__(
		"mov " RCX ", %5 \n"
		"cmp " RCX ", 8 \n"
		"jb 1f  \n"
		"test " RDI ", 1 \n"
		"jz 4f \n"
		"	mov byte ptr[" RDI "], al \n"
		"	xchg al, ah \n"
		"	mov byte ptr[" RDI " + %5 * 2 - 1], al \n"
		"	inc " RDI " \n"
		"	dec %5 \n"
		"4: \n"
#ifdef _WIN64
		"test rdi, 7 \n"
#else
		"test edi, 3 \n"
#endif
		"jz 2f \n"
		"	3: \n"
		"	stosw \n"
		"	dec %5 \n"
#ifdef _WIN64
		"	test rdi, 7 \n"
#else
		"	test edi, 3 \n"
#endif
		"	jnz 3b \n"
		"2: \n"
		"movzx " RCX ", ax \n"
		"shl " RCX ", 16 \n"
		"or " RAX ", " RCX " \n"
#ifdef _WIN64
		"shl rcx, 32 \n"
		"or rax, rcx \n"
#endif
		"mov " RCX ", %5 \n"
#ifdef _WIN64
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
		: "cx"
	);
	return dst;
}
