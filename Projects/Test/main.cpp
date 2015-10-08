#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	WideString ws(L"一二三四五67890");
	AnsiString as(ws);
	std::printf("str = %s$\n", as.GetStr());

	Utf8String u8s(as);
//	std::printf("u8s = %s$\n", u8s.GetStr());
	return 0;
}
