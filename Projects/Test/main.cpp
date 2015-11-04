#include <MCF/StdMCF.hpp>
#include <MCF/Core/Matrix.hpp>
#include <MCF/Core/Optional.hpp>

using namespace MCF;

template class Optional<int>;
template class Optional<std::exception_ptr>;

extern "C" unsigned MCFMain(){
	return 0;
}
