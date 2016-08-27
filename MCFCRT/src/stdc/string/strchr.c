// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

char *strchr(const char *s, int c){
	register const char *rp = s;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		const char rc = *rp;
		if(rc == (char)c){
			return (char *)rp;
		}
		if(rc == 0){
			return nullptr;
		}
		++rp;
	}
	for(;;){
		uintptr_t w = *(const uintptr_t *)rp;
		for(unsigned i = 0; i < sizeof(uintptr_t); ++i){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			const char rc = (char)w;
			w >>= 8;
#else
			const char rc = (char)(w >> (sizeof(uintptr_t) * 8 - 8));
			w <<= 8;
#endif
			if(rc == (char)c){
				return (char *)rp;
			}
			if(rc == 0){
				return nullptr;
			}
			++rp;
		}
	}
}
