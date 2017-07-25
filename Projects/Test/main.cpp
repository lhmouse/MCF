#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/CopyMoveFill.hpp>

using namespace MCF;

struct PageDeleter {
	constexpr void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(void *p) const noexcept {
		::VirtualFree(p, 0, MEM_RELEASE);
	}
};

using Char = char;

constexpr std::size_t kSize = 0x200F000;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1  = (Char *)((char *)p1.Get() + 6);
	const auto s1e = (Char *)((char *)p1.Get() + kSize);
	Fill(s1, s1e, 'a');
	s1e[-3] = 'b';
	s1e[-2] = 0;
	s1e[-1] = 'b';
	const auto s2  = (Char *)((char *)p2.Get() + 2);
	const auto s2e = (Char *)((char *)p2.Get() + kSize);
	Fill(s2, s2e, 'a');
	s2e[-3] = 'b';
	s2e[-2] = 0;
	s2e[-1] = 'b';

	const auto test = [&](WideStringView name){
		const auto fname = "memchr"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<char * (*)(const char *, int, std::size_t)>(fname);
			std::ptrdiff_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 100; ++i){
				r = (std::ptrdiff_t)(*pf)(s1, 'b', (std::size_t)(s1e - s1));
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
	test("UCRTBASE"_wsv);
	test("MCFCRT-2"_wsv);

	return 0;
}
