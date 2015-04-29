#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s1("hello world! 喵喵喵喵喵喵");
	for(unsigned i = 0; i < 20; ++i){
		s1 += s1;
	}
	Utf16String s2(s1);
	return 0;
}
