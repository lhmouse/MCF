#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ReaderWriterLock.hpp>
#include <MCF/Thread/Thread.hpp>
using namespace MCF;

unsigned int MCFMain(){
	const auto rwl = ReaderWriterLock::Create();

	std::shared_ptr<Thread> threads[10];
	for(unsigned int i = 0; i < COUNT_OF(threads); ++i){
		threads[i] = Thread::Create([&, i]{
			::Sleep(100 + i * 10);
			auto lock1 = rwl->GetReaderLock();
			__builtin_printf("*    %u\n", i);

			::Sleep(100);
			auto lock2 = rwl->GetReaderLock();
			__builtin_printf(" *   %u\n", i);

			::Sleep(100);
			auto lock3 = rwl->GetReaderLock();
			__builtin_printf("  *  %u\n", i);

			::Sleep(100);
			auto lock4 = rwl->GetReaderLock();
			__builtin_printf("   * %u\n", i);

			::Sleep(100);
			lock1.Unlock();

			::Sleep(100);
			lock2.Unlock();

			::Sleep(100);
			lock3.Unlock();

			::Sleep(100);
			lock4.Unlock();
		});
	}

	Thread::Create([&]{
		::Sleep(130);
		auto lock1 = rwl->GetWriterLock();
		__builtin_printf("----\n");

		::Sleep(10);
		auto lock2 = rwl->GetWriterLock();
		__builtin_printf("----\n");

		::Sleep(1000);
		__builtin_printf("----\n");
		lock1.Unlock();

		::Sleep(1000);
		__builtin_printf("----\n");
		lock2.Unlock();
	})->Join();

	for(auto &p : threads){
		p->Join();
	}
	return 0;
}
