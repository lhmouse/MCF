#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Components/Notation.hpp>
using namespace MCF;

unsigned int MCFMain(){
	Notation n;
	n.Parse(
		L"Default = Debug\n"
		"\n"
		"\\u55b5General {\n"
		"  CompilerFlags    = -Wall -Wextra -Wsuggest-attribute=noreturn -pipe -mfpmath=sse,387 -msse2 -masm=intel -I../..\n"
		"\n"
		"  CFlags        = -std=c11\n"
		"  CPPFlags      = -std=c++11\n"
		"\n"
		"  IgnoredFiles    = .Built* *.h *.hpp *.mcfproj\n"
		"\n"
		"  PreCompiledHeaders {\n"
		"    CPP {\n"
		"      SourceFile  = StdMCF.hpp\n"
		"      CommandLine  = g++ -x c++-header $CompilerFlags$ $CPPFlags$ %IN% -o %OUT%\n"
		"    }\n"
		"  }\n"
		"\n"
		"  Compilers {\n"
		"    c {\n"
		"      CommandLine  = gcc -x c -c $CompilerFlags$ $CFlags$ %IN% -o %OUT%\n"
		"      Dependency  = gcc -x c -c $CompilerFlags$ $CFlags$ %IN% -MM\n"
		"    }\n"
		"    cpp cxx cc {\n"
		"      CommandLine  = g++ -x c++ -c $CompilerFlags$ $CPPFlags$ %GCH.CPP% %IN% -o %OUT%\n"
		"      Dependency  = g++ -x c++ -c $CompilerFlags$ $CPPFlags$ %IN% -MM\n"
		"    }\n"
		"  }\n"
		"\n"
		"  Linkers {\n"
		"    Partial      = ld -r %IN% -o %OUT%\n"
		"    Full      = ar rcs %OUT% %IN%\n"
		"  }\n"
		"\n"
		"  DefaultOutput    = libmcf.a\n"
		"}\n"
		"\n"
		"Debug = General {\n"
		"  CompilerFlags    => -fno-builtin -g -O0\n"
		"}\n"
		"\n"
		"Release = General {\n"
		"  CompilerFlags    => -DNDEBUG -O3 -ffunction-sections -fdata-sections\n"
		"}\n"
	);
	auto s = n.Export(L"  ");
	DWORD dummy;
	::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), s.GetCStr(), s.GetLength(), &dummy, nullptr);
	std::printf("%p\n", n.GetPackageFromPath(L"喵General", L"Compilers", L"c"));
	return 0;
}
