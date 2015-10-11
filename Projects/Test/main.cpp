#include <MCF/Core/String.hpp>
#include <MCF/Core/Time.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	using namespace MCF;

	static constexpr unsigned loops = 10000000;

	auto &ku8s  = u8"你是我的小呀小苹果"_u8s;
	auto &ku16s =  u"你是我的小呀小苹果"_u16s;
	auto &ku32s =  U"你是我的小呀小苹果"_u32s;
	auto &kws   =  L"你是我的小呀小苹果"_ws;

	Utf32String u32s;
	auto t1 = GetHiResMonoClock();
	for(unsigned i = 0; i < loops; ++i){
		u32s.Clear();

//		u32s.Append(ku8s);              // Slower, strong exception safety guarantee.
		Utf8String::Unify(u32s, ku8s);  // Faster, basic exception safety guarantee.
	}
	auto t2 = GetHiResMonoClock();
	std::printf("MCF  : time elasped = %f, result = %s\n", t2 - t1, AnsiString(u32s).GetStr());

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
			for(int i = 0; i < 500000; ++i){
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
