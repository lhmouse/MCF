#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Thread/Thread.hpp>
using namespace MCF;

void foo(){
	DEBUG_THROW(Exception, "meow", 123);
}

extern "C" unsigned int MCFMain() noexcept {
	try {
		auto thread = Thread::Create(foo);
		thread->Join();
	} catch(Exception &e){
		std::printf("Exception: file = %s, line = %lu\n", e.GetFile(), e.GetLine());
	}
	return 0;
}
