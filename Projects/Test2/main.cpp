#include <MCF/StdMCF.hpp>
#include <MCF/Language/Notation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	try {
		Notation n;
		auto res = n.Parse(LR"(
			ZZZ {
				z = z
				y = y
				a = a
				b = b
			}
			YYY {
				z = z
				y = y
				a = a
				b = b
			}
			AAA = \U00000059\U00000059\x59 {
				w = w
			}
			BBB = AAA {
			}
		)"_wso);
		auto n2 = n;
		std::printf("err = %d\n%ls\n", res.first, n2.Export().GetStr());
	} catch(std::exception &e){
		std::printf("exception %s!\n", e.what());
	}
	return 0;
}
