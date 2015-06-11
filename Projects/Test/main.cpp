#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <iostream>
#include <string>

using namespace MCF;

struct foo : PolyIntrusiveBase<foo> {
	int i = 12345;
};
struct bar : foo {
};

extern "C" unsigned int MCFMain() noexcept {
	PolyIntrusiveWeakPtrUnknown wp1;
	PolyIntrusiveWeakPtr<foo> wp2;

	{
		PolyIntrusivePtrUnknown p1 = MakePolyIntrusive<foo>();
		PolyIntrusivePtr<foo> p2 = DynamicPointerCast<foo>(p1);
		PolyIntrusivePtr<foo> p3 = StaticPointerCast<foo>(p2);
		PolyIntrusivePtr<bar> p4 = DynamicPointerCast<bar>(p3);
		PolyIntrusivePtr<foo> p5 = p2->Share<foo>();
		PolyIntrusivePtr<bar> p6 = p3->Share<bar>();

		wp1 = p1->Weaken();
		wp2 = p2->Weaken<foo>();

		std::printf("p1 = %p\n", p1.Get());
		std::printf("p2 = %p, i = %d\n", p2.Get(), p2->i);
		std::printf("p3 = %p, i = %d\n", p3.Get(), p3->i);
		std::printf("p4 = %p\n", p4.Get());
		std::printf("p5 = %p, i = %d\n", p5.Get(), p5->i);
		std::printf("p6 = %p\n", p6.Get());

		std::printf("wp1 = %d\n", wp1.HasExpired());
		std::printf("wp2 = %d\n", wp2.HasExpired());
	}

	std::printf("wp1 = %d\n", wp1.HasExpired());
	std::printf("wp2 = %d\n", wp2.HasExpired());

	// p1 = 00000000002e01e0
	// p2 = 00000000002e01d0, i = 12345
	// p3 = 00000000002e01d0, i = 12345
	// p4 = 0000000000000000
	// p5 = 00000000002e01d0, i = 12345
	// p6 = 0000000000000000
	// wp1 = 00000000002e01e0
	// wp2 = 00000000002e01d0
	// wp1 = 0000000000000000
	// wp2 = 0000000000000000

	return 0;
}
