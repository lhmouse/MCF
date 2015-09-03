#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Variant.hpp>
#include <string>

template class MCF::Variant<int, double, std::string>;

extern "C" unsigned MCFMain(){
	MCF::Variant<int, double, std::string> v;

	v.Set(123);
	std::printf("type name = %s\n", v.GetTypeInfo()->name());

	v.Set(45.6);
	std::printf("type name = %s\n", v.GetTypeInfo()->name());

	v.Set(std::string("hello world!"));
	std::printf("type name = %s\n", v.GetTypeInfo()->name());

	int *iptr = v.Get<int>();
	std::printf("iptr = %p\n", (void *)iptr);

	std::string *sptr = v.Get<std::string>();
	std::printf("sptr = %p, s = %s\n", (void *)sptr, sptr->c_str());

	return 0;
}
