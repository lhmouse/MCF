#include <MCF/StdMCF.hpp>
#include <MCF/Containers/StaticVector.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::StaticVector<MCF::UniquePtr<int>, 5> v1, v2;
	for(int i = 0; i < 3; ++i){
		v1.Push(MCF::MakeUnique<int>(100 + i));
	}
	for(int i = 0; i < 5; ++i){
		v2.Push(MCF::MakeUnique<int>(200 + i));
	}
	swap(v1, v2);

	std::printf("v1 =");
	for(std::size_t i = 0; i < v1.GetSize(); ++i){
		std::printf(" %d", *v1[i]);
	}
	std::putchar('\n');

	std::printf("v2 =");
	for(std::size_t i = 0; i < v2.GetSize(); ++i){
		std::printf(" %d", *v2[i]);
	}
	std::putchar('\n');

	return 0;
}
