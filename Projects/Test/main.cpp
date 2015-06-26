#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/TupleManipulators.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::tuple<int, int, char, double, double> t;

	std::printf("%zu\n", FindFirstType<int>(t));
	std::printf("%zu\n", FindLastType<int>(t));
	std::printf("%zu\n", FindFirstType<double>(t));
	std::printf("%zu\n", FindLastType<double>(t));

	return 0;
}
