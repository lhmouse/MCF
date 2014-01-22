// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_stringasm.h"

void *memcpy(void *restrict dst, const void *restrict src, size_t cb){
	uintptr_t unused;
	__asm__ __volatile__(
		"cmp %5, 64 \n"
		"jb 1f \n"
		"	mov "__CX__", "__DI__" \n"
		"	neg "__CX__" \n"
		"	and "__CX__", 0x0F \n"
		"	sub %5, "__CX__" \n"
		"	rep movsb \n"
		"	mov "__CX__", %5 \n"
		"	shr "__CX__", 4 \n"
		"	and %5, 0x0F \n"
		"	test "__SI__", 0x0F \n"
		"	jnz 3f \n"
		"		2: \n"
		"		movdqa xmm0, xmmword ptr["__SI__"] \n"
		"		add "__SI__", 16 \n"
		"		lea "__DI__", dword ptr["__DI__" + 16] \n"
		"		movntdq xmmword ptr["__DI__" - 16], xmm0 \n"
		"		dec "__CX__" \n"
		"		jnz 2b \n"
		"		jmp 1f \n"
		"	3: \n"
		"	movdqu xmm0, xmmword ptr["__SI__"] \n"
		"	add "__SI__", 16 \n"
		"	lea "__DI__", dword ptr["__DI__" + 16] \n"
		"	movntdq xmmword ptr["__DI__" - 16], xmm0 \n"
		"	dec "__CX__" \n"
		"	jnz 3b \n"
		"1: \n"
		"mov "__CX__", %5 \n"
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
		: __CX__
	);
	return dst;
}
