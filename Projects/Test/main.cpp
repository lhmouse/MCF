#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ReaderWriterMutex.hpp>
#include <MCF/Core/Time.hpp>
#include <MCF/Core/Array.hpp>

using namespace MCF;

Mutex m;
volatile int c = 0;

extern "C" unsigned MCFMain(){

	Array<IntrusivePtr<Thread>, 10> threads;
	for(auto &p : threads){
		p = Thread::Create([]{
			for(int i = 0; i < 50000; ++i){
				{
					const auto l = m.GetLock();
					++c;
				}
				{
					const auto l = m.GetLock();
					++c;
				}
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
/*
	auto now = GetFastMonoClock();
	auto t1 = GetHiResMonoClock();
	auto l = m.TryGetLock(now + 1000);
	auto t2 = GetHiResMonoClock();
	std::printf("locked? %d  time = %f\n", l.IsLocking(), t2 - t1);

	now = GetFastMonoClock();
	t1 = GetHiResMonoClock();
	l = m.TryGetLock(now + 1000);
	t2 = GetHiResMonoClock();
	std::printf("locked? %d  time = %f\n", l.IsLocking(), t2 - t1);
*/
	return 0;
}
