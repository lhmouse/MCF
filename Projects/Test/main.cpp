#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>

using namespace MCF;

template class Array<int, 2, 5>;

extern "C" unsigned MCFCRT_Main(){
	Array<int, 2, 5> a {{ {0,1,2,3,4},{5,6,7,8,9} }};

	return 0;
}
