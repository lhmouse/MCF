#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	AnsiString s;
	auto src = "hello world!"_nso;
	Copy(std::front_inserter(s), src.GetBegin(), src.GetEnd());
	std::puts(s.GetCStr());
	return 0;
}
