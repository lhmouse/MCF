// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2032, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

__attribute__((__always_inline__))
static inline void copy_fwd_sse2(void *restrict s1, const void *restrict s2, size_t cxm){
	register char *wp = s1;
	char *const wend = wp + cxm * 16;
	register const char *rp = s2;
#define SSE2_STEP(si_, li_, r_)	\
	{	\
		__asm__ volatile (	\
			li_ " " r_ ", xmmword ptr[%1] \n"	\
			si_ " xmmword ptr[%0], " r_ " \n"	\
			: : "r"(wp), "r"(rp)	\
			: r_	\
		);	\
		wp += 16;	\
		rp += 16;	\
	}
#define SSE2_FULL(si_, li_)	\
	{	\
		switch(cxm % 8){	\
			do { __builtin_prefetch(rp + 256);	\
		default: SSE2_STEP(si_, li_, "xmm0")	\
		case 7:  SSE2_STEP(si_, li_, "xmm1")	\
		case 6:  SSE2_STEP(si_, li_, "xmm2")	\
		case 5:  SSE2_STEP(si_, li_, "xmm3")	\
		case 4:  SSE2_STEP(si_, li_, "xmm0")	\
		case 3:  SSE2_STEP(si_, li_, "xmm1")	\
		case 2:  SSE2_STEP(si_, li_, "xmm2")	\
		case 1:  SSE2_STEP(si_, li_, "xmm3")	\
			} while(wp != wend);	\
		}	\
	}
	if(cxm < 0x1000){
		if(((uintptr_t)rp & 15) != 0){
			SSE2_FULL("movdqa", "movdqu")
		} else {
			SSE2_FULL("movdqa", "movdqa")
		}
	} else {
		if(((uintptr_t)rp & 15) != 0){
			SSE2_FULL("movntdq", "movdqu")
		} else {
			SSE2_FULL("movntdq", "movntdqa")
		}
	}
#undef SSE2_STEP
#undef SSE2_FULL
}
__attribute__((__always_inline__))
static inline void copy_fwd_g(void *s1, const void *s2, size_t cb){
	uintptr_t z;
	__asm__ volatile (
#ifdef _WIN64
		"shr rcx, 3 \n"
		"rep movsq \n"
		"mov rcx, rax \n"
		"and rcx, 7 \n"
#else
		"shr ecx, 2 \n"
		"rep movsd \n"
		"mov ecx, eax \n"
		"and ecx, 3 \n"
#endif
		"rep movsb \n"
		: "=D"(z), "=S"(z), "=c"(z), "=a"(z)
		: "D"(s1), "S"(s2), "c"(cb), "a"(cb)
	);
}

__attribute__((__always_inline__))
static inline void copy_bkwd_sse2(void *s1, const void *s2, size_t cxm){
	char *const wbegin = s1;
	register char *wp = wbegin + cxm * 16;
	register const char *rp = (const char *)s2 + cxm * 16;
#define SSE2_STEP(si_, li_, r_)	\
	{	\
		wp -= 16;	\
		rp -= 16;	\
		__asm__ volatile (	\
			li_ " " r_ ", xmmword ptr[%1] \n"	\
			si_ " xmmword ptr[%0], " r_ " \n"	\
			: : "r"(wp), "r"(rp)	\
			: r_	\
		);	\
	}
#define SSE2_FULL(si_, li_)	\
	{	\
		switch(cxm % 8){	\
			do { __builtin_prefetch(rp - 256);	\
		default: SSE2_STEP(si_, li_, "xmm0")	\
		case 7:  SSE2_STEP(si_, li_, "xmm1")	\
		case 6:  SSE2_STEP(si_, li_, "xmm2")	\
		case 5:  SSE2_STEP(si_, li_, "xmm3")	\
		case 4:  SSE2_STEP(si_, li_, "xmm0")	\
		case 3:  SSE2_STEP(si_, li_, "xmm1")	\
		case 2:  SSE2_STEP(si_, li_, "xmm2")	\
		case 1:  SSE2_STEP(si_, li_, "xmm3")	\
			} while(wbegin != wp);	\
		}	\
	}
	if(cxm < 0x1000){
		if(((uintptr_t)rp & 15) != 0){
			SSE2_FULL("movdqa", "movdqu")
		} else {
			SSE2_FULL("movdqa", "movdqa")
		}
	} else {
		if(((uintptr_t)rp & 15) != 0){
			SSE2_FULL("movntdq", "movdqu")
		} else {
			SSE2_FULL("movntdq", "movntdqa")
		}
	}
#undef SSE2_STEP
#undef SSE2_FULL
}
__attribute__((__always_inline__))
static inline void copy_bkwd_g(void *s1, const void *s2, size_t cb){
	uintptr_t z;
	__asm__ volatile (
		"std \n"
#ifdef _WIN64
		"sub rdi, 8 \n"
		"sub rsi, 8 \n"
		"shr rcx, 3 \n"
		"rep movsq \n"
		"add rdi, 7 \n"
		"add rsi, 7 \n"
		"mov rcx, rax \n"
		"and rcx, 7 \n"
#else
		"sub edi, 4 \n"
		"sub esi, 4 \n"
		"shr ecx, 2 \n"
		"rep movsd \n"
		"add edi, 3 \n"
		"add esi, 3 \n"
		"mov ecx, eax \n"
		"and ecx, 3 \n"
#endif
		"rep movsb \n"
		"cld \n"
		: "=D"(z), "=S"(z), "=c"(z), "=a"(z)
		: "D"(s1), "S"(s2), "c"(cb), "a"(cb)
	);
}

void *memmove(void *s1, const void *s2, size_t n){
	if(s1 < s2){
		register char *wp = s1;
		char *const wend = wp + n;
		register const char *rp = s2;
		while(((uintptr_t)wp & 15) != 0){
			if(wp == wend){
				return s1;
			}
			*(wp++) = *(rp++);
		}
		size_t cnt;
		if((cnt = (size_t)(wend - wp) / 16) != 0){
			copy_fwd_sse2(wp, rp, cnt);
			wp += cnt * 16;
			rp += cnt * 16;
		}
		if((cnt = (size_t)(wend - wp)) != 0){
			copy_fwd_g(wp, rp, cnt);
		}
	} else {
		char *const wbegin = s1;
		register char *wp = wbegin + n;
		register const char *rp = (const char *)s2 + n;
		while(((uintptr_t)wp & 15) != 0){
			if(wbegin == wp){
				return s1;
			}
			*(--wp) = *(--rp);
		}
		size_t cnt;
		if((cnt = (size_t)(wp - wbegin) / 16) != 0){
			wp -= cnt * 16;
			rp -= cnt * 16;
			copy_bkwd_sse2(wp, rp, cnt);
		}
		if((cnt = (size_t)(wp - wbegin)) != 0){
			copy_bkwd_g(wp, rp, cnt);
		}
	}
	return s1;
}
