#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/LastError.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	SetWin32LastError(ERROR_PROCESS_ABORTED);
	auto wcs = GetWin32ErrorDescription();
	std::cout <<AnsiString(wcs).GetStr() <<std::endl;
	return 0;
}
