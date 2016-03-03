#include <MCF/StdMCF.hpp>
#include <MCF/Containers/StaticVector.hpp>

using namespace MCF;

template class StaticVector<std::string, 6>;

extern "C" unsigned MCFCRT_Main(){
	StaticVector<std::string, 6> v;
	for(unsigned i = 0; i < 6; ++i){
		v.Push("hello");
	}

	return 0;
}
