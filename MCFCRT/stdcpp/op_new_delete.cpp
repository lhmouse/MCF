// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.hpp"
#include "../env/heap.h"
#include "../env/bail.h"
#include <new>
#include <cstddef>

namespace {

inline __attribute__((__always_inline__))
void *Allocate(std::size_t uSize, bool bIsArray, const void *pRetAddr){
	const auto uSizeToAlloc = sizeof(std::max_align_t) + uSize;
	if(uSizeToAlloc < uSize){
		throw std::bad_alloc();
	}
	auto pbyRaw = (unsigned char *)::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	while(!pbyRaw){
		const auto pfnHandler = std::get_new_handler();
		if(!pfnHandler){
			throw std::bad_alloc();
		}
		(*pfnHandler)();
		pbyRaw = (unsigned char *)::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	}
	pbyRaw[0] = bIsArray;
	return pbyRaw + sizeof(std::max_align_t);
}

inline __attribute__((__always_inline__))
void *AllocateNoThrow(std::size_t uSize, bool bIsArray, const void *pRetAddr) noexcept {
	const auto uSizeToAlloc = sizeof(std::max_align_t) + uSize;
	if(uSizeToAlloc < uSize){
		return nullptr;
	}
	auto pbyRaw = (unsigned char *)::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	if(!pbyRaw){
		try {
			do {
				const auto pfnHandler = std::get_new_handler();
				if(!pfnHandler){
					return nullptr;
				}
				(*pfnHandler)();
				pbyRaw = (unsigned char *)::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
			} while(!pbyRaw);
		} catch(std::bad_alloc &){
			return nullptr;
		}
	}
	pbyRaw[0] = bIsArray;
	return pbyRaw + sizeof(std::max_align_t);
}

inline __attribute__((__always_inline__))
void Deallocate(void *pBlock, bool bIsArray, const void *pRetAddr) noexcept {
	if(!pBlock){
		return;
	}
	const auto pbyRaw = (unsigned char *)pBlock - sizeof(std::max_align_t);
	if(bIsArray != pbyRaw[0]){
		MCF_CRT_BailF(L"试图使用 operator delete%ls() 释放 operator new%ls() 分配的内存。\n调用返回地址：%0*zX",
			bIsArray ? L"" : L"[]", bIsArray ? L"[]" : L"", (int)(sizeof(std::size_t) * 2), (std::size_t)pRetAddr);
	}
	::__MCF_CRT_HeapFree(pbyRaw, pRetAddr);
}

}

#pragma GCC diagnostic ignored "-Wattributes"

__attribute__((__noinline__))
void *operator new(std::size_t cb){
	return Allocate(cb, false, __builtin_return_address(0));
}
__attribute__((__noinline__))
void *operator new(std::size_t cb, const std::nothrow_t &) noexcept {
	return AllocateNoThrow(cb, false, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete(void *p) noexcept {
	Deallocate(p, false, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete(void *p, const std::nothrow_t &) noexcept {
	Deallocate(p, false, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete(void *p, std::size_t) noexcept {
	Deallocate(p, false, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete(void *p, std::size_t, const std::nothrow_t &) noexcept {
	Deallocate(p, false, __builtin_return_address(0));
}

__attribute__((__noinline__))
void *operator new[](std::size_t cb){
	return Allocate(cb, true, __builtin_return_address(0));
}
__attribute__((__noinline__))
void *operator new[](std::size_t cb, const std::nothrow_t &) noexcept {
	return AllocateNoThrow(cb, true, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete[](void *p) noexcept {
	Deallocate(p, true, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete[](void *p, std::size_t) noexcept {
	Deallocate(p, true, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete[](void *p, const std::nothrow_t &) noexcept {
	Deallocate(p, true, __builtin_return_address(0));
}
__attribute__((__noinline__))
void operator delete[](void *p, std::size_t, const std::nothrow_t &) noexcept {
	Deallocate(p, true, __builtin_return_address(0));
}
