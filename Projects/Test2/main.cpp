#include <MCF/StdMCF.hpp>
#include <MCF/Core/Thunk.hpp>
#include <iostream>
#include <vector>
using namespace MCF;


extern "C" unsigned int MCFMain() noexcept {
	constexpr char init[1000] = { };
	std::vector<std::shared_ptr<const void>> v;
	try {
		for(;;)
			v.emplace_back(AllocateThunk(init, sizeof(init)));
	} catch(std::bad_alloc &){
	}
	std::cout <<"allocated thunks = " <<v.size();
	return 0;
}
