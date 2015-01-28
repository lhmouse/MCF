#include <MCF/StdMCF.hpp>
#include <MCF/Languages/MNotation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MCF::MNotation m;
	auto ret = m.Parse(LR"__(
		aa {
			xx = yy
			xx { zz { } }
		}
	)__"_wso);
	std::printf("result = %d\n===========\n%ls\n", ret.first, m.Export().GetStr());
	return 0;
}
