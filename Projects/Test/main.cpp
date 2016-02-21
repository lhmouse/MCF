#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/FileOutputStream.hpp>
#include <MCF/Streams/BufferingOutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto file = File(L".test.txt"_wsv, File::kToWrite);
	auto pofs = MakeUnique<BufferingOutputStreamFilter>(MakeUnique<FileOutputStream>(std::move(file)));

	const auto data = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\n"_nsv;

	for(unsigned i = 0; i < 100; ++i){
		pofs->Put(data.GetBegin(), data.GetSize());
	}

	return 0;
}
