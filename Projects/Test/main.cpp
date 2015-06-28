#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/IntrusivePtr.hpp>

using namespace MCF;

struct foo : IntrusiveBase<foo> {
	foo(int, const char *);
	~foo() override;
};

struct bar : foo {
	bar()
		: foo(1, "hello")
	{
	}
};

template class IntrusiveBase<foo>;
template class IntrusivePtr<foo>;
template class IntrusiveWeakPtr<foo>;

IntrusivePtr<foo> gp;

foo::foo(int i, const char *s){
	std::printf("- foo::foo(%d, %s)\n", i, s);

	gp = Share();
}
foo::~foo(){
	std::printf("- foo::~foo()\n");
}

extern "C" unsigned MCFMain(){
	IntrusivePtr<bar> p;
	IntrusiveWeakPtr<foo> wp;

	p = MakeIntrusive<bar>();
	wp = p;
	std::printf("p = %p, gp = %p, wp.Lock() = %p\n", p.Get(), gp.Get(), wp.Lock().Get());

	p.Reset();
	std::printf("p = %p, gp = %p, wp.Lock() = %p\n", p.Get(), gp.Get(), wp.Lock().Get());

	gp.Reset();
	std::printf("p = %p, gp = %p, wp.Lock() = %p\n", p.Get(), gp.Get(), wp.Lock().Get());

	return 0;
}
