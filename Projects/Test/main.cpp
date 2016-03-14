#include <MCF/StdMCF.hpp>
#include <MCF/Core/Variant.hpp>

template class MCF::Variant<int, double>;

extern "C" unsigned MCFCRT_Main(){
	MCF::Variant<int, double> v;
	v = 1;
	v = 2;
	v = 3.4;
	v = 5.6;
	return 0;
}
