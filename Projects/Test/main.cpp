#include <windows.h>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	auto h = ::LoadLibraryW(L"msvcr100.dll");
	::FreeLibrary(h);
	return 0;
}
