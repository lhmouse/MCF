#include <MCFCRT/ext/repz_cmps.h>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main() noexcept {
	const char *s1 = "xxxxxxacc";
	const char *s2 = "xxxxxxbcc";
	int r = ::_MCFCRT_repz_cmpsb(nullptr, s1, s2, 10);
	if(r != 0){
		std::puts("neq");
	} else {
		std::puts("eq");
	}
	return 0;
}
