#include <MCF/StdMCF.hpp>
#include <MCF/Core/IntrusivePtr.hpp>
using namespace MCF;

struct foo : IntrusiveBase<foo> {
};

template class IntrusivePtr<foo>;

extern "C" unsigned int MCFMain() noexcept {
	return 0;
}
