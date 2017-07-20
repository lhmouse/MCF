#include <MCF/Streams/StandardErrorStream.hpp>
#include <MCFCRT/MCFCRT.h>

volatile long double val = 1234.5678;

extern "C" unsigned _MCFCRT_Main() noexcept {
	using namespace MCF;

	char str[100];
	auto eptr = ::_MCFCRT_itoa_u(str, (unsigned)val);
	const auto err_s = MakeIntrusive<StandardErrorStream>();
	err_s->Put(str, (unsigned)(eptr - str));
	return 0;
}
