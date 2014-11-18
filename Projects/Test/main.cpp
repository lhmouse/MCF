#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto s = "abcdefg"_ns;
	s.Replace(4, 4, "xyz"_nso);
	for(auto ch : s){
		std::putchar(ch);
	}
	return 0;
}
