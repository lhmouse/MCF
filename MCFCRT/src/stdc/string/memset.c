// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/inline_mem.h"

#undef memset

void *memset(void *s, int c, size_t n){
	_MCFCRT_inline_mempset_fwd(s, c, n);
	return s;
}
