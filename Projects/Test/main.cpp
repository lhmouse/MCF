#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <MCF/Thread/Atomic.hpp>
#include <MCF/Thread/OnceFlag.hpp>
#include <cstdio>

MCF::OnceFlag fl;

extern "C" unsigned _MCFCRT_Main(){
	MCF::Atomic<bool> b;
	MCF::Mutex m(100);
	MCF::Array<MCF::Thread, 100> threads;
	{
		auto l = m.GetLock();
		for(auto &t : threads){
			t.Create(
				[&]{
					try {
						fl.CallOnce([&]{ auto r = m.GetLock(); /*std::puts("this should fail!");*/ throw 12345; });
					} catch(int e){
						{ auto r = m.GetLock(); /*std::printf("exception caught: e = %d\n", e);*/ }
					}

					fl.CallOnce([&]{ auto r = m.GetLock(); std::puts("this should succeed!"); b.Store(true, MCF::kAtomicRelaxed); });
					MCF_ASSERT(b.Load(MCF::kAtomicRelaxed));

					fl.CallOnce([&]{ auto r = m.GetLock(); std::puts("this should not happen!"); });
				},
				false);
		}
	}
	for(auto &t : threads){
		t.Join();
	}

	return 0;
}
