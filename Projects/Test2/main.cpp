#include <MCF/StdMCF.hpp>
#include <cstdio>
using namespace MCF;

extern "C" void MCF_PreInitModule() noexcept {
	std::puts("meow");

	std::atexit([]{ std::puts("bark"); });
}

extern "C" unsigned int MCFMain() noexcept {
	return 0;
}
