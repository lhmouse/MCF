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

constexpr std::size_t kSize = 0x10000000;

extern "C" unsigned _MCFCRT_Main(void) noexcept {

	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1b  = (Char *)((char *)p1.Get() + 2);
	const auto s1e = (Char *)((char *)p1.Get() + kSize);
	for(auto p = s1b; p != s1e; ++p){
		*p = (Char)((p - s1b) | 1);
	}
//	s1e[-3] = 'a';
//	s1e[-2] = 0;
//	s1e[-1] = 'c';
	const auto s2b  = (Char *)((char *)p2.Get() + 4);
	const auto s2e = (Char *)((char *)p2.Get() + kSize);
	for(auto p = s2b; p != s2e; ++p){
		*p = (Char)((p - s2b) | 1);
	}
//	s2e[-3] = 'a';
//	s2e[-2] = 0;
	s2e[-1] = 'c';

	const auto test = [&](WideStringView name){
		const auto fname = "memcmp"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<int (*)(const Char *, const Char *, std::size_t)>(fname);
			std::ptrdiff_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 30; ++i){
				r = (std::ptrdiff_t)(*pf)(s2b, s1b, (std::size_t)(s2e - s2b));
			}
			const auto t2 = GetHiResMonoClock();
			std::printf("%-10s.%s : t = %f, r = %td\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), t2 - t1, r);
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
/*
	static struct { char a[31]; char s[100]; } s1 = { "",  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	static struct { char a[ 1]; char s[100]; } s2 = { "",  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	static volatile auto fp = __builtin_memcmp;
	std::printf("%d\n", fp(s1.s, s2.s, 62));
*/
	return 0;
}
