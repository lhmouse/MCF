#include <MCF/Core/RefCountingNtmbs.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	auto s1 = MCF::RefCountingNtmbs::Copy("hello world!");
	auto s2 = MCF::RefCountingNtmbs::View("hello world!");
	auto s3 = s1;
	auto s4 = s2;
	std::printf("s1 = %p\n", (const void *)s1.GetStr());
	std::printf("s2 = %p\n", (const void *)s2.GetStr());
	std::printf("s3 = %p\n", (const void *)s3.GetStr());
	std::printf("s4 = %p\n", (const void *)s4.GetStr());
	return 0;
}
