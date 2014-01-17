// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_stringasm.h"

void *memset(void *dst, int ch, size_t cb){
	uintptr_t unused;
	__asm__ __volatile__(
		"mov "__CX__", %5 \n"
		"cmp "__CX__", 16 \n"
		"jb 1f  \n"
#ifdef __amd64__
		"test "__DI__", 7 \n"
#else
		"test "__DI__", 3 \n"
#endif
		"jz 2f \n"
		"	3: \n"
		"	stosb \n"
		"	dec %5 \n"
#ifdef __amd64__
		"	test "__DI__", 7 \n"
#else
		"	test "__DI__", 3 \n"
#endif
		"	jnz 3b \n"
		"2: \n"
		"movzx "__AX__", al \n"
		"mov ah, al \n"
		"movzx "__CX__", ax \n"
		"shl "__CX__", 16 \n"
		"or "__AX__", "__CX__" \n"
#ifdef __amd64__
		"shl rcx, 32 \n"
		"or rax, rcx \n"
#endif
		"mov "__CX__", %5 \n"
#ifdef __amd64__
		"shr rcx, 3 \n"
		"rep stosq \n"
		"mov rcx, %5 \n"
		"and rcx, 7 \n"
#else
		"shr ecx, 2 \n"
		"rep stosd \n"
		"mov ecx, %5 \n"
		"and ecx, 3 \n"
#endif
		"1: \n"
		"rep stosb \n"
		: "=D"(unused), "=a"(unused), "=r"(unused)
		: "0"(dst), "1"(ch), "2"(cb)
		: __CX__
	);
	return dst;
}
