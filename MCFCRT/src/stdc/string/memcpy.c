// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"

void *memcpy(void *restrict dst, const void *restrict src, size_t cb){
	uintptr_t unused;
	__asm__ volatile (
		"cmp %2, 64 \n"
		"jb 1f \n"
		"	mov " __MCFCRT_RCX ", " __MCFCRT_RDI " \n"
		"	neg " __MCFCRT_RCX " \n"
		"	and " __MCFCRT_RCX ", 0x0F \n"
		"	sub %2, " __MCFCRT_RCX " \n"
		"	rep movsb \n"
		"	mov " __MCFCRT_RCX ", %2 \n"
		"	shr " __MCFCRT_RCX ", 4 \n"
		"	and %2, 0x0F \n"
		"	cmp " __MCFCRT_RCX ", 64 * 1024 * 16 \n" // 16 MiB
		"	jb 4f \n"
		"		test " __MCFCRT_RSI ", 0x0F \n"
		"		jnz 3f \n"
		"			2: \n"
		"			prefetchnta byte ptr[" __MCFCRT_RSI " + 256] \n"
		"			movdqa xmm0, xmmword ptr[" __MCFCRT_RSI "] \n"
		"			movntdq xmmword ptr[" __MCFCRT_RDI "], xmm0 \n"
		"			add " __MCFCRT_RSI ", 16 \n"
		"			dec " __MCFCRT_RCX " \n"
		"			lea " __MCFCRT_RDI ", dword ptr[" __MCFCRT_RDI " + 16] \n"
		"			jnz 2b \n"
		"			jmp 1f \n"
		"		3: \n"
		"		prefetchnta byte ptr[" __MCFCRT_RSI " + 256] \n"
		"		movdqu xmm0, xmmword ptr[" __MCFCRT_RSI "] \n"
		"		movntdq xmmword ptr[" __MCFCRT_RDI "], xmm0 \n"
		"		add " __MCFCRT_RSI ", 16 \n"
		"		dec " __MCFCRT_RCX " \n"
		"		lea " __MCFCRT_RDI ", dword ptr[" __MCFCRT_RDI " + 16] \n"
		"		jnz 3b \n"
		"		jmp 1f \n"
		"	4: \n"
		"	test " __MCFCRT_RSI ", 0x0F \n"
		"	jnz 3f \n"
		"		2: \n"
		"		prefetchnta byte ptr[" __MCFCRT_RSI " + 256] \n"
		"		movdqa xmm0, xmmword ptr[" __MCFCRT_RSI "] \n"
		"		movdqa xmmword ptr[" __MCFCRT_RDI "], xmm0 \n"
		"		add " __MCFCRT_RSI ", 16 \n"
		"		dec " __MCFCRT_RCX " \n"
		"		lea " __MCFCRT_RDI ", dword ptr[" __MCFCRT_RDI " + 16] \n"
		"		jnz 2b \n"
		"		jmp 1f \n"
		"	3: \n"
		"	prefetchnta byte ptr[" __MCFCRT_RSI " + 256] \n"
		"	movdqu xmm0, xmmword ptr[" __MCFCRT_RSI "] \n"
		"	movdqa xmmword ptr[" __MCFCRT_RDI "], xmm0 \n"
		"	add " __MCFCRT_RSI ", 16 \n"
		"	dec " __MCFCRT_RCX " \n"
		"	lea " __MCFCRT_RDI ", dword ptr[" __MCFCRT_RDI " + 16] \n"
		"	jnz 3b \n"
		"1: \n"
		"mov " __MCFCRT_RCX ", %2 \n"
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
