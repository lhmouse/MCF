#include <MCF/StdMCF.hpp>
#include <MCF/Core/StreamBuffer.hpp>

extern "C" unsigned MCFCRT_Main(){
	MCF::StreamBuffer buffer;
	buffer.Put('a', 10000000);
	return 0;
}
