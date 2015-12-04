#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Containers/Vector.hpp>

using namespace MCF;

template class Vector<Utf8String>;

extern "C" unsigned MCFMain(){
	return 0;
}
