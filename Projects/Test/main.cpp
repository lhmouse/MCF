#include <MCFCRT/env/inline_mem.h>

char s1[100], s2[100];

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	void *p = s2;
	p = _MCFCRT_inline_mempset_fwd(p, 'a', 60);
	p = _MCFCRT_inline_mempset_fwd(p, 'b', 40);
	p = s1;
	p = _MCFCRT_inline_mempcpy_fwd(p, s2, 100);
	return 0;
}
