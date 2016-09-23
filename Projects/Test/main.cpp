#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Crc32OutputStream.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Crc32OutputStream s;
	s.Put("hello world!", 12);
	std::printf("crc32 = %08lX\n", (unsigned long)s.Finalize());
	return 0;
}
