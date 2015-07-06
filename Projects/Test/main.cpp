#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ReaderWriterMutex.hpp>
#include <MCF/Core/Time.hpp>

using namespace MCF;

ReaderWriterMutex rwm(1000);
Mutex m(1000);
volatile int c = 0;

extern "C" unsigned MCFMain(){
	std::array<IntrusivePtr<Thread>, 4> threads;
	for(auto &p : threads){
		p = Thread::Create([]{
			for(int i = 0; i < 500000; ++i){
//				const auto l = rwm.GetWriterLock();
				const auto l = m.GetLock();
				++c;
			}
		}, true);
	}

	const auto t1 = GetHiResMonoClock();
	for(auto &p : threads){
		p->Resume();
	}
	for(auto &p : threads){
		p->Join();
	}
	const auto t2 = GetHiResMonoClock();

	std::printf("c = %d, time = %f\n", c, t2 - t1);
	return 0;
}
