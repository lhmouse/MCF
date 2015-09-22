#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ReaderWriterMutex.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	volatile int cnt = 0;
	ReaderWriterMutex mtx;

	auto l = [&]{
		for(int i = 0; i < 100000; ++i){
			ReaderWriterMutex::UniqueWriterLock l(mtx);
			int old = cnt;
			cnt = old + 1;
		}
	};

	auto thread = Thread::Create(l);
	l();
	thread->Join();

	std::printf("cnt = %d\n", cnt);

	return 0;
}
