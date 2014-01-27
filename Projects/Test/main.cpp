#include <MCFCRT/MCFCRT.h>
#include <MCF/Hash/SHA1.hpp>
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
	const std::size_t cbRead = f.Read(buffer.get(), 0, fsize);
	ASSERT(cbRead == fsize);

	LARGE_INTEGER cnt1, cnt2;

	unsigned char result[20];
	MCF::SHA1 SHA1;
	::QueryPerformanceCounter(&cnt1);
	SHA1.Update(buffer.get(), fsize);
	SHA1.Finalize(result);
	::QueryPerformanceCounter(&cnt2);

	std::printf("SHA1 = ");
	for(const auto &x : result){
		std::printf("%02X", (unsigned int)x);
	}
	std::printf("\ntime = %lld\n", cnt2.QuadPart - cnt1.QuadPart);

	return 0;
}
