#include <MCF/StdMCF.hpp>
#include <MCF/Languages/MNotation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MNotation n;

	auto result = n.Parse(L""
		"root { \n"
		"   { \n"
		"     = uval1 \n"
		"    key1 = val1 \n"
		"     = uval2 \n"
		"    key1 = val1 \n"
		"  } \n"
		"  = { \n"
		"  } \n"
		"} \n"
		""_wso);
	std::printf("result = %d:\n%ls\n", result.first, n.Export().GetStr());

	return 0;
}
