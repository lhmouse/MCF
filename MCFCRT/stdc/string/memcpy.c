// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_string_asm.h"

void *memcpy(void *restrict dst, const void *restrict src, size_t cb){
	uintptr_t unused;
	__asm__ __volatile__(
		"cmp %2, 64 \n"
		"jb 1f \n"
		"	mov " __RCX ", " __RDI " \n"
		"	neg " __RCX " \n"
		"	and " __RCX ", 0x0F \n"
		"	sub %2, " __RCX " \n"
		"	rep movsb \n"
		"	mov " __RCX ", %2 \n"
		"	shr " __RCX ", 4 \n"
		"	and %2, 0x0F \n"
		"	cmp " __RCX ", 64 * 1024 * 16 \n" // 16 MiB
		"	jb 4f \n"
		"		test " __RSI ", 0x0F \n"
		"		jnz 3f \n"
		"			2: \n"
		"			prefetchnta byte ptr[" __RSI " + 256] \n"
		"			movdqa xmm0, xmmword ptr[" __RSI "] \n"
		"			movntdq xmmword ptr[" __RDI "], xmm0 \n"
		"			add " __RSI ", 16 \n"
		"			dec " __RCX " \n"
		"			lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"			jnz 2b \n"
		"			jmp 1f \n"
		"		3: \n"
		"		prefetchnta byte ptr[" __RSI " + 256] \n"
		"		movdqu xmm0, xmmword ptr[" __RSI "] \n"
		"		movntdq xmmword ptr[" __RDI "], xmm0 \n"
		"		add " __RSI ", 16 \n"
		"		dec " __RCX " \n"
		"		lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"		jnz 3b \n"
		"		jmp 1f \n"
		"	4: \n"
		"	test " __RSI ", 0x0F \n"
		"	jnz 3f \n"
		"		2: \n"
		"		prefetchnta byte ptr[" __RSI " + 256] \n"
		"		movdqa xmm0, xmmword ptr[" __RSI "] \n"
		"		movdqa xmmword ptr[" __RDI "], xmm0 \n"
		"		add " __RSI ", 16 \n"
		"		dec " __RCX " \n"
		"		lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"		jnz 2b \n"
		"		jmp 1f \n"
		"	3: \n"
		"	prefetchnta byte ptr[" __RSI " + 256] \n"
		"	movdqu xmm0, xmmword ptr[" __RSI "] \n"
		"	movdqa xmmword ptr[" __RDI "], xmm0 \n"
		"	add " __RSI ", 16 \n"
		"	dec " __RCX " \n"
		"	lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"	jnz 3b \n"
		"1: \n"
		"mov " __RCX ", %2 \n"
#ifdef _WIN64
		"shr rcx, 3 \n"
		"rep movsq \n"
		"mov rcx, %2 \n"
		"and rcx, 7 \n"
#else
		"shr ecx, 2 \n"
		"rep movsd \n"
		"mov ecx, %2 \n"
		"and ecx, 3 \n"
#endif
		"rep movsb \n"
		: "=D"(unused), "=S"(unused), "=r"(unused)
		: "0"(dst), "1"(src), "2"(cb)
		: "cx", "xmm0"
	);
	return dst;
}
