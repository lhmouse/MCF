// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "stpcpy.h"
#include "expect.h"
#include "assert.h"

char *_MCFCRT_stpcpy(char *restrict dst, const char *restrict src){
	register char *wp = dst;
	register const char *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		if((*wp = *rp) == 0){
			return wp;
		}
		++wp;
		++rp;
	}
	for(;;){
		uintptr_t w = *(const uintptr_t *)rp;
#ifdef _WIN64
		w = (w - 0x0101010101010101u) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & 0x8080808080808080u) != 0))
#else
		w = (w - 0x01010101u) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & 0x80808080u) != 0))
#endif
		{
			for(unsigned i = 0; i < sizeof(uintptr_t) - 1; ++i){
				if((*wp = *rp) == 0){
					return wp;
				}
				++wp;
				++rp;
			}
			*wp = 0;
			return wp;
		}
		w = *(const uintptr_t *)rp;
		*(uintptr_t *)wp = w;
		wp += sizeof(uintptr_t);
		rp += sizeof(uintptr_t);
	}
}
char *_MCFCRT_stppcpy(char *dst, char *end, const char *restrict src){
	register char *wp = dst;
	register const char *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		if(wp == end - 1){
			*wp = 0;
			return wp;
		}
		if((*wp = *rp) == 0){
			return wp;
		}
		++wp;
		++rp;
	}
	for(;;){
		uintptr_t w = *(const uintptr_t *)rp;
#ifdef _WIN64
		w = (w - 0x0101010101010101u) & ~w;
		if(((w & 0x8080808080808080u) != 0))
#else
		w = (w - 0x01010101u) & ~w;
		if(((w & 0x80808080u) != 0))
#endif
		{
			for(unsigned i = 0; i < sizeof(uintptr_t) - 1; ++i){
				if(wp == end - 1){
					*wp = 0;
					return wp;
				}
				if((*wp = *rp) == 0){
					return wp;
				}
				++wp;
				++rp;
			}
			*wp = 0;
			return wp;
		}
		const unsigned cap = (unsigned)(end - 1 - wp);
		if(cap >= sizeof(uintptr_t)){
			w = *(const uintptr_t *)rp;
			*(uintptr_t *)wp = w;
		} else {
			for(unsigned i = 0; i < cap; ++i){
				*wp = *rp;
				++wp;
				++rp;
			}
			*wp = 0;
			return wp;
		}
		wp += sizeof(uintptr_t);
		rp += sizeof(uintptr_t);
	}
}
