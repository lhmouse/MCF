#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/LastError.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	try {
		GetWin32ErrorDescription(0x123456);
	} catch(SystemError &e){
		std::printf("code = %lX\n", e.GetCode());
	}
	return 0;
}
