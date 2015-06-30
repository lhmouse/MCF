#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>

using namespace MCF;

Mutex m;
volatile int c = 0;

extern "C" unsigned MCFMain(){
	std::array<IntrusivePtr<Thread>, 100> threads;
	for(auto &p : threads){
		p = Thread::Create([]{
			for(int i = 0; i < 10000; ++i){
				const Mutex::UniqueLock l(m);
				++c;
			}
		});
	}
	for(auto &p : threads){
		p->Join();
	}
	std::printf("c = %d\n", c);
	return 0;
}
