#include <MCF/StdMCF.hpp>
#include <cmath>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	double d1, d2;
	std::scanf("%lf %lf", &d1, &d2);
	std::printf("%f\n", std::pow(d1, d2));
	return 0;
}
