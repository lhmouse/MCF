#include <MCF/StdMCF.hpp>
#include <MCFCRT/startup/exe_decl.h>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	auto f = [](intptr_t n){ std::printf("thread exit %d\n", int(n)); };
	::_MCFCRT_AtThreadExit(f, 1);
	::_MCFCRT_AtThreadExit(f, 2);
	::_MCFCRT_AtThreadExit(f, 3);

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
