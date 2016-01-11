#include <MCF/StdMCF.hpp>
#include <MCF/Core/VarChar.hpp>

using namespace MCF;

template class VarChar<255>;

extern "C" unsigned MCFMain(){
	VarChar<255> s;
	std::printf("capacity = %zu, sizeof(s) = %zu\n", s.GetCapacity(), sizeof(s));

	return 0;
}
