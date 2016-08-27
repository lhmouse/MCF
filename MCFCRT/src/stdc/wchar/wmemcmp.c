// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n){
	register const wchar_t *rp1 = s1;
	register const wchar_t *rp2 = s2;
	const wchar_t *const rend = rp1 + n;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp1 & (sizeof(uintptr_t) - 1)) != 0){
		if(rp1 == rend){
			return 0;
		}
		const int32_t rc1 = *rp1 & 0xFFFF;
		const int32_t rc2 = *rp2 & 0xFFFF;
		const int32_t d = rc1 - rc2;
		if(d != 0){
			return (d >> 31) | 1;
		}
		++rp1;
		++rp2;
	}
	while((size_t)(rend - rp1) >= sizeof(uintptr_t)){
		uintptr_t w1 = *(const uintptr_t *)rp1;
		uintptr_t w2 = *(const uintptr_t *)rp2;
		if(_MCFCRT_EXPECT_NOT(w1 != w2)){
			for(unsigned i = 0; i < sizeof(uintptr_t); ++i){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
				const int32_t rc1 = w1 & 0xFFFF;
				const int32_t rc2 = w2 & 0xFFFF;
				w1 >>= 16;
				w2 >>= 16;
#else
				const int32_t rc1 = (w1 >> (sizeof(uintptr_t) * 8 - 16)) & 0xFFFF;
				const int32_t rc2 = (w2 >> (sizeof(uintptr_t) * 8 - 16)) & 0xFFFF;
				w1 <<= 16;
				w2 <<= 16;
#endif
				const int32_t d = rc1 - rc2;
				if(d != 0){
					return (d >> 31) | 1;
				}
			}
			__builtin_trap();
		}
		rp1 += sizeof(uintptr_t) / 2;
		rp2 += sizeof(uintptr_t) / 2;
	}
	for(;;){
		if(rp1 == rend){
			return 0;
		}
		const int32_t rc1 = *rp1 & 0xFFFF;
		const int32_t rc2 = *rp2 & 0xFFFF;
		const int32_t d = rc1 - rc2;
		if(d != 0){
			return (d >> 31) | 1;
		}
		++rp1;
		++rp2;
	}
}
