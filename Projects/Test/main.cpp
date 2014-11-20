#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Utf8String s;
	s.Assign("aaabbbcccddddeeeefff"_u8so);
	s.Assign("0123456789"_u8so);
// auto obs = s.Slice(2, 10);
 __debugbreak();
	s.Pop();
 __debugbreak();
	std::puts(s.GetCStr());
	return 0;
}
