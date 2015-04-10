#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/CopyOnWriteIntrusivePtr.hpp>
using namespace MCF;

struct foo : CopyOnWriteIntrusiveBase<foo> {
	int i = 12345;

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
	auto p = MakeCopyOnWriteIntrusive<foo>();
	auto p2 = p;

	std::printf("-- equal? %d\n", p == p2);
	p.TakeOver();
	std::printf("-- equal? %d\n", p == p2);

	std::printf("p->i = %d, p2->i = %d\n", p->i, p2->i);

	return 0;
}
