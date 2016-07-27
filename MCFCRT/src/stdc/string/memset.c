// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_string_asm.h"

__attribute__((__force_align_arg_pointer__))
void *memset(void *dst, int ch, size_t cb){
	uintptr_t unused;
	alignas(16) char xmmreg[16];
	__asm__ volatile (
		"movzx eax, al \n"
		"mov ecx, eax \n"
		"shl ecx, 8 \n"
		"or eax, ecx \n"
		"mov ecx, eax \n"
		"shl ecx, 16 \n"
		"or eax, ecx \n"
#ifdef _WIN64
		"mov rcx, rax \n"
		"shl rcx, 32 \n"
		"or rax, rcx \n"
#endif
		"cmp %2, 64 \n"
		"jb 1f \n"
#ifdef _WIN64
		"	mov qword ptr[%3], rax \n"
		"	mov qword ptr[%3 + 8], rax \n"
#else
		"	mov dword ptr[%3], eax \n"
		"	mov dword ptr[%3 + 4], eax \n"
		"	mov dword ptr[%3 + 8], eax \n"
		"	mov dword ptr[%3 + 12], eax \n"
#endif
		"	mov " __MCFCRT_RCX ", " __MCFCRT_RDI " \n"
		"	neg " __MCFCRT_RCX " \n"
		"	and " __MCFCRT_RCX ", 0x0F \n"
		"	sub %2, " __MCFCRT_RCX " \n"
		"	movdqa xmm0, xmmword ptr[%3] \n"
		"	rep stosb \n"
		"	mov " __MCFCRT_RCX ", %2 \n"
		"	shr " __MCFCRT_RCX ", 4 \n"
		"	and %2, 0x0F \n"
		"	cmp " __MCFCRT_RCX ", 64 * 1024 * 16 \n" // 16 MiB
		"	jb 4f \n"
		"		3: \n"
		"		movntdq xmmword ptr[" __MCFCRT_RDI "], xmm0 \n"
		"		dec " __MCFCRT_RCX " \n"
		"		lea " __MCFCRT_RDI ", dword ptr[" __MCFCRT_RDI " + 16] \n"
		"		jnz 3b \n"
		"		jmp 1f \n"
		"	4: \n"
		"	3: \n"
		"	movdqa xmmword ptr[" __MCFCRT_RDI "], xmm0 \n"
		"	dec " __MCFCRT_RCX " \n"
		"	lea " __MCFCRT_RDI ", dword ptr[" __MCFCRT_RDI " + 16] \n"
		"	jnz 3b \n"
		"1: \n"
		"mov " __MCFCRT_RCX ", %2 \n"
#ifdef _WIN64
		"shr rcx, 3 \n"
		"rep stosq \n"
		"mov rcx, %2 \n"
		"and rcx, 7 \n"
#else
		"shr ecx, 2 \n"
		"rep stosd \n"
		"mov ecx, %2 \n"
		"and ecx, 3 \n"
#endif
		"rep stosb \n"
		: "=D"(unused), "=a"(unused), "=r"(unused), "=m"(xmmreg)
		: "0"(dst), "1"(ch), "2"(cb)
		: "cx", "xmm0"
	);
	return dst;
}
