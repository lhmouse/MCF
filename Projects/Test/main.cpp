#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/File.hpp>
#include <MCF/Compression/Z.hpp>
#include <MCF/Hash/CRC32.hpp>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstddef>

unsigned int MCFMain(){
	MCF::File f(L"E:\\笑傲江湖.txt", true, false, false);
	ASSERT(f);

	std::string compressed;
	std::string plain;

	MCF::ZEncoder zenc([&compressed](std::size_t requested){
		const auto old = compressed.size();
		compressed.resize(old + requested);
		return std::make_pair(&compressed[old], requested);
	});
	const auto fsize = f.GetSize();
	unsigned long long offset = 0;
	while(offset < fsize){
		unsigned char buffer[1024];

		const auto bytes = f.Read(buffer, offset, sizeof(buffer));
		ASSERT(bytes);

		zenc.Update(buffer, bytes);
		offset += bytes;
	}
	zenc.Finalize();

	std::printf("compressed size = %zu bytes\n", compressed.size());

	MCF::ZDecoder zdec([&plain](std::size_t requested){
		const auto old = plain.size();
		plain.resize(old + requested);
		return std::make_pair(&plain[old], requested);
	});
	zdec.Update(compressed.data(), compressed.size());
	zdec.Finalize();

	std::printf("decompressed size = %zu bytes\n", plain.size());

	MCF::CRC32 crc;
	crc.Update(plain.data(), plain.size());
	crc.Finalize();
	std::printf("  crc = %08lX\n", (unsigned long)crc.Finalize());

	return 0;
}
