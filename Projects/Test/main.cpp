#include <MCF/StdMCF.hpp>
#include <MCF/Core/Variant.hpp>

using namespace MCF;

using V = Variant<int, double>;

extern "C" unsigned MCFMain(){
	V v;
	std::printf("index = %zu\n", v.GetIndex());
	v = 1;
	std::printf("index = %zu\n", v.GetIndex());
	v = 1.0;
	std::printf("index = %zu\n", v.GetIndex());

	std::printf("ptr = %p\n", (void *)v.Get<int>());

	std::printf("ptr = %p\n", (void *)v.Get<double>());

	return 0;
}
