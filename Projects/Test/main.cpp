#include <MCF/StdMCF.hpp>
#include <MCF/Core/VarChar.hpp>

using namespace MCF;

template class VarChar<255>;

extern "C" unsigned MCFMain(){
	VarChar<255> s;
	std::printf("s = %s$\n", s.GetStr());

	s.Append("hello "_nsv);
	std::printf("s = %s$\n", s.GetStr());

	s.Append("world!"_nsv);
	std::printf("s = %s$\n", s.GetStr());

	s.Replace(5, 6, " QuQ "_nsv);
	std::printf("s = %s$\n", s.GetStr());

	return 0;
}
