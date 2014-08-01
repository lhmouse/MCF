#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Exception.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::printf("%s\n", AnsiString(GetWin32ErrorDesc(ERROR_INVALID_PARAMETER)).GetCStr());
	return 0;
}
