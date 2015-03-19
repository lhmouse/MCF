#include <MCF/StdMCF.hpp>
#include <MCF/Core/Print.hpp>
#include <MCF/SmartPointers/PolymorphicSharedPtr.hpp>
using namespace MCF;

struct Incomplete;

namespace {

PolymorphicSharedPtr<Incomplete> gp;

}

extern "C" unsigned int MCFMain() noexcept {
	PolymorphicSharedPtr<Incomplete> sp = gp;
	return 0;
}
