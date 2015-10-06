#include <MCF/Core/LastError.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/StdMCF.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	std::puts(AnsiString(GetWin32ErrorDescription(ERROR_INVALID_PARAMETER)).GetStr());

	return 0;
}
