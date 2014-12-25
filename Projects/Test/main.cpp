#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Uuid.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	char str[37];
	auto u = Uuid::Generate();
	Uuid u2("01234567-89AB-CDEF-cdef-0123456789AB");
	u.Print(str);
	std::puts(str);
	swap(u, u2);
	u.Print(str);
	std::puts(str);
	return 0;
}
