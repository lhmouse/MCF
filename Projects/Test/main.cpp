#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto s("0123456789"_u8s);
	s.Append(s.Slice(2, 10));
	std::puts(s.GetCStr());
	return 0;
}
