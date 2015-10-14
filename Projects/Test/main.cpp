#include <MCF/Core/String.hpp>
#include <MCF/Core/Time.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	using namespace MCF;

	static constexpr unsigned loops = 10000000;

	auto &ku8s  = u8"a𤭢𤭢喵𤭢𤭢𤭢Г𤭢𤭢𤭢𤭢"_u8s;
	auto &ku16s =  u"a𤭢𤭢喵𤭢𤭢𤭢Г𤭢𤭢𤭢𤭢"_u16s;
	auto &ku32s =  U"a𤭢𤭢喵𤭢𤭢𤭢Г𤭢𤭢𤭢𤭢"_u32s;
	auto &kws   =  L"a𤭢𤭢喵𤭢𤭢𤭢Г𤭢𤭢𤭢𤭢"_ws;

	Utf32String u32s;
	auto t1 = GetHiResMonoClock();
	for(unsigned i = 0; i < loops; ++i){
		u32s.Clear();
		Utf8String::UnifyAppend(u32s, ku8s);
	}
	auto t2 = GetHiResMonoClock();
	std::printf("MCF  : time elasped = %f, result = %s\n", t2 - t1, AnsiString(u32s).GetStr());
/*
	Utf16String s1(ku8s); ASSERT(s1 == ku16s);
	Utf32String s2(s1);   ASSERT(s2 == ku32s);
	Utf8String  s3(s2);   ASSERT(s3 == ku8s);

	Utf32String s4(s3);   ASSERT(s4 == ku32s);
	Utf16String s5(s4);   ASSERT(s5 == ku16s);
	Utf8String  s6(s5);   ASSERT(s6 == ku8s);
	std::puts("passed!");
*/
	return 0;
}

/*
#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Core/Time.hpp>

using namespace MCF;

Mutex m;
volatile int c = 0;

extern "C" unsigned MCFMain(){
	std::array<IntrusivePtr<Thread>, 4> threads;
	for(auto &p : threads){
		p = Thread::Create([]{
			for(int i = 0; i < 1000000; ++i){
				const auto l = m.GetLock();
				++c;
			}
		}, true);
	}

	const auto t1 = GetHiResMonoClock();
	for(auto &p : threads){
		p->Resume();
	}
	for(auto &p : threads){
		p->Join();
	}
	const auto t2 = GetHiResMonoClock();

	std::printf("c = %d, time = %f\n", c, t2 - t1);
	return 0;
}
*/
