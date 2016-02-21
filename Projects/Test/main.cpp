#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Crc32OutputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	constexpr unsigned char str[] = "hello world! hello world! hello world! hello world! hello world!";

	auto os = Crc32OutputStream();
	os.Put(str, sizeof(str) - 1);
	auto val = (unsigned long long)os.Finalize();
	std::printf("val = %08llX\n", val); // 3F4291DC

	for(unsigned i = 0; i < sizeof(str) - 1; ++i){
		os.Put(str[i]);
	}
	val = os.Finalize();
	std::printf("val = %08llX\n", val); // 3F4291DC

	return 0;
}
