#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Utilities.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	AbsorbTuple([](int i){ std::printf("%d\n", i); }, std::make_tuple(1, 2, 3));
	ReverseAbsorbTuple([](int i){ std::printf("%d\n", i); }, std::make_tuple(1, 2, 3));
	SqueezeTuple([](auto ...i){ std::printf("%d %d %d\n", i...); }, std::make_tuple(1, 2, 3));
	ReverseSqueezeTuple([](auto ...i){ std::printf("%d %d %d\n", i...); }, std::make_tuple(1, 2, 3));
	return 0;
}
