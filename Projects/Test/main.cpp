#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Vector<int> v(20, 1);

	return 0;
}
