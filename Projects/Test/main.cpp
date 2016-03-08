#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferInputStream.hpp>
#include <MCF/StreamFilters/TextInputStreamFilter.hpp>
#include <MCF/Streams/BufferOutputStream.hpp>
#include <MCF/StreamFilters/TextOutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	constexpr unsigned char str[] = "he\rllo\nwor\r\nld!\n\r";
	int c;

	{
		auto is = MakeIntrusive<BufferInputStream>();
		auto text_is = MakeIntrusive<TextInputStreamFilter>(is);

		is->GetBuffer().Put(str, sizeof(str) - 1);

		while((c = text_is->Get()) >= 0){
			std::printf("%02hhX ", c);
		}
		std::putchar('\n');
	}

	{
		auto os = MakeIntrusive<BufferOutputStream>();
		auto text_os = MakeIntrusive<TextOutputStreamFilter>(os);

		text_os->Put(str, sizeof(str) - 1);
		text_os->Flush(false);

		while((c = os->GetBuffer().Get()) >= 0){
			std::printf("%02hhX ", c);
		}
		std::putchar('\n');
	}

	return 0;
}
