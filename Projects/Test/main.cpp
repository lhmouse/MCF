#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Clocks.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Utf8String t;
	for(unsigned i = 0; i < 1000; ++i){
		t.Append("abcdefg αβγδεζη 一二三四五六七𤭢𤭢𤭢𤭢𤭢𤭢𤭢");
	}
	MCF::Utf8String ss = t;
	MCF::Utf32String sd;
	const auto t1 = MCF::GetHiResMonoClock();
	for(unsigned i = 0; i < 20000; ++i){
		sd.Clear();
		sd = ss;
		ss.Clear();
		ss = sd;
	}
	const auto t2 = MCF::GetHiResMonoClock();
	std::printf("t2 - t1 = %f, d = %d\n", t2 - t1, ss.Compare(t));
	return 0;
}
