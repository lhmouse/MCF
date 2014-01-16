#include <StdMCF.hpp>
//#include <Core/Format.hpp>
#include <Core/String.hpp>
#include <cstdio>

unsigned int MCFMain(){
	MCF::UTF8String u8s;
	for(char i = 'z' + 1; i-- != 'a'; ){
		u8s.Unshift(i);
	}
	std::puts(MCF::ANSIString(u8s).GetCStr());
	return 0;
}
