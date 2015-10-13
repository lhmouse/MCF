#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <MCF/Core/Time.hpp>

using namespace MCF;

Mutex m;
volatile int c = 0;

ConditionVariable cv;

extern "C" unsigned MCFMain(){
	std::array<IntrusivePtr<Thread>, 4> threads;
	for(auto &p : threads){
		p = Thread::Create([]{
			std::puts("sleep on cv!");
			Mutex::UniqueLock l(m);
			cv.Wait(l);
			std::puts("wake up!");
		}, true);
	}

	const auto t1 = GetHiResMonoClock();
	for(auto &p : threads){
		p->Resume();
	}
	for(int i = 0; i < 10; ++i){
		::Sleep(1000);
		std::puts("- signal one!");
		cv.Signal();
	}
	for(auto &p : threads){
		p->Join();
	}
	const auto t2 = GetHiResMonoClock();

	std::printf("c = %d, time = %f\n", c, t2 - t1);
	return 0;
}
