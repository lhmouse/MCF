#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/File.hpp>
#include <MCF/Hash/CRC32.hpp>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <MCF/Compression/Lzma.hpp>

static unsigned char buffer[16 * 0x400 * 0x400];

unsigned int MCFMain(){
	MCF::File f(L"E:\\笑傲江湖.txt", true, false, false);
	ASSERT(f);

	std::string compressed;
	std::string plain;

	MCF::LzmaEncoder enc([&compressed](std::size_t requested){
		const auto old = compressed.size();
		compressed.resize(old + requested);
		return std::make_pair(&compressed[old], requested);
	});
	const auto fsize = f.GetSize();
	unsigned long long offset = 0;
	while(offset < fsize){
		const auto bytes = f.Read(buffer, offset, sizeof(buffer));
		ASSERT(bytes);

		enc.Update(buffer, bytes);
		offset += bytes;
	}
	enc.Finalize();

	std::printf("compressed size = %zu bytes\n", compressed.size());

	MCF::LzmaDecoder dec([&plain](std::size_t){
		plain.push_back(0);
		return std::make_pair(&plain.back(), 1);
	});
	dec.Update(compressed.data(), compressed.size());
	dec.Finalize();

	std::printf("decompressed size = %zu bytes\n", plain.size());

	MCF::CRC32 crc;
	crc.Update(plain.data(), plain.size());
	crc.Finalize();
	std::printf("  crc = %08lX\n", (unsigned long)crc.Finalize());

	return 0;
}
