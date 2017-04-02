#include <MCF/Streams/StandardInputStream.hpp>
#include <MCF/Streams/StandardOutputStream.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	const auto is = MCF::MakeUnique<MCF::StandardInputStream>();
	const auto os = MCF::MakeUnique<MCF::StandardOutputStream>();
	char s[100];
	std::size_t n;
	while((n = is->Get(s, sizeof(s))) > 0){
		os->Put(s, n);
	}
	return 0;
}
