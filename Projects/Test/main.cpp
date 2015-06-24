#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s;
	std::printf("size = %zu, cap = %zu\n", sizeof(s), s.GetCapacity());
	return 0;
}

