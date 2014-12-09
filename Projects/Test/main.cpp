#include <MCF/StdMCF.hpp>
#include <MCF/Thread/MonitorPtr.hpp>
#include <MCF/Thread/Thread.hpp>
#include <vector>
using namespace MCF;

struct m_int {
	int val = 0;

	void increment(unsigned i){
		const int old = val;
		std::printf("thread %u\n", i);
		::Sleep(10);
		val = old + 1;
	}
};

extern "C" unsigned int MCFMain() noexcept {
	MonitorPtr<m_int> mp;
	std::vector<IntrusivePtr<Thread>> v(6);
	for(unsigned i = 0; i < v.size(); ++i){
		v.at(i) = Thread::Create([&, i]{
			::Sleep(10);
			for(unsigned j = 0; j < 50; ++j){
				mp->increment(i);
			}
		});
	}
	for(auto & p: v){
		p->Join();
	}
	std::printf("val: %d\n", mp->val);
	return 0;
}
