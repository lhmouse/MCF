#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Time.hpp>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s1("hello world! 你好世界！");
	Utf16String s2(s1);
	Utf32String s3(s1);

	auto t1 = GetHiResMonoClock();
	for(unsigned i = 0; i < 0x100000; ++i){
		s2 = s1;
	}
	auto t2 = GetHiResMonoClock();
	std::printf("UTF-8 => UTF-16 milliseconds elapsed: %f\n", t2 - t1);

	t1 = GetHiResMonoClock();
	for(unsigned i = 0; i < 0x100000; ++i){
		s1 = s2;
	}
	t2 = GetHiResMonoClock();
	std::printf("UTF-16 => UTF-8 milliseconds elapsed: %f, s1 = %s$\n", t2 - t1, AnsiString(s1).GetStr());

	t1 = GetHiResMonoClock();
	for(unsigned i = 0; i < 0x100000; ++i){
		s3 = s1;
	}
	t2 = GetHiResMonoClock();
	std::printf("UTF-8 => UTF-32 milliseconds elapsed: %f\n", t2 - t1);

	t1 = GetHiResMonoClock();
	for(unsigned i = 0; i < 0x100000; ++i){
		s1 = s3;
	}
	t2 = GetHiResMonoClock();
	std::printf("UTF-32 => UTF-8 milliseconds elapsed: %f, s1 = %s$\n", t2 - t1, AnsiString(s1).GetStr());

	return 0;
}
