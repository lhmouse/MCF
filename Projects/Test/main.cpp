#include <MCF/StdMCF.hpp>
#include <MCF/Streams/Sha256OutputStream.hpp>

using namespace MCF;

extern "C" unsigned MCFCRT_Main(){
	static constexpr char str[] = "The quick brown fox jumps over the lazy dog";

	Sha256OutputStream s;
	s.Put(str, sizeof(str) - 1);
	auto hash = s.Finalize();
	for(auto by : hash){
		std::printf("%02hhx", by);
	}
	std::puts("");

	return 0;
}
