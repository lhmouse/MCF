#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <MCF/Function/Bind.hpp>

using namespace MCF;

struct func {
	void operator()(double) const {
		std::puts(__PRETTY_FUNCTION__);
	}
//	void operator()(char *, bool) const {
//		std::puts(__PRETTY_FUNCTION__);
//	}
};

extern "C" unsigned MCFMain(){
	Function<void(int, bool)> f;
//	f = [](int){};
	f = Bind(func(), _1);
	f(123, true);

	return 0;
}
