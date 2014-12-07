#include <MCF/StdMCF.hpp>
#include <MCF/Thread/ReaderWriterLock.hpp>
#include <MCF/Thread/Thread.hpp>
#include <vector>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	CriticalSection m;
	int val = 0;
	std::vector<IntrusivePtr<Thread>> v(6);
	for(unsigned i = 0; i < v.size(); ++i){
		v.at(i) = Thread::Create([&, i]{
			::Sleep(100);
			for(int j = 0; j < 100; ++j){
				auto l = m.GetLock();
				while(!l){
					l.Try();
				}
				auto l2 = m.TryLock();
				const int old = val;
				std::printf("thread %u\n", i);
				::Sleep(100);
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
