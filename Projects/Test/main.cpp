#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <MCF/Function/Bind.hpp>
#include <iostream>
#include <functional>

using namespace MCF;

struct foo {
	int i = 0;

	int add(int k){
		return i += k;
	}

	bool check() const {
		return i > 5;
	}
};

extern "C" unsigned int MCFMain() noexcept {
	foo f;

	auto f1 = Function<int (foo *, int)>([](auto pf, auto k){ return pf->add(k); });
	auto fc1 = Function<bool (foo *)>(&foo::check);
	int val = f1(&f, 2);
	std::cout <<val <<' ' <<fc1(&f) <<std::endl;

	f1.Reset(&foo::add);
	val = f1(&f, 2);
	std::cout <<val <<' ' <<fc1(&f) <<std::endl;

	/////////////////////////////////////////
	auto f2 = Function<int (foo &, int)>([](auto &f, auto k){ return f.add(k); });
	auto fc2 = Function<bool (foo &)>(&foo::check);
	val = f2(f, 2);
	std::cout <<val <<' ' <<fc2(f) <<std::endl;

	f2.Reset(&foo::add);
	val = f2(f, 2);
	std::cout <<val <<' ' <<fc2(f) <<std::endl;

	/////////////////////////////////////////
	auto f3 = Bind(std::multiplies<int>(), 3, _1);
	std::cout <<f3(2) <<std::endl;
	std::cout <<f3(3) <<std::endl;

	std::string str = "hello";
	Bind([](auto p){ *p += " world!"; }, &str)();
	std::cout <<str <<std::endl;

	/////////////////////////////////////////
	auto f4 = Bind(&std::string::push_back, &str, _1);
	str.clear();
	f4('a');
	f4('b');
	f4('c');
	std::cout <<str <<std::endl;

	return 0;
}
