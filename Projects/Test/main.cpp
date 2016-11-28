#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Thunk.hpp>
#include <cstdio>

extern const char fn_begin __asm__("fn_begin"), fn_end __asm__("fn_end");

__asm__(
	"fn_begin: \n"
	"	mov eax, ecx \n"
	"	mul edx \n"
	"	ret \n"
	"fn_end: \n"
	"	nop \n"
);

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	const auto t = MCF::Thunk(&fn_begin, static_cast<std::size_t>(&fn_end - &fn_begin));
	const auto fn = t.Get<unsigned (__fastcall *)(unsigned, unsigned)>();
	std::printf("val = %u\n", fn(3, 5));
	return 0;
}
