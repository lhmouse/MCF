#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Fnv1a32OutputStream.hpp>

extern "C" unsigned MCFCRT_Main(){
	constexpr char str[] = "123 is a baka!";
	MCF::Fnv1a32OutputStream s;
	s.Put(str, sizeof(str) - 1);
	unsigned long val = s.Finalize();
	std::printf("str = %s, fnv1a(str) = %08lx\n", str, val);
	return 0;
}
