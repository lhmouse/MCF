#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <MCF/Thread/Atomic.hpp>
#include <MCF/Thread/Thread.hpp>

extern "C" unsigned _MCFCRT_Main(){
#if 0
	const auto t1 = MCF::GetHiResMonoClock();

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

	const auto t2 = MCF::GetHiResMonoClock();
	std::printf("val = %u, dt = %f\n", val, t2 - t1);
#else
	MCF::Mutex m;
	MCF::ConditionVariable cv;
	MCF::Array<MCF::Thread, 6> threads;

	MCF::Atomic<unsigned> cnt(0);

	for(auto &t : threads){
		t.Create(
			[&]{
				auto l = m.GetLock();
				std::printf("thread %lu waiting.\n", ::GetCurrentThreadId());
				::Sleep(500);

				cv.Wait(l, UINT64_MAX);
				std::printf("thread %lu signaled.\n", ::GetCurrentThreadId());
				::Sleep(500);

				cnt.Decrement(MCF::kAtomicRelaxed);
			},
			false);

		cnt.Increment(MCF::kAtomicRelaxed);
	}

	while(cnt.Load(MCF::kAtomicRelaxed) != 0){
		::Sleep(1900);
		auto signaled = cv.Broadcast();
		std::printf("signaled %zu threads!\n", signaled);
	}

	for(auto &t : threads){
		t.Join();
	}
#endif
	return 0;
}
