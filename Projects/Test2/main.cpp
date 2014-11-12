#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto so(u8"𪚥11𪚥22喵33"_u8s);
	Cesu8String mu8s(so);
	::MessageBoxW(NULL, WideString(mu8s).GetCStr(), NULL, MB_ICONINFORMATION);
	return 0;
}
