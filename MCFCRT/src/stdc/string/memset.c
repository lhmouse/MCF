// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

static inline void fill_fwd_g(void *s, int c, size_t cb){
	uintptr_t z;
	__asm__ volatile (
		"movzx edx, al \n"
		"shl eax, 8 \n"
		"add eax, edx \n"
		"movzx edx, ax \n"
		"shl eax, 16 \n"
		"add eax, edx \n"
#ifdef _WIN64
		"mov edx, eax \n"
		"shl rax, 32 \n"
		"add rax, rdx \n"
		"shr rcx, 3 \n"
		"rep stosq \n"
		"mov rcx, rsi \n"
		"and rcx, 7 \n"
#else
		"shr ecx, 2 \n"
		"rep stosd \n"
		"mov ecx, esi \n"
		"and ecx, 3 \n"
#endif
		"rep stosb \n"
		: "=D"(z), "=a"(z), "=c"(z), "=S"(z)
		: "D"(s), "a"(c), "c"(cb), "S"(cb)
		: "dx"
	);
}

void *memset(void *s, int c, size_t n){
	register char *wp = s;
	char *const wend = wp + n;
	while(((uintptr_t)wp & (sizeof(uintptr_t) - 1)) != 0){
		if(wp == wend){
			return s;
		}
		*(wp++) = (char)c;
	}
	size_t cnt;
	if((cnt = (size_t)(wend - wp)) != 0){
		fill_fwd_g(wp, c, cnt);
	}
	return s;
}
