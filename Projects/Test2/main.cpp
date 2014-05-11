#include <MCF/StdMCF.hpp>
#include <MCF/Core/Thunk.hpp>
#include <MCF/Core/Random.hpp>
#include <MCF/Core/Utilities.hpp>
#include <cstdlib>
using namespace MCF;

static const char buffer[0x1000] = { };

extern const char begin	__asm__("thunk_begin");
extern const char end	__asm__("thunk_end");

__asm__(
	"thunk_begin: \n"
	"lea eax, dword ptr[ecx + edx] \n"
	"ret \n"
	"thunk_end: \n"
);

unsigned int MCFMain(){
/*
	auto p = AllocateThunk(&begin, (std::size_t)(&end - &begin));
	std::printf("%d\n", (*(int (__fastcall *)(int, int))p.get())(2, 3));
*/
	Random rng(0);
	for(int i = 0; i < 10; ++i){
		std::vector<std::shared_ptr<const void>> v;
		try {
			for(;;){
				if(rng.Get<unsigned char>() % 3 > 0){
					v.emplace_back(AllocateThunk(buffer, rng.Get<std::size_t>() % sizeof(buffer)));
				} else if(!v.empty()){
					const auto idx = rng.Get<std::size_t>() % v.size();
					std::swap(v.at(idx), v.back());
					v.pop_back();
				}
			}
		} catch(std::bad_alloc &){
			std::printf("allocated %zu thunks\n", v.size());
		}
		v.clear();
		v.shrink_to_fit();
		std::puts("cleared");
	}

	return 0;
}
