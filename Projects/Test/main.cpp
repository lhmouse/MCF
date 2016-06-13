#include <MCF/StdMCF.hpp>
#include <MCF/Containers/FlatSet.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::FlatSet<int> s{6,9,4,0,8,3,1,5,2,7};
	for(auto p = begin(s); p != end(s); ++p){
		std::printf("%d ", *p);
	}
	std::putchar('\n');
	return 0;
}
