#include <MCF/StdMCF.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	const auto fname = "memchr"_nsv;

	NarrowString s;
	s.Append('a', 0x100000);
	s.Append('b');

	const auto test = [&](WideStringView name){
		const DynamicLinkLibrary dll(name);
		const auto pf = dll.RequireProcAddress<void * (*)(const void *, int, std::size_t)>(fname);
		void *p;
		const auto t1 = GetHiResMonoClock();
		for(unsigned i = 0; i < 10000; ++i){
			p = (*pf)(s.GetData(), 'b', s.GetSize());
		}
		const auto t2 = GetHiResMonoClock();
		std::printf("%-10s.%s : t2 - t1 = %f, p = %p\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), t2 - t1, p);
	};

	test("NTDLL"_wsv);
	test("MSVCRT"_wsv);
	test("MSVCR100"_wsv);
	test("MCFCRT-9"_wsv);

	return 0;
}
