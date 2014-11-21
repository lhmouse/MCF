#include <MCF/StdMCF.hpp>
#include <MCF/Core/IntrusivePtr.hpp>
using namespace MCF;

struct foo : IntrusiveBase<foo> {
};

template class IntrusivePtr<foo>;

extern "C" unsigned int MCFMain() noexcept {
	IntrusivePtr<foo> p1, p2;
	p1.Reset(new foo);
	p2.Reset(new foo);
	p2.Reset(p1);
	return 0;
}
