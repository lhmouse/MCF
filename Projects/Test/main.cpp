#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/FileStreams.hpp>
#include <MCF/Streams/StreamIterators.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto path = WideString(NarrowStringView(__FILE__));
	auto file = File(path, File::kToRead | File::kDontCreate | File::kSharedRead);
	auto strm = FileInputStream(std::move(file));

	std::printf("got      : %c\n", strm.Get());
	std::printf("got      : %c\n", strm.Get());
	std::printf("discarded: %d\n", (int)strm.Discard(1000));
	std::printf("got      : %c\n", strm.Get());

	return 0;
}
