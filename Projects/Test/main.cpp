#include <MCF/StdMCF.hpp>
#include <MCF/Streams/StringInputStream.hpp>
#include <MCF/StreamFilters/BufferingInputStreamFilter.hpp>
#include <MCFCRT/ext/itoa.h>

using namespace MCF;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	const auto is = MakeIntrusive<StringInputStream>("0123456789abcdefghijklmnopqrstuvwxyz"_ns);
	const auto bs = MakeIntrusive<BufferingInputStreamFilter>(is);
	std::printf("ss  = %s\n", is->GetString().GetStr() + is->GetOffset());

	char str[256];
	auto len = bs->Get(str, 6);
	str[len] = 0;
	std::printf("str = %s\n", str);
	std::printf("ss  = %s\n", is->GetString().GetStr() + is->GetOffset());

	len = bs->Get(str, 6);
	str[len] = 0;
	std::printf("str = %s\n", str);
	std::printf("ss  = %s\n", is->GetString().GetStr() + is->GetOffset());

	bs->Invalidate();
	std::printf("ss  = %s\n", is->GetString().GetStr() + is->GetOffset());
	return 0;
}
