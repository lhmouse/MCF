#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	NarrowString s1, s2;
	s1.Append('a', 0x100000);
	s1.Append(1);
	s2.Append('a', 0x100000);
	s2.Append(-1);

	const auto fname = "strcmp"_nsv;

	const auto test = [&](WideStringView name){
		const DynamicLinkLibrary dll(name);
		const auto pf = dll.RequireProcAddress<int (*)(const void *, const void *)>(fname);
		std::intptr_t r;
		const auto t1 = GetHiResMonoClock();
		for(unsigned i = 0; i < 10000; ++i){
			r = (*pf)(s1.GetStr(), s2.GetStr());
		}
		const auto t2 = GetHiResMonoClock();
		std::printf("%-10s.%s : t2 - t1 = %f, r = %td\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), t2 - t1, r);
	};

	test("NTDLL"_wsv);
	test("MSVCRT"_wsv);
	test("MSVCR100"_wsv);
	test("MCFCRT-9"_wsv);

	return 0;
}
