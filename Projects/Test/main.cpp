#include <MCF/Thread/CallOnce.hpp>
#include <cstdio>

MCF::OnceFlag fl;

extern "C" unsigned MCFMain(){
	try {
		MCF::CallOnce(fl, [](auto s){ std::puts(s); throw 123; }, "call 1");
	} catch(int e){
		std::printf("exception! e = %d\n", e);
	}
	try {
		MCF::CallOnce(fl, [](auto s){ std::puts(s); }, "call 2");
	} catch(int e){
		std::printf("exception! e = %d\n", e);
	}
	return 0;
}
