#include <MCF/StdMCF.hpp>
#include <MCF/Core/Time.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	TimeBenchmarker t([](auto t){
		std::printf("elasped = %f\n", t);
	});
	::Sleep(1000);

	return 0;
}
