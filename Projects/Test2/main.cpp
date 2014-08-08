#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/StreamFilters/IsaacExFilters.hpp>
using namespace MCF;

extern "C" {

void MCF_OnHeapAlloc(void *pBlock, std::size_t uBytes, const void *){
	std::printf("  heap alloc   = %p, %zu\n", pBlock, uBytes);
}
void MCF_OnHeapDealloc(void *pBlock, const void *){
	std::printf("  heap dealloc = %p\n", pBlock);
}

}

extern "C" unsigned int MCFMain() noexcept {
std::puts("1 ==========");
	StreamBuffer buf;
	const auto data = "hello world!"_nso;
std::puts("2 ==========");
	buf.Insert(data.GetBegin(), data.GetSize());
std::puts("3 ==========");
	IsaacExEncoder("key", 3).FilterInPlace(buf);
	std::printf("encoded = %zu bytes\n", buf.GetSize());
std::puts("4 ==========");
	IsaacExDecoder("key", 3).FilterInPlace(buf);
std::puts("5 ==========");
	auto v = buf.Squash();
std::puts("6 ==========");
	v.Push((unsigned)'$');
	v.Push(0);
std::puts("7 ==========");
	std::printf("decoded = %s\n", v.GetData());
std::puts("8 ==========");
	return 0;
}
