#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned MCFCRT_Main(){
	MCF::Utf8String s1("hello world!");
	MCF::Utf32String s2(s1);
	return 0;
}
