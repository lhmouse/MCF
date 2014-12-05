#include <MCF/StdMCF.hpp>
#include <MCF/Core/Argv.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto rt = GetArgv();
	for(std::size_t i = 0; i < rt.uArgc; ++i){
		std::printf("arg %zu = %ls$\n", i, rt.pArgv[i].pwszStr);
	}
	return 0;
}
