#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/LastError.hpp>
#include <MCF/Core/CopyMoveFill.hpp>
#include <MCF/Core/MinMax.hpp>

using namespace MCF;

struct PageDeleter {
	constexpr void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(void *p) const noexcept {
		::VirtualFree(p, 0, MEM_RELEASE);
	}
};

using Char = wchar_t;

constexpr std::size_t kSize = 0x1000000;

extern "C" unsigned _MCFCRT_Main(void) noexcept {

	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1b = (Char *)((char *)p1.Get() + 2);
	const auto s1e = (Char *)((char *)p1.Get() + kSize);
	const auto s2b = (Char *)((char *)p2.Get() + 4);
	const auto s2e = (Char *)((char *)p2.Get() + kSize);
	const auto len = (std::size_t)Min(s1e - s1b, s2e - s2b);
	for(std::size_t i = 0; i < len; ++i){
		s1b[i] = s2b[i] = (Char)(i | 1);
	}
	s1e[-3] = '\x0F';
	s1b[len - 2] = s2b[len - 2] = 0;

	const auto test = [&](WideStringView name){
		const auto fname = "wmemcmp"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<int (*)(const Char *, const Char *, std::size_t)>(fname);
			std::ptrdiff_t r;
			const auto t1 = GetHiResMonoClock();
			for(std::uint64_t i = 0; i < 1000; ++i){
				r = (std::ptrdiff_t)(*pf)(s1b, s2b, len);
			}
			const auto t2 = GetHiResMonoClock();
			std::printf("%-10s.%s : t = %f, r = %td\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), t2 - t1, r);
		} catch(Exception &e){
			std::printf("%-10s.%s : error %lu : %s\n", AnsiString(name).GetStr(), AnsiString(fname).GetStr(), e.GetErrorCode(), AnsiString(GetWin32ErrorDescription(e.GetErrorCode())).GetStr());
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
	static struct { char a[21]; char s[200]; } s1 = { "", "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW\0XYZ" };
	static struct { char a[ 1]; char s[200]; } s2 = { "", "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW\0XaZ" };
	const DynamicLinkLibrary dll(L"MCFCRT-2"_wsv);
	const auto pf = dll.RequireProcAddress<int (*)(const char *, const char *, std::size_t)>("strncmp"_nsv);
	std::printf("%d\n", pf(s1.s, s2.s, 62));
*/
	return 0;
}
