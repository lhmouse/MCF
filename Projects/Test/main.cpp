#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Variant.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Variant<char, int, double> v;
	auto v2 = v;
	return 0;
}
