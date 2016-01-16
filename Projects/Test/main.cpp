#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <cstdio>

template class MCF::UniquePtr<int>;

extern "C" unsigned MCFCRT_Main(){
	MCF::UniquePtr<int> p1, p2;
	p1.Reset(new int(123));
	p2.Reset(new int(456));
	swap(p1, p2);
	std::printf("%d %d\n", *p1, *p2);
	return 0;
}
