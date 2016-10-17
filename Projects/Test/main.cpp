#include <MCF/Core/String.hpp>
#include <MCFCRT/env/mcfwin.h>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	::RaiseException(1, 0, 0, nullptr);
	try {
		MCF::Utf8String s;
		s.Push('\xFF');
		MCF::Utf16String s2(s);
	} catch(std::exception &e1){
		auto &e = dynamic_cast<MCF::Exception &>(e1);
		std::printf("MCF::Exception: code = %lu, msg = %s\n", e.GetErrorCode(), MCF::AnsiString(MCF::WideStringView(e.GetErrorMessage())).GetStr());
	}
	return 0;
}
