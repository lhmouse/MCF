#include <MCF/File/File.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Hash/Crc32.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	try {
		File f(LR"(.\\make64.cmd)"_wso, File::kToRead);

		Crc32 crc32;
		char data[512];
		std::size_t size;
		std::uint64_t offset = 0;
		while((size = f.Read(data, sizeof(data), offset)) != 0){
			crc32.Update(data, size);
			offset += size;
		}
		std::printf("crc = %08lX\n", (unsigned long)crc32.Finalize());
	} catch(Exception &e){
		std::printf("exception %lu: %s\n", e.GetCode(), e.GetMsg());
	}
	return 0;
}
