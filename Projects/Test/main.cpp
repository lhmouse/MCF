#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferOutputStream.hpp>
#include <MCF/Streams/TextOutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	constexpr char src[] = "1\rhello\r\nworld\n234\r";

	auto bufs = MakeIntrusive<BufferOutputStream>();

	auto s = MakeIntrusive<TextOutputStreamFilter>(bufs);
	s->Put(src, sizeof(src) - 1);

	char str[200];
	auto len = bufs->GetBuffer().Get(str, sizeof(str));
	for(std::size_t i = 0; i < len; ++i){
		std::printf("%02hhX ", str[i]);
	}
/*
	int c;
	while((c = s->Get()) >= 0){
		std::printf("%02hhX ", c);
	}
*/
	std::putchar('\n');

	return 0;
}
