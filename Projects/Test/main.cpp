#include <MCF/StdMCF.hpp>
#include <MCF/Function/Bind.hpp>

extern "C" unsigned MCFCRT_Main(){
	auto fn = MCF::Bind(std::plus<>(), MCF::_1, 5);
	int a;
	std::scanf("%d", &a);
	std::printf("fn(a) = %d\n", fn(a));
	return 0;
}
