#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/heap_dbg.h>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/EventDriver.hpp>
using namespace std;
using namespace MCF;

static void heap_callback(int op, const void *p, std::size_t cb, const void *ret, std::intptr_t) noexcept {
	std::printf("%s - p = %p, cb = %zu, ret = %p\n", op ? "deallocated" : "allocated  ", p, cb, ret);
}

unsigned int MCFMain()
try {
	::MCF_CRT_HeapSetCallback(nullptr, MCF_HEAP_CALLBACK{ &heap_callback, 0 });
	puts("--- 1 ---");
	{
		AnsiString s;
		puts("--- 2 ---");
		NarrowStringObserver obs(u8"喵喵喵喵喵喵喵喵喵喵"_U8SO);
		puts("--- 3 ---");
		s.Assign<StringEncoding::UTF8>(obs);
		puts("--- 4 ---");
		puts(s.GetCStr());
		puts("--- 5 ---");
	}
	puts("--- 6 ---");
	::MCF_CRT_HeapSetCallback(nullptr, MCF_HEAP_CALLBACK{ nullptr, 0 });
	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->pszFunction);
		printf("  line = %lu\n", p->ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->pwszMessage)).GetCStr());
	}
	return 0;
}
