#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned MCFMain(){
	auto s = UR"(\帝球是个⑨/)"_u32s; // UTF-32
	std::printf("s.len = %zu\n", s.GetSize());

	auto sv = Utf32StringView(s);
	auto s2 = AnsiString(sv);
	std::printf("s2.len = %zu, s2 = %s\n", s2.GetSize(), s2.GetStr());

	return 0;
}
