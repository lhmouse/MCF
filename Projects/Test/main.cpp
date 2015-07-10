#include <MCF/StdMCF.hpp>
#include <MCF/Function/Bind.hpp>
#include <functional>

using namespace MCF;

extern "C" unsigned MCFMain(){
	auto fn = [](auto i){
		std::printf("outer, i = %d\n", i);
		return [](auto i){ std::printf("inner, i = %d\n", i); };
	};
	auto invoke = [](auto f, auto p){
		return f(p);
	};

	std::puts("-- Bind() inside Bind() --");
	auto f1 = Bind(invoke, Bind(fn, _1), _1);
	f1(123);

	std::puts("-- Curry() inside Curry() --");
	auto f2 = Curry(invoke, Curry(fn, _1), _1);
	f2(123);

	return 0;
}
