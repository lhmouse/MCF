#include <MCF/StdMCF.hpp>
//#include <MCFCRT/env/gthread.h>
#include <MCF/Core/Array.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Core/Clocks.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	::__gthread_mutex_t m;
	::__gthread_mutex_init_function(&m);

	volatile unsigned val = 0;
	MCF::Array<MCF::Thread, 100> threads;

	auto t1 = MCF::GetHiResMonoClock();
	{
		for(auto &t : threads){
			t.Create(
				[&]{
					for(unsigned i = 0; i < 10000; ++i){
						::__gthread_mutex_lock(&m);
						val = val + 1;
						::__gthread_mutex_unlock(&m);
					}
					_MCFCRT_Bail(L"aas");
				},
				false);
		}
	}
	for(auto &t : threads){
		t.Join();
	}
	auto t2 = MCF::GetHiResMonoClock();
	std::printf("val = %u, delta_t = %f\n", val, t2 - t1);

	::__gthread_mutex_destroy(&m);
	return 0;
}
