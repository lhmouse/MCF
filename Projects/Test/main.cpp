#include <MCF/StdMCF.hpp>
#include <MCF/Function/Function.hpp>
#include <MCF/Function/Bind.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	Function<void(int, int)> f;
//	f = [](int){};
	f = Bind([](int){}, _1);

	return 0;
}
