#include <MCF/StdMCF.hpp>
#include <MCF/Hash/Crc32.hpp>
#include <MCF/Core/StringView.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	Crc32 crc;
	const auto str = "hello world!"_nsv;
	crc.Update(str.GetBegin(), str.GetSize());
	auto val = crc.Finalize();
	// crc = 0x03B4C26D
	std::printf("crc = 0x%08lX\n", (unsigned long)val);
	return 0;
}
