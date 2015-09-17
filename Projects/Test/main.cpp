#include <cmath>
#include <cstdio>

volatile double d = 1e100;

extern "C" unsigned MCFMain(){
	std::printf("%.20f\n", std::cos(d));
	return 0;
}
