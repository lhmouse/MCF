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
	MCF::PolyIntrusivePtr<Base1>   pb2 = pmy;
	auto pmy2 = MCF::StaticPointerCast<MyClass>(pb1);

	std::printf("use_count = %zu\n", pmy.GetRef());

	return 0;
}
