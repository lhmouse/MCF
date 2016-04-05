#include <MCF/StdMCF.hpp>
#include <MCF/Thread/OnceFlag.hpp>
#include <cstdio>

MCF::OnceFlag fl;

extern "C" unsigned _MCFCRT_Main(){
	try {
		fl.CallOnce([]{ std::puts("this should fail!"); throw 12345; });
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	fl.CallOnce([]{ std::puts("this should succeed!"); });
	fl.CallOnce([]{ std::puts("this should not happen!"); });
	return 0;
}
