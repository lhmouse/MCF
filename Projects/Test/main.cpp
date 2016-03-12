#include <MCF/StdMCF.hpp>
#include <cmath>

volatile auto p_sin = static_cast<double (*)(double)>(&std::sin);
volatile auto p_cos = static_cast<double (*)(double)>(&std::cos);

extern "C" unsigned MCFCRT_Main(){
	const double theta = 0x1p63;
	std::printf("sin(2 * theta) = %f, builtin = %f\n", p_sin(2 * theta), __builtin_sin(2 * theta));
	std::printf("2 * sin(theta) * cos(theta) = %f, builtin = %f\n", 2 * p_sin(theta) * p_cos(theta), 2 * __builtin_sin(theta) * __builtin_cos(theta));
	return 0;
}
