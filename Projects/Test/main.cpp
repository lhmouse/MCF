#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	for(unsigned i = 0; i < 70; ++i){
		StreamBuffer b2;
		b2.Put("0123456789abcdefghijklmnopqrstuvexyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 62);
		auto b1 = b2.CutOff(i);

		char str[256];
		auto len = b1.Get(str, sizeof(str) - 1);
		str[len] = 0;
		std::printf("b1 = %s\n", str);
		len = b2.Get(str, sizeof(str) - 1);
		str[len] = 0;
		std::printf("b2 = %s\n\n", str);
	}
	return 0;
}
