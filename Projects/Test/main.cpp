#include <MCFCRT/MCFCRT.h>

extern "C" unsigned _MCFCRT_Main() noexcept {
	const auto ptr = static_cast<char *>(::_MCFCRT_malloc(10));
	_MCFCRT_ASSERT(ptr);
	ptr[10] = 'a';
	::_MCFCRT_free(ptr);
	return 0;
}
