#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/IntrusivePtr.hpp>

using namespace MCF;

struct foo : IntrusiveBase<foo> {
	foo(int, const char *){
	}
};

template class IntrusiveBase<foo>;
template class IntrusivePtr<foo>;
template class IntrusiveWeakPtr<foo>;

extern "C" unsigned int MCFMain() noexcept {
	IntrusiveWeakPtr<foo> wp;

	auto p1 = MakeIntrusive<foo>(1, "hello");
	auto p2 = p1;
	wp = p2;
	std::printf("p1 = %p, p2 = %p, wp.lock() = %p\n", p1.Get(), p2.Get(), wp.Lock().Get());

	p1.Reset();
	std::printf("p1 = %p, p2 = %p, wp.lock() = %p\n", p1.Get(), p2.Get(), wp.Lock().Get());

	p2.Reset();
	std::printf("p1 = %p, p2 = %p, wp.lock() = %p\n", p1.Get(), p2.Get(), wp.Lock().Get());

	return 0;
}
