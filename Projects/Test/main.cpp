#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Core/Utilities.hpp>
#include <MCF/Hash/CRC32.hpp>
#include <MCF/Compression/Lzma.hpp>
#include <MCF/Compression/Z.hpp>
#include <memory>
#include <string>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <windows.h>

unsigned int MCFMain(){
	try {
		MCF::File f(L"E:\\microarchitecture.pdf", true, false, false);
		ASSERT(f);

		std::string origin;
		origin.resize(f.GetSize());
		f.Read(&origin[0], 0, origin.size());

		std::string compressed;
		compressed.reserve(100 * 0x400 * 0x400);
		std::string plain;
		plain.reserve(100 * 0x400 * 0x400);

		MCF::LzmaEncoder enc([&compressed](std::size_t requested){
			const auto old = compressed.size();
			compressed.resize(old + requested);
			return std::make_pair(&compressed[old], requested);
		});
enc.Update(origin.data(), 10);
enc.Finalize();
compressed.clear();
		enc.Update(origin.data(), origin.size());
		enc.Finalize();

		std::printf("compressed size = %zu bytes\n", compressed.size());

		MCF::LzmaDecoder dec([&plain](std::size_t requested){
			const auto old = plain.size();
			plain.resize(old + requested);
			return std::make_pair(&plain[old], requested);
		});
dec.Update(compressed.data(), 10);
dec.Finalize();
plain.clear();
		std::size_t decompressed = 0;
		while(decompressed < compressed.size()){
			dec.Update(compressed.data() + decompressed, 1);
			++decompressed;
		}
		dec.Finalize();

		std::printf("decompressed size = %zu bytes\n", plain.size());

		MCF::CRC32 crc;
		crc.Update(plain.data(), plain.size());
		crc.Finalize();
		std::printf("  crc32 = %08lX\n", (unsigned long)crc.Finalize());
	} catch(MCF::Exception &e){
		::__MCF_CRT_BailF(L"MCF::Exception:\n  func = %s\n  err = %lu\n  desc = %ls", e.m_pszFunction, e.m_ulCode, e.m_pwszMessage);
	} catch(std::exception &e){
		::__MCF_CRT_BailF(L"std::exception:\n  desc = %s", e.what());
	} catch(...){
		::__MCF_CRT_Bail(L"Unknown exception");
	}
	return 0;
}
