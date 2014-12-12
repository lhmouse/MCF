#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Utilities.hpp>
using namespace MCF;

struct foo {
	const int id;

	explicit foo(int x) : id(x) {
	}
};

struct func {
	void operator()(const foo &f1, const foo &f2, const foo &f3){
		std::printf("-- const foo & %d %d %d\n", f1.id, f2.id, f3.id);
	}
	void operator()(foo &f1, foo &f2, foo &f3){
		std::printf("-- foo & %d %d %d\n", f1.id, f2.id, f3.id);
	}
	void operator()(foo &&f1, foo &&f2, foo &&f3){
		std::printf("-- foo && %d %d %d\n", f1.id, f2.id, f3.id);
	}
};

extern "C" unsigned int MCFMain() noexcept {
	const auto cl = std::make_tuple(foo(1), foo(2), foo(3));
	ReverseSqueezeTuple(func(), cl); // const lvalue

	auto l = std::make_tuple(foo(4), foo(5), foo(6));
	ReverseSqueezeTuple(func(), l); // non-const lvalue

	ReverseSqueezeTuple(func(), std::forward_as_tuple(foo(7), foo(8), foo(9))); // xvalue

	ReverseSqueezeTuple(func(), std::make_tuple(foo(10), foo(11), foo(12))); // prvalue

	return 0;
}
