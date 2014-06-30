#include <MCF/StdMCF.hpp>
#include <MCF/Language/Notation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
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
		AAA {
			z = z
			y = y
			a = a
			b = b
		}
		BBB {
			z = z
			y = y
			a = a
			b = b
		}
	)"_wso);
	auto n2 = n;
	std::printf("err = %d\n%ls\n", res.first, n2.Export().GetStr());
	return 0;
}
