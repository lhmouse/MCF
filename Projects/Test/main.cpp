#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	for(unsigned i = 0; i < 110; ++i){
		StreamBuffer buf1;
		for(unsigned j = 0x21; j < 0x7F; ++j){
			buf1.Put(j);
		}
		auto buf2 = buf1.CutOff(i);

		char temp[1024];
		auto len = buf1.Peek(temp, sizeof(temp));
		temp[len] = 0;
		std::printf("buf1 = (%zu) %s\n", buf1.GetSize(), temp);
		len = buf2.Peek(temp, sizeof(temp));
		temp[len] = 0;
		std::printf("buf2 = (%zu) %s\n", buf2.GetSize(), temp);
	}
	return 0;
}
