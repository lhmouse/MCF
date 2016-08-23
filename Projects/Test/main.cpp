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
/*
#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Clocks.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Utf8String ss;
	for(unsigned i = 0; i < 1000; ++i){
		ss.Append("abcdefg αβγδεζη 一二三四五六七");
	}
	MCF::Utf16String sd;
	const auto t1 = MCF::GetHiResMonoClock();
	for(unsigned i = 0; i < 50000; ++i){
		sd.Clear();
		sd = ss;
	}
	const auto t2 = MCF::GetHiResMonoClock();
	std::printf("t2 - t1 = %f\n", t2 - t1);
	return 0;
}
*/
