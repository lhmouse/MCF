#include <MCF/StdMCF.hpp>
#include <MCF/Core/LastError.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::printf("%s\n", AnsiString(GetWin32ErrorDescription(ERROR_HANDLE_EOF)).GetCStr());
	return 0;
}
