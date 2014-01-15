#include <StdMCF.hpp>
//#include <Core/Format.hpp>
#include <Core/String.hpp>
#include <cstdio>

unsigned int MCFMain(){
	MCF::UTF8String u8s;
	u8s.Assign ("_第二部分_");
	u8s.Unshift("_第一部分_");
	u8s.Append ("_第三部分_");
	std::puts(MCF::ANSIString(u8s).GetCStr());
	return 0;
}
