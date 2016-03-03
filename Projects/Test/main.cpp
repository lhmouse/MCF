#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferInputStream.hpp>
#include <MCF/StreamFilters/TextInputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto is = MakeIntrusive<BufferInputStream>();
	constexpr char str[] = "01\r23\n45\r\n67\n89\r";
	is->GetBuffer().Put(str, sizeof(str) - 1);
	auto text_is = MakeIntrusive<TextInputStreamFilter>(is);

	int c;
	while((c = text_is->Get()) >= 0){
		std::printf("%02hhX ", c);
	}

	return 0;
}
