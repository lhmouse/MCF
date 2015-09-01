#include <MCF/StdMCF.hpp>

extern "C" unsigned MCFMain(){
	std::printf("len = %zu\n", std::wcslen(L"hello world!"));
	return 0;
}
