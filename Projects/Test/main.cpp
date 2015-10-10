#include <MCF/Core/String.hpp>
#include <MCF/Core/Time.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	static constexpr unsigned loops = 1000000;
	static constexpr unsigned char str[] = { 0xE4, 0xBD, 0xA0, 0xE6, 0x98, 0xAF, 0xE6, 0x88, 0x91, 0xE7, 0x9A, 0x84, 0xE5, 0xB0, 0x8F, 0xE5, 0x91, 0x80, 0xE5, 0xB0, 0x8F, 0xE8, 0x8B, 0xB9, 0xE6, 0x9E, 0x9C };

	MCF::Utf8String u8s((const char *)str, sizeof(str));
	MCF::Utf16String u16s;
	auto t1 = MCF::GetHiResMonoClock();
	for(unsigned i = 0; i < loops; ++i){
//		temp.Clear(); // MCF::Utf32String().Swap(temp);
//		MCF::Utf8String::Unify(temp, u8s);
//		u16s.Clear();
//		MCF::Utf16String::Deunify(u16s, temp);

		u16s.Clear();
//		u16s.Append(u8s);						// Slower, strong exception safety guarantee.
		MCF::Utf8String::Unify(u16s, u8s);		// Faster, basic exception safety guarantee.
	}
	auto t2 = MCF::GetHiResMonoClock();
	std::printf("MCF  : time elasped = %f, result = %s\n", t2 - t1, MCF::AnsiString(u16s).GetStr());

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
