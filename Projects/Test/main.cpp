#include <MCFCRT/MCFCRT.h>
#include <MCF/Encryption/RC4Ex.hpp>
#include <MCF/Hash/CRC32.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Core/Utilities.hpp>
#include <memory>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

unsigned int MCFMain(){
	std::srand(MCF::GenRandSeed());

	MCF::CRC32 crc;
	MCF::File f(L"E:\\Download\\WinXP_SP3.iso", true, false, false);
	std::vector<unsigned char> decoded;
	const auto fsize = f.GetSize();
	std::printf("File size: %lld\n", (long long)fsize);
	decoded.resize(fsize);
	f.Read(decoded.data(), 0, decoded.size());
	crc.Update(decoded.data(), decoded.size());
	std::printf("CRC: %08X\n", crc.Finalize());

	LARGE_INTEGER t1, t2;

	std::vector<unsigned char> encoded;
	encoded.reserve(decoded.size());
	MCF::RC4ExEncoder enc([&encoded](std::size_t cb){
		const auto old_size = encoded.size();
		encoded.resize(old_size + cb);
		return encoded.data() + old_size;
	}, "meow", 4, 0);
	::QueryPerformanceCounter(&t1);
	std::size_t pos = 0;
	while(pos != decoded.size()){
		std::size_t next = pos + (std::size_t)std::rand() % 65536u + 1;
		if(next > decoded.size()){
			next = decoded.size();
		}
		enc.Update(decoded.data() + pos, next - pos);
		pos = next;
	}
	::QueryPerformanceCounter(&t2);
	std::printf("encoded in %lld ticks\n", t2.QuadPart - t1.QuadPart);

	decoded.clear();
	MCF::RC4ExDecoder dec([&decoded](std::size_t cb){
		const auto old_size = decoded.size();
		decoded.resize(old_size + cb);
		return decoded.data() + old_size;
	}, "meow", 4, 0);
	::QueryPerformanceCounter(&t1);
	dec.Update(encoded.data(), encoded.size());
	::QueryPerformanceCounter(&t2);
	std::printf("decoded in %lld ticks\n", t2.QuadPart - t1.QuadPart);
	crc.Update(decoded.data(), decoded.size());
	std::printf("CRC: %08X\n", crc.Finalize());

	return 0;
}
