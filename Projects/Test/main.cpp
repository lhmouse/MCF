#include <MCFCRT/MCFCRT.h>
#include <MCF/Hash/SHA256.hpp>
#include <MCF/Core/File.hpp>
#include <MCF/Core/Utilities.hpp>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

unsigned int MCFMain(){
	MCF::File f;
	f.Open(L"F:\\Downloads\\6.0.6001.18000.367-KRMSDK_EN.iso", true, false, false);
	ASSERT(f);
	const long long fsize = f.GetSize();
	long long offset = 0;

	unsigned char result[32];
	MCF::SHA256 sha;

	unsigned char buffer[4096];
	std::srand(MCF::GenRandSeed());
	while(offset < fsize){
		const std::size_t cbToRead = std::rand() % (sizeof(buffer) / 2) + sizeof(buffer) / 2;
		const std::size_t cbRead = f.Read(buffer, offset, cbToRead);
		offset += cbRead;
		sha.Update(buffer, cbRead);
	}

	// 18263802f73bf6f2f4afad5434e0b4066d40320a25b7eed3103a59f358e1ea85
	sha.Finalize(result);
	std::printf("SHA256 = ");
	for(const auto &x : result){
		std::printf("%02X", (unsigned int)x);
	}

	return 0;
}
