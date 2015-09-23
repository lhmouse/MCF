#include <MCF/File/File.hpp>
#include <MCF/File/Utf8TextFile.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Hash/Crc32.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	try {
		File f(LR"(.\\make64.cmd)"_wso, File::kToRead);
		Utf8TextFileReader r(std::move(f));

		unsigned n = 1;
		Utf8String ln;
		while(r.ReadLine(ln)){
			std::printf("line %u: %s\n", n, AnsiString(ln).GetStr());
			++n;
		}
	} catch(Exception &e){
		std::printf("exception %lu: %s\n", e.GetCode(), e.GetMsg());
	}
	return 0;
}
