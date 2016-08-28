#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	WideString s1, s2;
	s1.Append('a', 0x1000002);
	s1.Append('b');
	s2.Append('a', 0x1000002);
	s2.Append('b');

	const auto test = [&](WideStringView name){
		const auto fname = "wcsncmp"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<int (*)(const wchar_t *, const wchar_t *, std::size_t)>(fname);
			std::intptr_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 100; ++i){
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
