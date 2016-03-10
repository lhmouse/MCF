#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Defer.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	try {
		auto d1 = DeferOnNormalExit([]{ std::puts("deferred on normal exit!"); });
		auto d2 = DeferOnException([]{ std::puts("deferred on exception!"); });
		std::puts("normal code!");
	//	throw 123;
	} catch(int e){
		std::printf("exception caught! e = %d\n", e);
	}
	return 0;
}
