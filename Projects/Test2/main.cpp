#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Utilities.hpp>
using namespace MCF;

char data1[1024] = "meow";
char data2[1024] = "meow";

extern "C" unsigned int MCFMain() noexcept {
	BSwap(data1, data2);
	return 0;
}
