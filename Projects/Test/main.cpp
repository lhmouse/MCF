#include <MCFCRT/MCFCRT.h>
#include <MCF/Hash/SHA256.hpp>
#include <MCF/Core/File.hpp>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

unsigned int MCFMain(){
	MCF::File f;
	f.Open(L"F:\\Downloads\\6.0.6001.18000.367-KRMSDK_EN.iso", true, false, false);
	ASSERT(f);
	const long long fsize = f.GetSize();
	std::unique_ptr<unsigned char[]> buffer(new unsigned char[fsize]);
	f.Read(buffer.get(), 0, fsize);

	LARGE_INTEGER cnt1, cnt2;

	unsigned char result[32];
	MCF::SHA256 sha;
	::QueryPerformanceCounter(&cnt1);
	sha.Update(buffer.get(), fsize);
	sha.Finalize(result);
	::QueryPerformanceCounter(&cnt2);

	// 18263802f73bf6f2f4afad5434e0b4066d40320a25b7eed3103a59f358e1ea85
	std::printf("SHA256 = ");
	for(const auto &x : result){
		std::printf("%02X", (unsigned int)x);
	}
	std::printf("\ntime = %lld\n", cnt2.QuadPart - cnt1.QuadPart);

	return 0;
}
