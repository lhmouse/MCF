// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "wcpcpy.h"
#include "expect.h"
#include "assert.h"

wchar_t *_MCFCRT_wcpcpy(wchar_t *restrict dst, const wchar_t *restrict src){
	register wchar_t *wp = dst;
	register const wchar_t *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1) & (size_t)-2) != 0){
		if((*wp = *rp) == 0){
			return wp;
		}
		++wp;
		++rp;
	}
	for(;;){
		uintptr_t w = *(const uintptr_t *)rp;
#ifdef _WIN64
		w = (w - 0x0001000100010001u) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & 0x8000800080008000u) != 0))
#else
		w = (w - 0x00010001u) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & 0x80008000u) != 0))
#endif
		{
			for(unsigned i = 0; i < sizeof(uintptr_t) / 2 - 1; ++i){
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
		wp += sizeof(uintptr_t) / 2;
		rp += sizeof(uintptr_t) / 2;
	}
}
wchar_t *_MCFCRT_wcppcpy(wchar_t *dst, wchar_t *end, const wchar_t *restrict src){
	register wchar_t *wp = dst;
	register const wchar_t *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1) & (size_t)-2) != 0){
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
		w = (w - 0x0001000100010001u) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & 0x8000800080008000u) != 0))
#else
		w = (w - 0x00010001u) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & 0x80008000u) != 0))
#endif
		{
			for(unsigned i = 0; i < sizeof(uintptr_t) / 2 - 1; ++i){
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
		if(cap >= sizeof(uintptr_t) / 2){
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
		wp += sizeof(uintptr_t) / 2;
		rp += sizeof(uintptr_t) / 2;
	}
}
