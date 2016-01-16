#include <MCF/StdMCF.hpp>
#include <MCF/SmartPointers/UniquePtr.hpp>
#include <cstdio>

template class MCF::UniquePtr<int>;

struct d1 {
	constexpr int *operator()() noexcept {
		return nullptr;
	}
	void operator()(int *) noexcept {
	}
};
struct d2 {
	constexpr int *operator()() noexcept {
		return nullptr;
	}
	void operator()(int *) noexcept {
	}
};

extern "C" unsigned MCFCRT_Main(){
	MCF::UniquePtr<int, d1> p1;
	MCF::UniquePtr<int, d2> p2;
	std::printf("%d\n", p1 == p2);

	return 0;
}
