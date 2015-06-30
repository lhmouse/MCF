#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/IntrusivePtr.hpp>

using namespace MCF;

struct probe {
	probe(){
		std::puts("probe::probe()");
	}
	probe(const probe &){
		std::puts("probe::probe(const probe &)");
	}
	probe(probe &&) noexcept {
		std::puts("probe::probe(probe &&)");
	}
	~probe(){
		std::puts("probe::~probe()");
	}
};

struct foo : IntrusiveBase<foo> {
	probe p;

	foo();
};

template class IntrusiveBase<foo>;
template class IntrusivePtr<foo>;
template class IntrusiveWeakPtr<foo>;

IntrusivePtr<foo> gp;

foo::foo(){
	gp = Share();

	throw 123;
}

extern "C" unsigned MCFMain(){
	try {
		auto p = MakeIntrusive<foo>();
	} catch(int e){
		std::printf("caught exception: e = %d\n", e);
	}
	return 0;
}
