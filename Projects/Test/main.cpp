#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Crc64OutputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	static constexpr char data[] = "hello world!";

	Crc64OutputStream strm;
	strm.Put(data, sizeof(data) - 1);
	auto val = strm.Finalize();
	std::printf("val = %016llX\n", (unsigned long long)val);

	return 0;
}
