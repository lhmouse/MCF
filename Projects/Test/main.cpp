#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Defer.hpp>
#include <MCF/Core/Exception.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	try {
		auto d1 = DeferOnNormalExit([]{ std::puts("deferred on normal exit!"); });
		auto d2 = DeferOnException([]{ std::puts("deferred on exception!"); });
		std::puts("normal code!");
		DEBUG_THROW_NESTED(Exception, ERROR_INVALID_PARAMETER, Rcntws::Copy(L"hello world!"));
	} catch(std::exception &e){
		std::printf("exception caught! e = %s\n", e.what());
	}
	return 0;
}
