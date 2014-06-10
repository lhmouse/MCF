#include <MCF/StdMCF.hpp>
#include <MCF/Thread/CriticalSection.hpp>
#include <MCF/Thread/Thread.hpp>
using namespace MCF;

unsigned int MCFMain(){
	std::shared_ptr<Thread> threads[200];
	auto lock = CriticalSection::Create();

	for(auto &p : threads){
		p = Thread::Create([&]{
			::Sleep(1000);
			auto v = lock->GetLock();
			::Sleep(100);
		});
	}
	for(auto &p : threads){
		p->Join();
	}
	return 0;
}
