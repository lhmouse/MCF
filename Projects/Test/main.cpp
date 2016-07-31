#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>
#include <MCF/Streams/StandardErrorStream.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::StandardInputStream  in;
	MCF::StandardOutputStream out;
	MCF::StandardErrorStream  err;

	out.PutText(L"标准输出 1", 6, true);
	err.PutText(L"标准错误 1", 6, true);
	wchar_t str[10];
	auto len = in.GetText(str, 10, true);
	out.PutText(str, len, true);

	return 0;
}
