#include <MCF/StdMCF.hpp>
#include <MCF/Containers/StaticVector.hpp>
#include <MCF/Containers/Vector.hpp>
#include <string>

template class MCF::StaticVector<std::string, 5>;
template class MCF::Vector<std::string>;

extern "C" unsigned MCFCRT_Main(){

	return 0;
}
