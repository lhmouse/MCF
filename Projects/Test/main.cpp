#include <MCF/StdMCF.hpp>
#include <MCF/Containers/List.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	List<int> l{1,2,3,4,5};
	for(auto &i : l){
		i *= 2;
	}
	for(auto &i : l){
		std::printf("%d\n", i);
	}
	return 0;
}
