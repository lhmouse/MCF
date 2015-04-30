#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Core/Time.hpp>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s1("hello world! 你好世界！");
	for(unsigned i = 0; i < 21; ++i){
		s1 += s1;
	}

	auto t1 = GetHiResMonoClock();
	Utf16String s2(s1);
	auto t2 = GetHiResMonoClock();
	std::printf("UTF-8 => UTF-16 milliseconds elapsed: %f\n", t2 - t1);

	t1 = GetHiResMonoClock();
	s1 = s2;
	t2 = GetHiResMonoClock();
	std::printf("UTF-16 => UTF-8 milliseconds elapsed: %f\n", t2 - t1);

	t1 = GetHiResMonoClock();
	Utf32String s3(s1);
	t2 = GetHiResMonoClock();
	std::printf("UTF-8 => UTF-32 milliseconds elapsed: %f\n", t2 - t1);

	t1 = GetHiResMonoClock();
	s1 = s3;
	t2 = GetHiResMonoClock();
	std::printf("UTF-32 => UTF-8 milliseconds elapsed: %f\n", t2 - t1);

	return 0;
}
