#include <MCF/StdMCF.hpp>
#include <MCF/Core/Thunk.hpp>

extern const char from __asm__("FROM"), to __asm__("TO");

__asm__(
	"FROM: \n"
	"	lea eax, dword ptr[ecx + edx] \n"
	"	ret \n"
	"TO: \n"
);

unsigned int MCFMain(){
	const std::size_t cb = &to - &from;
	auto p = MCF::AllocateThunk(cb);
	__builtin_memcpy(p.get(), &from, cb);

	int res = (*(int (__fastcall *)(int, int))p.get())(4, 6);
	std::printf("res = %d\n", res);

	return 0;
}
