#include <MCF/Core/String.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main() noexcept {
	const auto dump = [&](auto &&s){
		for(auto c : s){
			std::printf("%0*lx ", (int)sizeof(c) * 2, (unsigned long)(std::make_unsigned_t<std::decay_t<decltype(c)>>)c);
		}
		std::putchar('\n');
	};
	auto u8s  = MCF::Utf8String  ("喵喵");
	dump(u8s);
	auto ws   = MCF::WideString  (u8s);
	dump(ws);
	auto u32s = MCF::Utf32String (ws);
	dump(u32s);
	auto as   = MCF::AnsiString  (u32s);
	dump(as);
	auto u16s = MCF::Utf16String (as);
	dump(u16s);
	return 0;
}
