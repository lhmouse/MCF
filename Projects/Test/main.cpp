#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/CRC32.hpp>
#include <MCF/Core/File.hpp>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

unsigned int MCFMain(){
	MCF::File f;
	f.Open(L"F:\\Downloads\\VMware-workstation-full-v10.0.0-1295980.zip", true, false, false);
	ASSERT(f);

	long long offset = 0;

	MCF::CRC32 crc;
	char buffer1[0x1000];
	char buffer2[0x1000];
	std::size_t cbRead = 0;
	for(;;){
		cbRead = f.Read(buffer1, offset, sizeof(buffer1), [&crc, cbRead, &buffer2]{
			crc.Update(buffer2, cbRead, false);
		});
		if(cbRead == 0){
			break;
		}
		offset += cbRead;

		cbRead = f.Read(buffer2, offset, sizeof(buffer2), [&crc, cbRead, &buffer1]{
			crc.Update(buffer1, cbRead, false);
		});
		if(cbRead == 0){
			break;
		}
		offset += cbRead;
	}
	std::printf("crc = %08lX\n", (unsigned long)crc.GetResult());

	return 0;
}
