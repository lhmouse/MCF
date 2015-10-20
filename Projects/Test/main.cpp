#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Core/Time.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Core/LastError.hpp>
#include <MCF/Core/String.hpp>

using namespace MCF;

extern "C" unsigned MCFMain()
try {
	Mutex m1;
	volatile int c = 0;

	Array<IntrusivePtr<Thread>, 10> threads;
	for(auto &p : threads){
		p = Thread::Create([&]{
			for(int i = 0; i < 10000; ++i){
				m1.Lock();
				++c;
				m1.Unlock();
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
} catch(Exception &e){
	std::printf("MCF::Exception: code = %lu (%s), desc = %s\n", e.GetCode(), AnsiString(GetWin32ErrorDescription(e.GetCode())).GetStr(), e.GetDescription());
	return 3;
}
