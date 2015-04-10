#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/CopyOnWritePtr.hpp>
using namespace MCF;

struct foo : public CopyOnWriteBase<foo> {
	foo(){
		std::puts("foo::foo()");
	}
	foo(const foo &){
		std::puts("foo::foo(const foo &)");
	}
	foo(foo &&) noexcept {
		std::puts("foo::foo(foo &&)");
	}
	foo &operator=(const foo &){
		std::puts("foo::operator=(const foo &)");
		return *this;
	}
	foo &operator=(foo &&) noexcept {
		std::puts("foo::operator=(foo &&)");
		return *this;
	}
	~foo(){
		std::puts("foo::~foo()");
	}
};

extern "C" unsigned int MCFMain() noexcept {
	CopyOnWritePtr<foo> p(new foo);
	auto p2 = p;
	p.TakeOver();

	return 0;
}
