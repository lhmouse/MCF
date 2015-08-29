#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Utilities/Bail.hpp>

extern "C" unsigned MCFMain(){
	MCF::AnsiString s1("0123456789");
	s1.Replace(3, 5, '_', 4);
	std::printf("s1 = %s\n", s1.GetStr());

	MCF::AnsiString s2("0123456789");
	s2.Replace(3, 7, '_', 2);
	std::printf("s2 = %s\n", s2.GetStr());

	return 0;
}
