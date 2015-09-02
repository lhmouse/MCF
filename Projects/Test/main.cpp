#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/ConstructDestruct.hpp>
#include <string>

extern "C" unsigned MCFMain(){
	char temp[sizeof(std::string)];
	const auto ptr = (std::string *)temp;
	MCF::Construct(ptr, "hello world!");
	MCF::Destruct(ptr);
	return 0;
}
