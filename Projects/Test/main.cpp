#include <cstdio>
#include <cmath>

double t = 12.345;

extern "C" unsigned MCFMain(){
	int exp;
	auto d = std::frexp(t, &exp);
	std::printf("%d, %f\n", exp, d);
	return 0;
}
