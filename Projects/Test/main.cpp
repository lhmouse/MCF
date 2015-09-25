#include <MCF/Core/String.hpp>
#include <MCF/Core/Time.hpp>
#include <MCF/Core/Random.hpp>
#include <MCFCRT/env/mcfwin.h>

using namespace MCF;

extern "C" unsigned MCFMain(){
	Utf8String str, to_find;
	str.Resize(102400);
	to_find.Resize(100);

	unsigned faster = 0, slower = 0, correct = 0, incorrect = 0;
	for(unsigned i = 0; i < 100; ++i){
		for(auto ptr = str.GetBegin(); ptr != str.GetEnd(); ++ptr){
			*ptr = static_cast<char>((GetRandomUint32() & 0x3F) | 1);
		}
		for(auto ptr = to_find.GetBegin(); ptr != to_find.GetEnd(); ++ptr){
			*ptr = static_cast<char>((GetRandomUint32() & 0x3F) | 1);
		}

		std::memcpy(str.GetBegin() + GetRandomUint64() % (str.GetSize() - to_find.GetSize()), to_find.GetBegin(), to_find.GetSize());

		double t11, t12, t21, t22;
		char *pos1, *pos2;

		t11 = GetHiResMonoClock();
		pos1 = std::strstr(str.GetStr(), to_find.GetStr());
		t12 = GetHiResMonoClock();
//		std::printf("msvcrt strstr()  time = %f, pos = %p\n", t12 - t11, pos1);

		t21 = GetHiResMonoClock();
		auto off = str.Find(to_find);
		pos2 = (off == Utf8String::kNpos) ? nullptr : (str.GetBegin() + off);
		t22 = GetHiResMonoClock();
//		std::printf("String::Find()   time = %f, pos = %p\n", t22 - t21, pos2);

		if(t22 - t21 < t12 - t11){
			++faster;
		} else {
			++slower;
		}

		if(pos2 == pos1){
			++correct;
		} else {
			++incorrect;
		}
	}
	std::printf("faster = %u, slower = %u, correct = %u, incorrect = %u\n", faster, slower, correct, incorrect);

	return 0;
}
