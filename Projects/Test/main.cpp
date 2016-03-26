#include <MCF/StdMCF.hpp>
#include <MCF/Function/FunctionView.hpp>

extern "C" unsigned _MCFCRT_Main(){
	MCF::FunctionView<void (int, int)> fv;
	fv = [](int a, int b){ return a + b; };
	fv(1, 2);
	return 0;
}
