#include <MCFCRT/env/heap.h>

static void heap_callback(void *ptr_new, std::size_t size_new, void *ptr_old, const void *ret_outer, const void *ret_inner){
	__builtin_printf("heap_callback(%p, %2u, %p, %p, %p)\n", ptr_new, (unsigned)size_new, ptr_old, ret_outer, ret_inner);
}

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	::_MCFCRT_SetHeapCallback(&heap_callback);
	auto p1 = ::__MCFCRT_HeapAlloc(30, true, nullptr);
	p1 = ::__MCFCRT_HeapRealloc(p1, 20, true, nullptr);
	auto p2 = ::__MCFCRT_HeapAlloc(15, true, nullptr);
	p1 = ::__MCFCRT_HeapRealloc(p1, 40, true, nullptr);
	::__MCFCRT_HeapFree(p1, nullptr);
	::__MCFCRT_HeapFree(p2, nullptr);
	return 0;
}
