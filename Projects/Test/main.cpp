#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>
#include <vector>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	Vector<int> v;

	std::vector<int> l;
	for(int i = 0; i < 100; ++i){
		l.push_back(i);
	}

	v.CopyToEnd(l.begin(), l.end());

	return 0;
}
