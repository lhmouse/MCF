// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

int wcscmp(const wchar_t *s1, const wchar_t *s2){
	register const wchar_t *rp1 = s1, *rp2 = s2;
	for(;;){
		const int32_t rc1 = *rp1 & 0xFFFF;
		const int32_t rc2 = *rp2 & 0xFFFF;
		const int32_t d = rc1 - rc2;
		if(d != 0){
			return (d >> 31) | 1;
		}
		if(rc1 == 0){
			return 0;
		}
		++rp1;
		++rp2;
	}
}
