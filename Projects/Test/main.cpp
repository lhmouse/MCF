#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Crc32OutputStream.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Crc32OutputStream s;
	s.AddRef();
	return 0;
}
