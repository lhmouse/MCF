#include <MCF/StdMCF.hpp>
#include <set>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::multiset<int> s;
	for(int i = 0; i < 100000; ++i){
		s.insert(std::rand());
	}

	auto p = new unsigned;

	return 0;
}
