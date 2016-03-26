#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/Thread.hpp>

extern "C" unsigned _MCFCRT_Main(){
	volatile unsigned val = 0;
	MCF::Mutex m;
	MCF::Array<MCF::Thread, 10> threads;

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
	for(auto &t : threads){
		t.Join();
	}

	std::printf("val = %u\n", val);
	return 0;
}
