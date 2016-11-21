#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

//	NarrowString s1, s2;
//	s1.Append('b', 0x1000005);
//	s1.Append('a');
//	s2.Append('b', 0x1000002);
//	s2.Append('b');

	struct PageDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *p) const noexcept {
			::VirtualFree(p, 0, MEM_RELEASE);
		}
	};
	constexpr std::size_t kStringSize = 0x200F000;
	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, kStringSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, kStringSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1  = (wchar_t *)((char *)p1.Get() + 2);
	const auto s1e = (wchar_t *)((char *)p1.Get() + kStringSize);
	const auto s2  = (wchar_t *)((char *)p2.Get() + 22);
	const auto s2e = (wchar_t *)((char *)p2.Get() + kStringSize);

	std::wmemset(s1, 'a', (std::size_t)(s1e - s1));
	s1e[-1] = 0;
	std::wmemset(s2, 'a', (std::size_t)(s2e - s2));
	s2e[-1] = 0;

	const auto test = [&](WideStringView name){
		const auto fname = "wcscmp"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<int (*)(const wchar_t *, const wchar_t *)>(fname);
			std::intptr_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 100; ++i){
				r = (std::intptr_t)(*pf)(s1, s2);
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
	test("MCFCRT-9"_wsv);

	return 0;
}
