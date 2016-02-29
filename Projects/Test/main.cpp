#include <MCF/StdMCF.hpp>
#include <MCF/Core/Optional.hpp>

using namespace MCF;

template class Optional<int>;
template class Optional<std::string>;

extern "C" unsigned MCFCRT_Main(){
	return 0;
}
