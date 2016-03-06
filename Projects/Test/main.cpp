#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/FileInputStream.hpp>
#include <MCF/StreamFilters/BufferingInputStreamFilter.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto stdos = StandardOutputStream();

	auto wpath = WideString(NarrowStringView(__FILE__));
	auto file = File(wpath, File::kToRead);

	auto ifs = MakeIntrusive<FileInputStream>(std::move(file));
	auto buffered_ifs = MakeIntrusive<BufferingInputStreamFilter>(std::move(ifs));

	int c;
	NarrowString ln;
	while((c = buffered_ifs->Get()) >= 0){
		if(c != '\n'){
			ln.Push(c);
		} else {
			stdos.Put("Line:\t", 6);
			stdos.Put(ln.GetData(), ln.GetSize());
			stdos.Put("# EOL\n", 6);
			ln.Clear();
		}
	}

	return 0;
}
