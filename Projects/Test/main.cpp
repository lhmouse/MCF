#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferOutputStream.hpp>
#include <MCF/StreamFilters/TextOutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto os = MakeIntrusive<BufferOutputStream>();
	constexpr char str[] = "01\r23\n45\r\n67\n89\r";
	auto text_os = MakeIntrusive<TextOutputStreamFilter>(os);
	text_os->Put(str, sizeof(str) - 1);

	int c;
	while((c = os->GetBuffer().Get()) >= 0){
		std::printf("%02hhX ", c);
	}

	return 0;
}
