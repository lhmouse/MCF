#include <MCF/Core/Array.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/ReadersWriterMutex.hpp>

using namespace MCF;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	volatile unsigned c = 0;
	ReadersWriterMutex m;
	Array<IntrusivePtr<Thread>, 20> ts;
	for(auto &t : ts){
		t = MakeThread([&]{
			for(unsigned i = 0; i < 100000; ++i){
				auto l = m.GetLockAsWriter();
				auto x = c;
				__builtin_ia32_pause();
				c = x + 1;
			}
		}, false);
	}
	for(auto &t : ts){
		t->Wait();
	}
	std::printf("c = %u\n", c);
	return 0;
}
