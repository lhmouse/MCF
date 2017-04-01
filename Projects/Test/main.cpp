#include <MCFCRT/ext/repz_cmps.h>
#include <MCFCRT/ext/repnz_cmps.h>
#include <cstdio>
#include <cstring>

char s1[100], s2[100];

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	std::memset(s1, 2, sizeof(s1));
	std::memset(s2, 1, sizeof(s2));
	s2[60] = 2;
	void *p;
	int r = ::_MCFCRT_repnz_cmpsb(&p, s1, s2, sizeof(s1));
	std::printf("p = %p, off = %td, r = %d\n", p, (char *)p - s1, r);
	return 0;
}
