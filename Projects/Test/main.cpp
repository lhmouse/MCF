#include <MCF/StdMCF.hpp>
#include <MCF/Thread/CallOnce.hpp>

MCF::OnceFlag fl;

extern "C" unsigned _MCFCRT_Main(){
	try {
		MCF::CallOnce(fl, []{ throw 1; });
	} catch(int){
		__builtin_puts("exception!");
	}
	MCF::CallOnce(fl, []{ __builtin_puts("called!"); });
	MCF::CallOnce(fl, []{ __builtin_puts("not called!");});
	return 0;
}
