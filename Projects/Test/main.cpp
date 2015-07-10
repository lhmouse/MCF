#include <MCF/StdMCF.hpp>
#include <MCF/Function/Bind.hpp>
#include <functional>

using namespace MCF;

extern "C" unsigned MCFMain(){

	auto fn = []{ std::puts("outer");
		return []{ std::puts("inner"); }; };
	auto invoke = [](auto f){ f(); };

	Bind(invoke, Bind(fn))();
	std::puts("---");
	Curry(invoke, Curry(fn))();

	return 0;
}
