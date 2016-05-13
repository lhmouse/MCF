#include <MCF/StdMCF.hpp>
#include <MCFCRT/env/thread.h>
#include <MCFCRT/startup/exe_decl.h>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

void f(std::intptr_t n){
	static int a = 1000;
	std::printf("thread exit %d\n", int(n));
	if(a < 1100){
		::_MCFCRT_AtThreadExit(f, ++a);
	}
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	for(int i = 0; i < 100; ++i){
		::_MCFCRT_AtThreadExit(f, i);
	}

/*
	MCF::StandardInputStream  in;
	MCF::StandardOutputStream out;
	MCF::StandardErrorStream  err;

	out.PutString(L"标准输出 1", 6, true);
	err.PutString(L"标准错误 1", 6, true);
	wchar_t str[10];
	auto len = in.GetString(str, 10, true);
	out.PutString(str, len, true);
*/
	return 0;
}
