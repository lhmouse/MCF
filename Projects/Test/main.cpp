#include <MCF/StdMCF.hpp>
#include <MCF/Core/Variant.hpp>
#include <string>
#include <iostream>

using namespace MCF;

template class Variant<int, double, std::string>;

struct func {
	void operator()(int v){
		std::cout <<"func(int): v = " <<v <<std::endl;
	}
	void operator()(double v){
		std::cout <<"func(double): v = " <<v <<std::endl;
	}
	void operator()(const std::string &v){
		std::cout <<"func(double): v = " <<v <<std::endl;
	}
};

extern "C" unsigned MCFMain(){
	Variant<int, double, std::string> v;

	v = 123;
	v.Apply(func());

	v = 45.67;
	v.Apply(func());

	v = std::string("hello world!");
	v.Apply(func());

	return 0;
}
