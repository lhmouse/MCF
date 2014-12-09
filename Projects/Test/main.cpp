#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ReaderWriterMutex.hpp>
#include <MCF/Thread/UniversalConditionVariable.hpp>
#include <MCF/Thread/Thread.hpp>
#include <vector>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	UniversalConditionVariable cv;
	cv.Broadcast();

	ReaderWriterMutex m;
	int val = 0;
	std::vector<IntrusivePtr<Thread>> v(6);
	for(unsigned i = 0; i < v.size(); ++i){
		v.at(i) = Thread::Create([&, i]{
			::Sleep(10);
			for(int j = 0; j < 50; ++j){
				auto l = m.GetWriterLock();
				auto l2 = m.GetReaderLock();
				const int old = val;
				std::printf("thread %u\n", i);
				::Sleep(10);
				val = old + 1;
			}
		});
	}
	for(auto & p: v){
		p->Join();
	}
	std::printf("val: %d\n", val);
	return 0;
}
