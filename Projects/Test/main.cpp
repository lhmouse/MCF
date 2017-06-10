#include <MCFCRT/mcfcrt.h>

extern "C" unsigned _MCFCRT_Main() noexcept {
	auto p = static_cast<char *>(::_MCFCRT_malloc(10));
	// ::_MCFCRT_free(p);
	auto p2 = static_cast<char *>(::_MCFCRT_malloc(100));
	p2[100] = 'a';
	::_MCFCRT_free(p2);
	return 0;
}
