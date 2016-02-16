#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <cstdio>

using namespace MCF;

struct foo : PolyIntrusiveBase {
	foo();
};

PolyIntrusiveWeakPtr<foo> gp;

foo::foo(){
	std::printf("weak ref = %zu\n", gp.GetWeakRef());
	gp = this->Weaken<foo>();
	std::printf("inside foo::foo(), gp is now %p\n", (void *)gp.Lock().Get());
	std::printf("weak ref = %zu\n", gp.GetWeakRef());
	throw 12345; // ok.
}

extern "C" unsigned MCFCRT_Main(){
	std::printf("weak ref = %zu\n", gp.GetWeakRef());
	try {
		auto p = MakeIntrusive<foo>();
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	std::printf("inside main(), gp is now %p\n", (void *)gp.Lock().Get());
	std::printf("weak ref = %zu\n", gp.GetWeakRef());

	return 0;
}
