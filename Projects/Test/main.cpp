#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Streams/StringInputStream.hpp>
#include <MCF/StreamFilters/BufferingInputStreamFilter.hpp>
#include <MCF/StreamFilters/BufferingOutputStreamFilter.hpp>

using namespace MCF;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	const auto ss = MakeIntrusive<StringInputStream>();
	ss->GetString().Append("0123456789abcdefghijklmnopqrstuvwxyz", 36);
	std::printf("ss = %s\n", ss->GetString().GetStr() + ss->GetOffset());
	const auto bs = MakeIntrusive<BufferingInputStreamFilter>(ss);
	for(unsigned i = 0; i < 3; ++i){
		char str[60];
		auto len = bs->Get(str, 6);
		str[len] = 0;
		std::printf("> i = %u, str = %s\n", i, str);
	}
	std::printf("ss = %s\n", ss->GetString().GetStr() + ss->GetOffset());
	bs->Invalidate();
	std::printf("ss = %s\n", ss->GetString().GetStr() + ss->GetOffset());
	return 0;
}
