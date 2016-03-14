#include <MCF/StdMCF.hpp>
#include <MCF/Core/Variant.hpp>

template class MCF::Variant<int, double>;

extern "C" unsigned MCFCRT_Main(){
	auto print_type = [](auto v){ return std::printf("type = %s\n", typeid(v).name()); };
	MCF::Variant<int, double> v;
	v = 1;
	v = 2;
	v.Apply(std::move(print_type));
	v = 3.4;
	v = 5.6;
	v.Apply(std::move(print_type));
	return 0;
}
