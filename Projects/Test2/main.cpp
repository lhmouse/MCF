#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto ws = GetWin32ErrorDesc(10);
	std::puts((AnsiString(L"meow "_wso + ws)).GetCStr());
	return 0;
}
