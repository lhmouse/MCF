#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <MCF/Thread/Thread.hpp>

extern "C" unsigned _MCFCRT_Main(){
#if 1
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

	std::printf("val = %u\n", val);
#else
	MCF::Mutex m;
	MCF::ConditionVariable cv;
	MCF::Array<MCF::Thread, 6> threads;

	for(auto &t : threads){
		t.Create(
			[&]{
				auto l = m.GetLock();
				std::printf("thread %lu waiting.\n", ::GetCurrentThreadId());
				::Sleep(500);

				cv.Wait(l, 100000000);
				std::printf("thread %lu signaled.\n", ::GetCurrentThreadId());
				::Sleep(500);
			},
			false);
	}

	for(;;){
		::Sleep(900);
		auto cnt = cv.Signal(2);
		if(cnt == 0){
			break;
		}
		std::printf("signaled %zu threads!\n", cnt);
	}

	for(auto &t : threads){
		t.Join();
	}
#endif
	return 0;
}
