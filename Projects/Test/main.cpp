#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>
#include <MCF/Utilities/Thunk.hpp>
#include <MCF/Core/Random.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	static const char data[4096] = { };
	MCF::Vector<MCF::Thunk> v;
	for(unsigned i = 0; i < 100000; ++i){
		if((MCF::GetRandomUint32() % 3 == 1) && !v.IsEmpty()){
			v.Pop();
		} else {
			auto t = MCF::Thunk(data, MCF::GetRandomUint32() % sizeof(data));
			v.Push(std::move(t));
		}
	}
	return 0;
}
