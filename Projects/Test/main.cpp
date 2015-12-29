#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <cstdio>

using namespace MCF;

struct foo : PolyIntrusiveBase {
	foo();
};

PolyIntrusiveWeakPtr<foo> gp;

foo::foo(){
	gp = this->Weaken<foo>();
	std::printf("inside foo::foo(), gp is now %p\n", (void *)gp.Lock().Get());
	throw 12345; // ok.
}

extern "C" unsigned MCFMain(){
	try {
		auto p = MakeIntrusive<foo>();
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}
	std::printf("inside main(), gp is now %p\n", (void *)gp.Lock().Get());

	return 0;
}
