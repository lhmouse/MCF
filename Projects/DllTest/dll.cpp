#include <MCF/StdMCF.hpp>
#include <MCF/Core/StringObserver.hpp>
#include <cstdio>

using namespace MCF;

extern "C" {

__declspec(dllexport) int __stdcall dlltest(int a, int b) noexcept {
	static auto p = std::make_unique<int>();
	return a + b;
}

bool MCFDll_OnProcessAttach(void *hDll, bool bDynamic) noexcept {
	std::printf("on process attach: hDll = %p, dynamic = %d\n", hDll, bDynamic);
	return true;
}
void MCFDll_OnProcessDetach(void *hDll, bool bDynamic) noexcept {
	std::printf("on process detach: hDll = %p, dynamic = %d\n", hDll, bDynamic);
}

void MCFDll_OnThreadAttach(void *hDll) noexcept {
	std::printf("on thread attach: hDll = %p\n", hDll);
}
void MCFDll_OnThreadDetach(void *hDll) noexcept {
	std::printf("on thread detach: hDll = %p\n", hDll);
}

}
