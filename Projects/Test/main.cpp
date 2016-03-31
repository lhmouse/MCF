#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <MCF/Thread/Atomic.hpp>

extern "C" unsigned _MCFCRT_Main(){
#if 1
	auto t1 = MCF::GetHiResMonoClock();

	volatile unsigned val = 0;
	MCF::Mutex m(1000);
	MCF::Array<MCF::Thread, 100> threads;

	{
		auto l = m.GetLock();
		for(auto &t : threads){
			t.Create(
				[&]{
					for(unsigned i = 0; i < 100000; ++i){
						auto l = m.TryGetLock(UINT64_MAX);
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
#else
	MCF::Atomic<unsigned> tcnt(0);
	MCF::Mutex m;
	MCF::ConditionVariable cv;
	MCF::Array<MCF::Thread, 6> threads;

	for(auto &t : threads){
		t.Create(
			[&]{
				auto l = m.GetLock();
				std::printf("thread %lu waiting.\n", ::GetCurrentThreadId());
				::Sleep(500);

				cv.Wait(l, UINT64_MAX);
				std::printf("thread %lu signaled.\n", ::GetCurrentThreadId());
				::Sleep(500);

				tcnt.Decrement(MCF::kAtomicRelaxed);
			},
			false);
		tcnt.Increment(MCF::kAtomicRelaxed);
	}

	while(tcnt.Load(MCF::kAtomicRelaxed)){
		::Sleep(900);
		const auto cnt = cv.Broadcast();
		std::printf("signaled %zu threads!\n", cnt);
	}

	for(auto &t : threads){
		t.Join();
	}
#endif
	return 0;
}
