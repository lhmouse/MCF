#include <MCF/StdMCF.hpp>
#include <MCF/Core/Variant.hpp>
#include <typeinfo>
#include <cstdio>

template class MCF::Variant<int, long, double>;

MCF::Variant<char, int, double> v;

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	auto f = [](auto &&x){ std::printf("type = %s\n", typeid(x).name()); };
	v.Set(1);
	v.Visit(f);
	auto v2 = v;
	v.Set(1.2);
	v.Visit(f);
	v2.Visit(f);

	auto pi = v.Get<const int>();
	std::printf("pi = %p\n", (void *)pi);
	auto pd = v.Get<const double>();
	std::printf("pd = %p\n", (void *)pd);

	return 0;
}
