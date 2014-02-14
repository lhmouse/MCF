#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/File.hpp>
#include <MCF/Core/Utilities.hpp>
#include <MCF/Hash/CRC32.hpp>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <MCF/Compression/Lzma.hpp>
#include <MCF/Compression/Z.hpp>

unsigned int MCFMain(){
	MCF::File f(L"E:\\enwik8", true, false, false);
	ASSERT(f);

	std::string origin;
	origin.resize(f.GetSize());
	f.Read(&origin[0], 0, origin.size());

	std::string compressed;
	compressed.reserve(100 * 0x400 * 0x400);
	std::string plain;
	plain.reserve(100 * 0x400 * 0x400);

	std::uint64_t t1, t2;

	MCF::LzmaEncoder enc([&compressed](std::size_t requested){
		const auto old = compressed.size();
		compressed.resize(old + requested);
		return std::make_pair(&compressed[old], requested);
	});
	t1 = MCF::GetHiResCounter();
	enc.Update(origin.data(), origin.size());
	enc.Finalize();
	t2 = MCF::GetHiResCounter();
	t2 -= t1;

	std::printf("compressed size = %zu bytes\n", compressed.size());
	std::printf("  time elasped = %llX.%06llX seconds\n", t2 >> 24, t2 & 0xFFFFFF);

	MCF::LzmaDecoder dec([&plain](std::size_t requested){
		const auto old = plain.size();
		plain.resize(old + requested);
		return std::make_pair(&plain[old], requested);
	});
	t1 = MCF::GetHiResCounter();
	dec.Update(compressed.data(), compressed.size());
	dec.Finalize();
	t2 = MCF::GetHiResCounter();
	t2 -= t1;

	std::printf("decompressed size = %zu bytes\n", plain.size());
	std::printf("  time elasped = %llX.%06llX seconds\n", t2 >> 24, t2 & 0xFFFFFF);

	MCF::CRC32 crc;
	crc.Update(plain.data(), plain.size());
	crc.Finalize();
	std::printf("  crc32 = %08lX\n", (unsigned long)crc.Finalize());

	return 0;
}
