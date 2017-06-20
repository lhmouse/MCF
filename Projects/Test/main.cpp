#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main() noexcept {
	constexpr char text[] = "aqwebqwecqwedqwef", pattern[] = "cqwedqwe";
	auto pos = MCF::NarrowStringView(text).Find(MCF::NarrowStringView(pattern));
	__builtin_printf("pos = %d\n", (int)pos);
	return 0;
}
