#include <MCF/StdMCF.hpp>
#include <MCF/Streams/BufferStreams.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	BufferStream s;
	char str[64];
	unsigned len;

	s.Put("hello ", 6);
	len = s.Get(str, sizeof(str));
	std::fwrite(str, len, 1, stdout);

	s.Put("world!", 6);
	len = s.Get(str, sizeof(str));
	std::fwrite(str, len, 1, stdout);

	return 0;
}
