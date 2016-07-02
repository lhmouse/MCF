#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/module.h>
#include <MCFCRT/env/thread_env.h>

extern "C" unsigned _MCFCRT_Main(void){
	for(int i = 0; i < 10; ++i){
		::_MCFCRT_AtThreadExit([](auto n){ __builtin_printf("at_thread_exit_proc(%d)\n", (int)n); }, i);
		::_MCFCRT_AtModuleExit([](auto n){ __builtin_printf("at_module_exit_proc(%d)\n", (int)n); }, i);
	}
	return 0;
}
