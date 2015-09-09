#include <MCF/Core/String.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	auto s = "____1___1__1_1__1___1____1_____1______1"_u8so;
	std::printf("offset = %zu\n", s.Find("______1"_u8so));
	return 0;
}
