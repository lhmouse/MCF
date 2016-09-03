#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <cstdio>

struct Common : MCF::PolyIntrusiveBase<Common> {
};

struct Base1 : MCF::PolyIntrusiveBase<Base1>, virtual Common {
};
struct Base2 : MCF::PolyIntrusiveBase<Base2>, virtual Common {
};

struct MyClass : MCF::PolyIntrusiveBase<MyClass>, Base1, Base2 {
};

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::PolyIntrusivePtr<MyClass> pmy = MCF::MakeIntrusive<MyClass>();
	MCF::PolyIntrusivePtr<Base1>   pb1 = pmy;
	MCF::PolyIntrusivePtr<Base2>   pb2 = pmy;
	std::printf("use_count = %zu\n", pmy.GetRef());
	auto pmy2 = MCF::StaticPointerCast<MyClass>(pb2);
	std::printf("pb2 = %p, pmy2 = %p, eq = %d\n", (void *)pb2.Get(), (void *)pmy2.Get(), pb2 == pmy2);
	return 0;
}
