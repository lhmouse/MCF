#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto so(U"𪚥𪚥喵"_u32s);
	ModUtf8String mu8s(so);
	::MessageBoxW(NULL, WideString(mu8s).GetCStr(), NULL, MB_ICONINFORMATION);
	return 0;
}
