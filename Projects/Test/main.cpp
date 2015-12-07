#include <MCF/StdMCF.hpp>
#include <MCF/Core/Array.hpp>

using namespace MCF;

template class Array<int, 3, 2>;

extern "C" unsigned MCFMain(){
	Array<int, 3, 2> a = { 1,2,3,4,5,6, };
	std::printf("size = %zu\n", a.GetSize());
	return 0;
}
