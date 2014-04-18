#include <MCF/StdMCF.hpp>
#include <MCF/Core/Thread.hpp>
#include <MCF/Core/CriticalSection.hpp>
using namespace MCF;

static const auto lock = CriticalSection::Create();

struct S : NO_COPY {
	S(int i, char c, double d){
		CRITICAL_SECTION_SCOPE(lock){
			std::printf("ctor in thread %lu\n", ::GetCurrentThreadId());
			std::printf("  params (%d, %c, %f)\n", i, c, d);
		}
	}
	~S(){
		CRITICAL_SECTION_SCOPE(lock){
			std::printf("dtor in thread %lu\n", ::GetCurrentThreadId());
		}
	}

	void bark(){
		CRITICAL_SECTION_SCOPE(lock){
			std::printf("bark in thread %lu\n", ::GetCurrentThreadId());
		}
	}
};

auto s = MakeThreadLocal<S>(1, 'a', 2.0);

void foo(){
	s->bark();
}

unsigned int MCFMain(){
	auto pthread = Thread::Create(&foo);
	foo();
	pthread->Wait();

	return 0;
}
