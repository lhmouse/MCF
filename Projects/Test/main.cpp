#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Invoke.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <iostream>

struct foo {
	int bark(char c, int i) const volatile && {
		std::cout <<"bark: c = " <<c <<", i = " <<i <<std::endl;
		return 123;
	}
};

struct bar : foo {
};

extern "C" unsigned MCFCRT_Main(){
	bar f;
	//auto f = MCF::MakeUnique<bar>();
	auto p = &foo::bark;
	auto ret = MCF::Invoke(p, std::move(f), 'w', 456);
	std::cout <<"ret = " <<ret <<std::endl;
	return 0;
}
