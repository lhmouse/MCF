// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_stringasm.h"

void *memcpy(void *restrict dst, const void *restrict src, size_t cb){
	uintptr_t unused;
	__asm__ __volatile__(
		"cmp %5, 64 \n"
		"jb 1f \n"
		"	mov " RCX ", " RDI " \n"
		"	neg " RCX " \n"
		"	and " RCX ", 0x0F \n"
		"	sub %5, " RCX " \n"
		"	rep movsb \n"
		"	mov " RCX ", %5 \n"
		"	shr " RCX ", 4 \n"
		"	and %5, 0x0F \n"
		"	cmp " RCX ", 64 \n" // 1KiB
		"	jb 4f \n"
		"		test " RSI ", 0x0F \n"
		"		jnz 3f \n"
		"			2: \n"
		"			movdqa xmm0, xmmword ptr[" RSI "] \n"
		"			add " RSI ", 16 \n"
		"			lea " RDI ", dword ptr[" RDI " + 16] \n"
		"			movntdq xmmword ptr[" RDI " - 16], xmm0 \n"
		"			dec " RCX " \n"
		"			jnz 2b \n"
		"			jmp 1f \n"
		"		3: \n"
		"		movdqu xmm0, xmmword ptr[" RSI "] \n"
		"		add " RSI ", 16 \n"
		"		lea " RDI ", dword ptr[" RDI " + 16] \n"
		"		movntdq xmmword ptr[" RDI " - 16], xmm0 \n"
		"		dec " RCX " \n"
		"		jnz 3b \n"
		"		jmp 1f \n"
		"	4: \n"
		"	test " RSI ", 0x0F \n"
		"	jnz 3f \n"
		"		2: \n"
		"		movdqa xmm0, xmmword ptr[" RSI "] \n"
		"		add " RSI ", 16 \n"
		"		lea " RDI ", dword ptr[" RDI " + 16] \n"
		"		movdqa xmmword ptr[" RDI " - 16], xmm0 \n"
		"		dec " RCX " \n"
		"		jnz 2b \n"
		"		jmp 1f \n"
		"	3: \n"
		"	movdqu xmm0, xmmword ptr[" RSI "] \n"
		"	add " RSI ", 16 \n"
		"	lea " RDI ", dword ptr[" RDI " + 16] \n"
		"	movdqa xmmword ptr[" RDI " - 16], xmm0 \n"
		"	dec " RCX " \n"
		"	jnz 3b \n"
		"1: \n"
		"mov " RCX ", %5 \n"
#ifdef _WIN64
		"shr rcx, 3 \n"
		"rep movsq \n"
		"mov rcx, %5 \n"
		"and rcx, 7 \n"
#else
		"shr ecx, 2 \n"
		"rep movsd \n"
		"mov ecx, %5 \n"
		"and ecx, 3 \n"
#endif
		"rep movsb \n"
		: "=D"(unused), "=S"(unused), "=r"(unused)
		: "0"(dst), "1"(src), "2"(cb)
		: "cx"
	);
	return dst;
}
