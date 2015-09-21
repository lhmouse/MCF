#include <MCF/File/Utf8TextFile.hpp>
#include <MCFCRT/env/mcfwin.h>

using namespace MCF;

extern "C" unsigned MCFMain(){
/*
	Utf8TextFileReader f;
	f.Reset(File(L"test.txt", File::kToRead));
	Utf8String line;
	while(f.ReadLine(line)){
		std::printf("line = %s\n", AnsiString(line).GetStr());
	}
*/
	Utf8TextFileWriter f;
	f.Reset(File(L"test.txt", File::kToWrite),  Utf8TextFileWriter::kHasBom | Utf8TextFileWriter::kEndlCrLf);
	for(int i = 0; i < 10; ++i){
		f.WriteLine("hello! 喵"_u8so);
		f.WriteLine("world! 咪"_u8so);
	}

	return 0;
}
