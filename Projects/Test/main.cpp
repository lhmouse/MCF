#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	NarrowString s1, s2;
	s1.Append('a', 0x1000000);
	s1.Append('v');
	s2.Append('a', 0x1000000);
	s2.Append('c');

	const auto fname = "memmove"_nsv;

	const auto test = [&](WideStringView name){
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<void * (*)(void *, const void *, std::size_t)>(fname);
			std::intptr_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 1000; ++i){
				r = (std::intptr_t)(*pf)(s1.GetStr(), s2.GetStr(), s1.GetSize());
			}
			const auto t2 = GetHiResMonoClock();
			std::printf("%-10s.%s : t2 - t1 = %f, r = %td\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), t2 - t1, r);
		} catch(Exception &e){
			std::printf("%-10s.%s : error %lu\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), e.GetErrorCode());
		}
	};

	test("NTDLL"_wsv);
	test("MSVCRT"_wsv);
	test("MSVCR100"_wsv);
	test("MSVCR110"_wsv);
	test("MSVCR120"_wsv);
	test("MCFCRT-9"_wsv);

	return 0;
}
