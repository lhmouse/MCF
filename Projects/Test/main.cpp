#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Core/Clocks.hpp>
#include <MCF/Core/DynamicLinkLibrary.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/CopyMoveFill.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	using namespace MCF;

	using Char = char;

	struct PageDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *p) const noexcept {
			::VirtualFree(p, 0, MEM_RELEASE);
		}
	};
	constexpr std::size_t kSize = 0x200F000;
	const UniquePtr<void, PageDeleter> p1(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const UniquePtr<void, PageDeleter> p2(::VirtualAlloc(nullptr, kSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	const auto s1  = (Char *)((char *)p1.Get() + 2);
	const auto s1e = (Char *)((char *)p1.Get() + kSize);
	const auto s2  = (Char *)((char *)p2.Get() + 18);
	const auto s2e = (Char *)((char *)p2.Get() + kSize);

	Fill(s1, s1e, 'a')[-1] = 0;
	Fill(s2, s2e, 'a')[-1] = 0;

	const auto test = [&](WideStringView name){
		const auto fname = "strlen"_nsv;
		try {
			const DynamicLinkLibrary dll(name);
			const auto pf = dll.RequireProcAddress<std::size_t (*)(const Char *)>(fname);
			std::intptr_t r;
			const auto t1 = GetHiResMonoClock();
			for(unsigned i = 0; i < 100; ++i){
				r = (std::intptr_t)(*pf)(s2);
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
