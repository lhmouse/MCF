#include <MCF/StdMCF.hpp>
#include <set>
#include <cstdlib>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::multiset<int> s;
	for(int i = 0; i < 0x100000; ++i){
		s.insert(std::rand());
	}
	return 0;
}
