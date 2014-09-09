#include <MCF/StdMCF.hpp>
#include <MCF/Language/Notation.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Notation n1, n2;
	const auto str = L""
		"General {\n"
		" CompilerFlags = -Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn \\\n"
		"      -pipe -mfpmath=both -march=nocona -masm=intel\n"
		" LinkerFlags  = -Wall -Wextra -static -nostdlib \\\n"
		"      -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import \\\n"
		"      -Wl,--wrap=atexit,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free,--wrap=__cxa_throw \\\n"
		"      -lstdc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lmsvcrt -luser32 -lkernel32\n"
		"\n"
		" CFlags   = $CompilerFlags -std=c11\n"
		" CXXFlags  = $CompilerFlags -std=c++14 -Wnoexcept\n"
		" RCFlags   = -c 65001\n"
		"\n"
		" CC    = gcc\n"
		" CXX    = g++\n"
		" WINDRES   = windres\n"
		"}\n"
		"\n"
		"Debug = General {\n"
		" CompilerConfig = -fno-builtin -g -D_GLIBCXX_DEBUG_PEDANTIC -O0\n"
		" LinkerConfig = -O0\n"
		" RCConfig  =\n"
		"}\n"
		"Release = General {\n"
		" CompilerConfig = -DNDEBUG -O3 -ffunction-sections -fdata-sections -O0\n"
		" LinkerConfig = -Wl,--gc-sections,-s -O3\n"
		" RCConfig  = -DNDEBUG\n"
		"}\n"
		"\n"
		".MCFBuildVersion = 0.4\n"
		".DefaultConfig  = Debug\n"
	""_wso;
	const auto result = n1.Parse(str);
	std::printf("result code = %d\n", result.first);
	n2 = n1;
	auto rebuilt = n2.Export();
	std::puts(AnsiString(rebuilt).GetCStr());
	return 0;
}
