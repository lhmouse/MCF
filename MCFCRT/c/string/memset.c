// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_stringasm.h"

void *memset(void *dst, int ch, size_t cb){
	uintptr_t unused;
	__asm__ __volatile__(
		"mov " RCX ", %5 \n"
		"cmp " RCX ", 16 \n"
		"jb 1f  \n"
#ifdef _WIN64
		"test rdi, 7 \n"
#else
		"test edi, 3 \n"
#endif
		"jz 2f \n"
		"	3: \n"
		"	stosb \n"
		"	dec %5 \n"
#ifdef _WIN64
		"	test rdi, 7 \n"
#else
		"	test edi, 3 \n"
#endif
		"	jnz 3b \n"
		"2: \n"
		"movzx " RAX ", al \n"
		"mov ah, al \n"
		"movzx " RCX ", ax \n"
		"shl " RCX ", 16 \n"
		"or " RAX ", " RCX " \n"
#ifdef _WIN64
		"shl rcx, 32 \n"
		"or rax, rcx \n"
#endif
		"mov " RCX ", %5 \n"
#ifdef _WIN64
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
		: "cx"
	);
	return dst;
}
