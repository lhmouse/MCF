#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s;
	for(int i = 0; i < 80; ++i){
		s.Push(' ' + i);
	}
	std::puts(s.GetStr());
	auto s2 = std::move(s);
	std::puts(s.GetStr());
	return 0;
}

