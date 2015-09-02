#include <MCF/StdMCF.hpp>
#include <MCF/Core/Argv.hpp>

extern "C" unsigned MCFMain(){
	MCF::Argv args;
	for(unsigned i = 0; ; ++i){
		const auto ptr = args.GetStr(i);
		if(!ptr){
			break;
		}
		std::printf("arg %u = %ls\n", i, ptr);
	}
	return 0;
}
