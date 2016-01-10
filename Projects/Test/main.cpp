#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/Array.hpp>
#include <MCF/Core/LastError.hpp>
#include <MCF/Core/String.hpp>

using namespace MCF;

Mutex m;
volatile long long c = 0;
Array<IntrusivePtr<Thread>, 500> threads;

extern "C" unsigned MCFMain()
try {
	for(auto &p : threads){
		p = Thread::Create([&]{
			for(int i = 0; i < 10000; ++i){
				m.Lock();
				++c;
				m.Unlock();
			}
		});
	}

	const auto t1 = GetHiResMonoClock();
	for(auto &p : threads){
		p->Wait();
	}
	const auto t2 = GetHiResMonoClock();

	std::printf("c = %lld, time = %f\n", c, t2 - t1);
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
	std::printf("MCF::Exception: code = %lu (%s), desc = %s\n", e.GetCode(), AnsiString(GetWin32ErrorDescription(e.GetCode())).GetStr(), e.GetErrorMessage());

	for(auto &p : threads){
		if(p){
			p->Wait();
		}
	}

	return 3;
}
