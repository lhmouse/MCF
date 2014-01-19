#include <StdMCF.hpp>
#include <Core/String.hpp>
#include <cstdio>
#include <cstdlib>

unsigned int MCFMain(){
	MCF::UTF16String s1, s2;
	s1 = L"looooooooooooooooooooooooooooooooooooooooong";
	s2 = L"meow";

	std::puts("---------- before swap ----------");
	s1.Swap(s2);
	std::puts("---------- after swap ----------");

	std::fputws(s1.GetCStr(), stdout);
	std::fputws(s2.GetCStr(), stdout);

	return 0;
}
