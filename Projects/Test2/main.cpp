#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ReaderWriterLock.hpp>
using namespace MCF;

const auto lock = ReaderWriterLock::Create();

extern "C" unsigned int MCFMain() noexcept {
	const auto thread = Thread::Create([]{
		::Sleep(2000);
		std::printf("child will try reader lock\n");
		auto l = lock->TryReaderLock();
		std::printf("  result = %d\n", (bool)l);

		::Sleep(1000);
		std::printf("child will try lock\n");
		l.Try();
		std::printf("  result = %d\n", (bool)l);

		::Sleep(2000);
		std::printf("child will try lock\n");
		l.Try();
		std::printf("  result = %d\n", (bool)l);
	});

	::Sleep(1000);
	std::printf("main will get writer lock\n");
	auto l = lock->GetWriterLock();

	::Sleep(3000);
	std::printf("main will release lock\n");
	l.Unlock();

	thread->Join();
	return 0;
}
