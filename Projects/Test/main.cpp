#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/PolyIntrusivePtr.hpp>
#include <cstdio>

using namespace MCF;

struct foo : PolyIntrusiveBase<foo> {
	foo();
};

PolyIntrusivePtr<foo> gp;

foo::foo(){
	gp = this->Share();
	throw 12345; // calls std::terminate()!
}

extern "C" unsigned MCFMain(){
	try {
		auto p = MakePolyIntrusive<foo>();
	} catch(int e){
		std::printf("exception caught: e = %d\n", e);
	}

	return 0;
}
