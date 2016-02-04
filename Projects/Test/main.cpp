#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>
#include <iostream>

template class MCF::Vector<int>;

extern "C" unsigned MCFCRT_Main(){
	MCF::Vector<int> v1, v2;
	v1 = {0,1,2,3,4,5,6};
	v2 = v1;
	v2 = v1;
	return 0;
}
