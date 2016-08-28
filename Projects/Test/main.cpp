#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>

struct PageDeleter {
	constexpr void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(void *p) const noexcept {
		::VirtualFree(p, 0, MEM_RELEASE);
	}
};

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

//	NarrowString s1, s2;
//	s1.Append('b', 0x1000005);
//	s1.Append('a');
//	s2.Append('b', 0x1000002);
//	s2.Append('b');

	constexpr std::size_t kStringSize = 0xF000;
	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, kStringSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, kStringSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1  = (char *)p1.Get() + 1;
	const auto s1e = (char *)p1.Get() + kStringSize;
	const auto s2  = (char *)p2.Get() + 6;
	const auto s2e = (char *)p2.Get() + kStringSize;

	std::memset(s1, 'a', (std::size_t)(s1e - s1));
	s1e[-1] = 0;
	std::memset(s2, 'a', (std::size_t)(s2e - s2));
	s2e[-1] = 0;

	const auto test = [&](WideStringView name){
		const auto fname = "strcmp"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<int (*)(const char *, const char *)>(fname);
			std::intptr_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 100000; ++i){
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
	test("MCFCRT-9"_wsv);

	return 0;
}
