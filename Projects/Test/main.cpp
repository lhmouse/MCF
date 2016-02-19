#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/FileStreams.hpp>
#include <MCF/Streams/StreamIterators.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto path = WideString(NarrowStringView(__FILE__));
	auto file = File(path, File::kToRead | File::kDontCreate | File::kSharedRead);

	FileInputStream strm(std::move(file));
	int c;
	for(auto it = StreamReadIterator(strm); (c = *it) >= 0; ++it){
		std::putchar(c);
	}

	return 0;
}
