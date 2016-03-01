#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/FileInputStream.hpp>
#include <MCF/Streams/BufferingInputStreamFilter.hpp>
#include <MCF/Streams/TextInputStreamFilter.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	auto stdos = StandardOutputStream();

	auto wpath = WideString(NarrowStringView(__FILE__));
	auto file = File(wpath, File::kToRead);

	auto ifs = MakeIntrusive<FileInputStream>(std::move(file));
	auto buffered_ifs = MakeIntrusive<BufferingInputStreamFilter>(std::move(ifs));
	auto text_ifs = MakeIntrusive<TextInputStreamFilter>(std::move(buffered_ifs));

	int c;
	NarrowString ln;
	while((c = text_ifs->Get()) >= 0){
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
