#include <MCF/StdMCF.hpp>
#include <MCF/Core/Thunk.hpp>
#include <MCF/Random/IsaacRng.hpp>
#include <vector>
using namespace MCF;

constexpr char INIT[0x200000] = { };

extern "C" unsigned int MCFMain() noexcept {
	IsaacRng rng(100);
	std::vector<ThunkPtr> v;
	try {
		for(;;){
			v.emplace_back(CreateThunk(INIT, rng.Get() % sizeof(INIT)));
		}
	} catch(std::bad_alloc &){
		const unsigned long cnt = v.size();
		while(!v.empty()){
			const std::size_t i = rng.Get() % v.size();
			v[i].swap(v.back());
			v.pop_back();
		}
		std::printf("count = %lu\n", cnt);
	}
	return 0;
}
