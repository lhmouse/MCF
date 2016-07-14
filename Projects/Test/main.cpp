#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::StandardInputStream  in;
	MCF::StandardOutputStream out;
	MCF::StandardErrorStream  err;

	out.PutString(L"标准输出 1", 6, true);
	err.PutString(L"标准错误 1", 6, true);
	wchar_t str[10];
	auto len = in.GetString(str, 10, true);
	out.PutString(str, len, true);

	return 0;
}
