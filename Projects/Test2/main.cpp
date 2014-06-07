#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Language/Notation.hpp>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	const auto s = L""
		"Default = Debug \n"
		" \n"
		"General { \n"
		"	CompilerFlags		= -Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pedantic -pipe -mfpmath=both -march=core2 -masm=intel \n"
		" \n"
		"	CFlags				= -std=c11 \n"
		"	CPPFlags			= -std=c++1y -Wnoexcept \n"
		" \n"
		"	IgnoredFiles		= .Built* *.h *.hpp *.mcfproj \n"
		" \n"
		"	PreCompiledHeaders { \n"
		"		C { \n"
		"			SourceFile	= env\\_crtdef.h \n"
		"			CommandLine	= gcc -x c-header $CompilerFlags$ $CFlags$ %IN% -o %OUT% \n"
		"		} \n"
		"		CPP { \n"
		"			SourceFile	= env\\_crtdef.hpp \n"
		"			CommandLine	= g++ -x c++-header $CompilerFlags$ $CPPFlags$ %IN% -o %OUT% \n"
		"		} \n"
		"	} \n"
		" \n"
		"	Compilers { \n"
		"		c { \n"
		"			CommandLine	= gcc -x c -c $CompilerFlags$ $CFlags$ %GCH.C% %IN% -o %OUT% \n"
		"			Dependency	= gcc -x c -c $CompilerFlags$ $CFlags$ %IN% -MM \n"
		"		} \n"
		"		cpp cxx cc { \n"
		"			CommandLine	= g++ -x c++ -c $CompilerFlags$ $CPPFlags$ %GCH.CPP% %IN% -o %OUT% \n"
		"			Dependency	= g++ -x c++ -c $CompilerFlags$ $CPPFlags$ %IN% -MM \n"
		"		} \n"
		"	} \n"
		" \n"
		"	Linkers { \n"
		"		Partial			= ld -r %IN% -o %OUT% \n"
		"		Full			= ar rcs %OUT% %IN% \n"
		"	} \n"
		" \n"
		"	DefaultOutput		= libmcfcrt.a \n"
		"} \n"
		" \n"
		"Debug = General { \n"
		"	CompilerFlags		=> -fno-builtin -g -O0 \n"
		"} \n"
		" \n"
		"Release = General { \n"
		"	CompilerFlags		=> -DNDEBUG -O3 -ffunction-sections -fdata-sections \n"
		"} \n"
	""_wso;

	Notation n;
	const auto res = n.Parse(s);
	const auto p = n.CreatePackageFromPath(std::initializer_list<WideStringObserver>{ L"General"_wso, L"meow"_wso, L"meow!"_wso });
	printf("p = %p\n", p.first);
	printf("result = %d\n", res.first);
	printf("%ls\n", n.Export().GetStr());

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->m_wcsMessage)).GetCStr());
	}
	return 0;
}
