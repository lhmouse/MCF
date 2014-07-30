// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.hpp"
#include "../env/heap.h"
#include <new>

namespace {

inline __attribute__((__always_inline__)) void *Allocate(std::size_t uSize, const void *pRetAddr){
	const auto uSizeToAlloc = (uSize == 0) ? 1 : uSize;
	auto *pRet = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	while(!pRet){
		const auto pfnHandler = std::get_new_handler();
		if(!pfnHandler){
			throw std::bad_alloc();
		}
		(*pfnHandler)();
		pRet = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	}
	return pRet;
}

inline __attribute__((__always_inline__)) void *AllocateNoThrow(std::size_t uSize, const void *pRetAddr) noexcept {
	const auto uSizeToAlloc = (uSize == 0) ? 1 : uSize;
	auto *pRet = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	if(!pRet) {
		try {
			do {
				const auto pfnHandler = std::get_new_handler();
				if(!pfnHandler){
					return nullptr;
				}
				(*pfnHandler)();
				pRet = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
			} while(!pRet);
		} catch(std::bad_alloc &){
			return nullptr;
		}
	}
	return pRet;
}

inline __attribute__((__always_inline__)) void Deallocate(void *pBlock, const void *pRetAddr) noexcept {
	if(pBlock){
		::__MCF_CRT_HeapFree(pBlock, pRetAddr);
	}
}

}

#pragma GCC diagnostic ignored "-Wattributes"

__attribute__((__noinline__)) void *operator new(std::size_t cb){
	return Allocate(cb, __builtin_return_address(0));
}
__attribute__((__noinline__)) void *operator new(std::size_t cb, const std::nothrow_t &) noexcept {
	return AllocateNoThrow(cb, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete(void *p) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete(void *p, const std::nothrow_t &) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete(void *p, std::size_t) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete(void *p, std::size_t, const std::nothrow_t &) noexcept {
	Deallocate(p, __builtin_return_address(0));
}

__attribute__((__noinline__)) void *operator new[](std::size_t cb){
	return Allocate(cb, __builtin_return_address(0));
}
__attribute__((__noinline__)) void *operator new[](std::size_t cb, const std::nothrow_t &) noexcept {
	return AllocateNoThrow(cb, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete[](void *p) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete[](void *p, std::size_t) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete[](void *p, const std::nothrow_t &) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
__attribute__((__noinline__)) void operator delete[](void *p, std::size_t, const std::nothrow_t &) noexcept {
	Deallocate(p, __builtin_return_address(0));
}
