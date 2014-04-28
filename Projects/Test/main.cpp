#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/heap_dbg.h>
#include <MCF/Core/File.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Networking/HttpClient.hpp>
using namespace MCF;

#ifndef NDEBUG
#	define HEAP_DBG
#endif

#ifdef HEAP_DBG
static void heap_callback(int op, const void *p, std::size_t cb, const void *ret, std::intptr_t) noexcept {
	std::printf("%s - p = %p, cb = %zu, ret = %p\n", op ? "deallocated" : "allocated  ", p, cb, ret);
}
#endif

unsigned int MCFMain(){
#ifdef HEAP_DBG
	::__MCF_CRT_HeapSetCallback(nullptr, nullptr, &heap_callback, 0);
#endif

	try {
		WideStringObserver path(L"nib.seikooc");
		auto f = File::Open(path.GetReverse(), true, true, true);
		HttpClient http(false, nullptr, 0);

		Vector<Vector<unsigned char>> cookies;
		std::uint64_t off = 0;
		std::uint32_t cb;
		while(f->Read(&cb, sizeof(cb), off) == sizeof(cb)){
			auto &top = cookies.Push();
			top.Resize(cb);
			off += sizeof(cb);
			f->Read(top.GetData(), cb, off);
			off += cb;
		}
		http.ImportCookies(cookies);

		http.Connect(L"POST", L"http://localhost/", -1, "post_test1=meow1&post_test2=meow2", 33);
		http.Connect(L"GET", L"https://localhost/?get_test1=meow1&get_test2=meow2");
/*
		f->Clear();
		off = 0;
		cookies = http.ExportCookies(true);
		for(auto &v : cookies){
			cb = v.GetSize();
			f->Write(off, &cb, sizeof(cb));
			off += sizeof(cb);
			f->Write(off, v.GetData(), cb);
			off += cb;
		}
*/
	} catch(Exception &e){
		Bail(
			L"MCF::Exception:\nFunc = %s\nLine = %lu\nCode = %lu\nDesc = %ls\nMsg  = %ls",
			e.pszFunction,
			e.ulLine,
			e.ulErrorCode,
			GetWin32ErrorDesc(e.ulErrorCode).GetCStr(),
			e.pwszMessage
		);
	}

#ifdef HEAP_DBG
	::__MCF_CRT_HeapSetCallback(nullptr, nullptr, nullptr, 0);
#endif
	return 0;
}
