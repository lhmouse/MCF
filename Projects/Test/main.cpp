#include <MCF/StdMCF.hpp>
#include <MCF/Hash/Crc32.hpp>
#include <MCF/Hash/Crc64.hpp>
#include <MCF/Core/StringView.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	const auto str = "hello world!"_nsv;

	Crc32 crc32;
	crc32.Update(str.GetBegin(), str.GetSize());
	unsigned long long val = crc32.Finalize();
	// crc32 = 0x03B4C26D
	std::printf("crc32 = 0x%08llX\n", val);

	Crc64 crc;
	crc.Update(str.GetBegin(), str.GetSize());
	val = crc.Finalize();
	// crc64 = 0x8483C0FA32607D61
	std::printf("crc64 = 0x%016llX\n", val);

	return 0;
}
