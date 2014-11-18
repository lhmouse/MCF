#include <MCF/StdMCF.hpp>
#include <MCF/Language/MNotation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MNotation nt;
	const auto result = nt.Parse(L"root{val=2\nval=1\nnested{\nval=3}}"_wso);
	std::printf("%d\n", result.first);
	std::printf("%ls\n", nt.Export().GetCStr());
	return 0;
}
