#include <MCF/Core/String.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	try {
		MCF::Utf8String s;
		s.Push('\xFF');
		MCF::Utf16String s2(s);
	} catch(MCF::Exception &e){
		std::printf("MCF::Exception: code = %lu, msg = %s\n", e.GetErrorCode(), MCF::AnsiString(MCF::WideStringView(e.GetErrorMessage())).GetStr());
	}
	return 0;
}
