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

using Char = char;
constexpr std::size_t size = 0x10000000;

extern "C" unsigned _MCFCRT_Main(void) noexcept {

	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1b = (Char *)((char *)p1.Get() + 4);
	const auto s1e = (Char *)((char *)p1.Get() + size);
	const auto s2b = (Char *)((char *)p2.Get() + 2);
	const auto s2e = (Char *)((char *)p2.Get() + size);
	const auto len = (std::size_t)Min(s1e - s1b, s2e - s2b);
	for(std::size_t i = 0; i < len; ++i){
		s1b[i] = s2b[i] = (Char)(i | 1);
	}
	s1b[len - 2] = (Char)0xAABB;
	s1b[len - 1] = (Char)0x2211;
	s2b[len - 2] = (Char)0xBBAA;
	s2b[len - 1] = (Char)0x1122;

	const auto test = [&](WideStringView name){
		const auto fname = "memcpy"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<Char * (*)(Char *, const Char *, std::size_t)>(fname);
			std::ptrdiff_t r;
			const auto t1 = GetHiResMonoClock();
			for(std::uint64_t i = 0; i < 100; ++i){
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
/*
	static constexpr std::size_t buff_len = 2000;
	static wchar_t dstb[buff_len], srcb[buff_len];
	for(std::size_t i = 0; i < buff_len / 2 - 10; ++i){
		std::memset(dstb, 'z', sizeof(dstb));
		std::memset(srcb, 'a', sizeof(srcb));
		wchar_t *const dst = dstb + i + 1;
		wchar_t *const src = srcb + i + 2;
		src[i] = 0;
		constexpr std::size_t max_len = buff_len * 2 / 5;
		const auto epd = ::_MCFCRT_wcppcpy(dst, dst + max_len, src);
		std::printf("i = %zu\n", i);
//		std::printf("  src = %s$\n", (char *)src);
//		std::printf("  dst = %s$\n", (char *)dst);
		const int cmp = std::wcscmp(src, dst);
		const std::size_t len = (std::size_t)(epd - dst);
		const int end = dst[i + 1];
		std::printf("  cmp = %d, len = %zd, end = %x\n", cmp, len, end);
		if((len != std::min(i, max_len - 1)) || (end != dstb[0])){
			std::abort();
		}
	}
*/
	return 0;
}
