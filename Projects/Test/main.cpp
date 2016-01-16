#include <MCF/StdMCF.hpp>
#include <MCF/Core/Argv.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

extern "C" unsigned MCFCRT_Main(){
	auto &&argv = MCF::Argv();
	for(unsigned i = 0; i < argv.GetSize(); ++i){
		auto s = MCF::AnsiString(argv[i]);
		std::printf("arg %u = %s\n", i, s.GetStr());
	}
	return 0;
}
