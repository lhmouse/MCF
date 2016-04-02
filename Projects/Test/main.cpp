#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <MCF/Core/Random.hpp>
#include <MCF/Core/Clocks.hpp>

extern "C" unsigned _MCFCRT_Main(){
	MCF::Vector<MCF::UniquePtr<char []>> v;
	auto t1 = MCF::GetHiResMonoClock();
	for(unsigned i = 0; i < 1000; ++i){
		v.Clear();
		v.Resize(100);
		for(auto &p : v){
			const auto size = MCF::GetRandomUint32() % 65536 * 256;
			p.Reset(new char[size]);
		}
	}
	auto t2 = MCF::GetHiResMonoClock();
	std::printf("delta_t = %f\n", t2 - t1);
	return 0;
}
