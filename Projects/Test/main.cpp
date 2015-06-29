#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Random.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	for(int i = 0; i < 10; ++i){
		std::printf("rand = %f\n", GetRandomDouble());
	}
	return 0;
}
