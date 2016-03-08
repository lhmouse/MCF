#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferInputStream.hpp>
#include <MCF/Streams/BufferOutputStream.hpp>
#include <MCF/StreamFilters/Base64InputStreamFilter.hpp>
#include <MCF/StreamFilters/Base64OutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	constexpr unsigned char str[] = "hello world!";
	for(unsigned len = 0; len < sizeof(str); ++len){
		auto os = MakeIntrusive<BufferOutputStream>();
		auto bs = MakeIntrusive<Base64OutputStreamFilter>(os);
		bs->Put(str, len);
		bs->Flush(false);

		int c;
		while((c = os->GetBuffer().Get()) >= 0){
			std::putchar(c);
		}
		std::putchar('\n');
	}
	return 0;
}
