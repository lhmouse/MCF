#include <MCF/StdMCF.hpp>
#include <MCF/Core/SharedHandle.hpp>
using namespace MCF;

struct foo {
	foo() noexcept {
		std::puts("ctor of foo");
	}
	foo(const foo &) noexcept {
		std::puts("cp ctor of foo");
	}
	foo(foo &&) noexcept {
		std::puts("mv ctor of foo");
	}
	~foo() noexcept {
		std::puts("dtor of foo");
	}
};

struct deleter {
	constexpr foo *operator()() const noexcept {
		return nullptr;
	}
	void operator()(foo *p) const noexcept {
		delete p;
	}
};

extern "C" unsigned int MCFMain() noexcept {
	SharedHandle<deleter> sp1, sp2;
	WeakHandle<deleter> wp1, wp2;

	UniqueHandle<deleter> up(new foo());
	std::printf("--- up : reset new, up = %p\n", up.Get());
	sp1.Reset(std::move(up));
	std::printf("--- sp1: reset from up, sp1 = %p\n", sp1.Get());
	sp2 = /*std::move*/(sp1);
	std::printf("--- sp2: move from sp1, sp1 = %p, sp2 = %p\n", sp1.Get(), sp2.Get());
	{
		wp1 = sp1;
		std::printf("--- wp1: assign from sp1, weak = %zu, strong = %zu\n", wp1.GetWeakRefCount(), wp1.GetRefCount());
		wp2 = sp2;
		std::printf("--- wp2: assign from sp2, weak = %zu, strong = %zu\n", wp2.GetWeakRefCount(), wp2.GetRefCount());
	}
	std::printf("--- wp2: lock = %p\n", wp2.Lock().Get());
	sp2.Reset();
	std::printf("--- sp2: reset, sp2 = %p\n", sp2.Get());
	sp1.Reset();
	std::printf("--- sp1: reset, sp1 = %p\n", sp1.Get());
	std::printf("--- wp2: lock = %p\n", wp2.Lock().Get());
	std::printf("--- wp1: weak = %zu, strong = %zu\n", wp1.GetWeakRefCount(), wp1.GetRefCount());
	wp1.Reset();
	std::printf("--- wp1: reset, lock = %p\n", wp1.Lock().Get());

	return 0;
}
