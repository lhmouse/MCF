#include <MCF/Core/Time.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	std::printf("qpc = %f\n", MCF::GetHiResMonoClock());
	return 0;
}
