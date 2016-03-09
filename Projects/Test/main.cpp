#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto a = (std::puts("hello") == 0) ? 1 : DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, ""_rcs);
	std::printf("a = %d\n", a);
	return 0;
}
