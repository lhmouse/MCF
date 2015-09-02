#include <MCF/StdMCF.hpp>

long double x = 1.0, y = 2.0;

extern "C" unsigned MCFMain(){
	std::printf("fdim(x, y) = %Lf\n", std::fdim(x, y));
	std::printf("fdim(y, x) = %Lf\n", std::fdim(y, x));
	return 0;
}
