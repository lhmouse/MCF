#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Thread/Thread.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(){
	auto t1 = MCF::GetHiResMonoClock();

	volatile unsigned val = 0;
	MCF::Mutex m(100);
	MCF::Array<MCF::Thread, 100> threads;

	{
		auto l = m.GetLock();
		for(auto &t : threads){
			t.Create(
				[&]{
					for(unsigned i = 0; i < 100000; ++i){
						auto l = m.GetLock();
						val = val + 1;
					}
				},
				false);
		}
	}
	for(auto &t : threads){
		t.Join();
	}

	auto t2 = MCF::GetHiResMonoClock();
	std::printf("val = %u, delta_t = %f\n", val, t2 - t1);
	return 0;
}
