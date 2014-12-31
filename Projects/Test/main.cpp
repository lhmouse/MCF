#include <MCF/StdMCF.hpp>
#include <MCF/Languages/MNotation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	MNotation n;

	auto result = n.Parse(
		LR"-------(
			root {
				= \ ; aaa
				{
					= val1
					= val2
				}
				{
					= val3
					= val4
				}
				{
					= val5
				}
				{
					= val6
					= val7
					= val8
				}
				{
					= val9
				}
			}
		)-------"_wso);
	std::printf("result = %d:\n---------\n%ls\n", result.first, n.Export().GetStr());

	return 0;
}
