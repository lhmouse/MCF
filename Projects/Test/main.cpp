#include <MCF/StdMCF.hpp>
#include <MCF/Thread/CallOnce.hpp>

using namespace MCF;

OnceFlag fl;

void print(const char *s){
	if(std::strcmp(s, "abc") == 0){
		throw 100;
	}
	std::puts(s);
}

extern "C" unsigned MCFMain(){
	try {
		CallOnce(fl, print, "abc");
	} catch(int e){
		std::printf("exception: %d\n", e);
	}
	try {
		CallOnce(fl, print, "def");
	} catch(int e){
		std::printf("exception: %d\n", e);
	}

	return 0;
}
