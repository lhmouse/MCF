// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../ext/expect.h"
#include "../string/_endian.h"

size_t wcslen(const wchar_t *s){
	register const wchar_t *rp = s;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1) & (size_t)-2) != 0){
		const wchar_t rc = *rp;
		if(rc == 0){
			return (size_t)(rp - s);
		}
		++rp;
	}
	uintptr_t m = 0x0001;
	uintptr_t t = 0x8000;
	for(unsigned i = 2; i < sizeof(m); i <<= 1){
		m += m << i * 8;
		t += t << i * 8;
	}
	for(;;){
		uintptr_t w = __MCFCRT_LOAD_UINTPTR_LE(*(const uintptr_t *)rp);
		w = (w - m) & ~w;
		if(_MCFCRT_EXPECT_NOT((w & t) != 0)){
			for(unsigned i = 0; i < sizeof(uintptr_t) / 2; ++i){
				const bool rc = w & 0x8000;
				if(rc){
					return (size_t)(rp - s) + i;
				}
				w >>= 16;
			}
			__builtin_trap();
		}
		rp += sizeof(uintptr_t) / 2;
	}
}
