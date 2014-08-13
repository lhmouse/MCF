#include <MCF/StdMCF.hpp>
#include <MCF/LiteralConverter/LiteralConverter.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
/*	int i = 12345;
	auto s = LiteralConverter<int>(i) + AnsiString(" meow"_as);
	std::puts(s.GetCStr());*/
	char buf1[20] = "aaaaaaaaaaaaaaaaaaa", buf2[] = "hello world!";
	MoveBackward(buf1 + 20, buf2, buf2 + sizeof(buf2));
	std::puts(buf1);
	return 0;
}
