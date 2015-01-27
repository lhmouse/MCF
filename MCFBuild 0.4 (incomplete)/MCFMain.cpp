#include <StdMCF.hpp>

extern "C" unsigned MCFMain() noexcept
try {
	std::puts("hello world!");
	return EXIT_SUCCESS;
} catch(...){
	return EXIT_FAILURE;
}
