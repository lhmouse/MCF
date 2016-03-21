#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>

extern "C" unsigned _MCFCRT_Main(){
	MCF::Function<void (int)> fn;
	fn = [](int a){ std::printf("a = %d\n", a); };
	fn(123);
	return 0;
}
