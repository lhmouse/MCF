#include <cstdio>
#include <cmath>

double d = -9.9999;

extern "C" unsigned MCFMain(){
	std::printf("floor = %f\n", std::floor(d));
	return 0;
}
