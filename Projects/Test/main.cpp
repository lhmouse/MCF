#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/RecursiveMutex.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	RecursiveMutex m;
	m.Lock();
	auto p1 = Thread::Create([&]{
		::Sleep(1000);
		std::puts("Thread1 unlocking...");
		m.Unlock();
	});
	auto p2 = Thread::Create([&]{
		::Sleep(2000);
		std::puts("Thread2 locking...");
		m.Lock();

		::Sleep(5000);
		std::puts("Thread2 unlocking...");
		m.Unlock();
	});
	auto p3 = Thread::Create([&]{
		::Sleep(3000);
		std::puts("Thread3 locking...");
		m.Lock();

		std::puts("Thread3 unlocking...");
		m.Unlock();
	});

	p1->Join();
	p2->Join();
	p3->Join();

	return 0;
}
