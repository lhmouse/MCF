#include <MCF/StdMCF.hpp>
#include <MCF/Thread/CriticalSection.hpp>
#include <MCF/Thread/Thread.hpp>
#include <vector>
using namespace MCF;

auto cs = CriticalSection::Create(0);

extern "C" unsigned int MCFMain() noexcept {
	int val = 0;
	std::vector<IntrusivePtr<Thread>> v(6);
	for(auto & p: v){
		p = Thread::Create([&]{
			::Sleep(100);
			for(int i = 0; i < 10; ++i){
				auto l = cs->GetLock();
				const int old = val;
				std::printf("thread %lu\n", Thread::GetCurrentId());
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
