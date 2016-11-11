#include <MCF/Core/Function.hpp>
#include <cstdio>

int make_int(){
	return 12345;
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Function<const int & ()> fn;
	fn = make_int;
	std::printf("result = %d\n", fn());
	return 0;
}
