// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.hpp"
#include "../env/heap.h"
#include "../env/heap_dbg.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include <new>
#include <cstddef>

namespace {

#ifndef NDEBUG

std::uintptr_t GetMagic(bool bIsArray){
	return (std::uintptr_t)::EncodePointer((void *)(bIsArray ?
#ifdef _WIN64
		0xDEADBEEFDEADBEEF : 0xBEEFDEADBEEFDEAD
#else
		0xDEADBEEF : 0xBEEFDEAD
#endif
		));
}

#endif

static_assert(sizeof(std::uintptr_t) <= sizeof(std::max_align_t), "wtf?");

void *Allocate(std::size_t uSize, bool bIsArray, const void *pRetAddr){
#ifdef __MCF_CRT_HEAPDBG_ON
	const auto uSizeToAlloc = sizeof(std::max_align_t) + uSize;
	if(uSizeToAlloc < uSize){
		throw std::bad_alloc();
	}
#else
	const auto uSizeToAlloc = uSize;
#endif
	auto pRaw = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	while(!pRaw){
		const auto pfnHandler = std::get_new_handler();
		if(!pfnHandler){
			throw std::bad_alloc();
		}
		(*pfnHandler)();
		pRaw = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	}
#ifdef __MCF_CRT_HEAPDBG_ON
	*(std::uintptr_t *)pRaw = GetMagic(bIsArray);
	return (unsigned char *)pRaw + sizeof(std::max_align_t);
#else
	(void)bIsArray;
	return pRaw;
#endif
}
void *AllocateNoThrow(std::size_t uSize, bool bIsArray, const void *pRetAddr) noexcept {
#ifdef __MCF_CRT_HEAPDBG_ON
	const auto uSizeToAlloc = sizeof(std::max_align_t) + uSize;
	if(uSizeToAlloc < uSize){
		return nullptr;
	}
#else
	const auto uSizeToAlloc = uSize;
#endif
	auto pRaw = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	if(!pRaw){
		try {
			do {
				const auto pfnHandler = std::get_new_handler();
				if(!pfnHandler){
					return nullptr;
				}
				(*pfnHandler)();
				pRaw = ::__MCF_CRT_HeapAlloc(uSizeToAlloc, pRetAddr);
			} while(!pRaw);
		} catch(std::bad_alloc &){
			return nullptr;
		}
	}
#ifdef __MCF_CRT_HEAPDBG_ON
	*(std::uintptr_t *)pRaw = GetMagic(bIsArray);
	return pRaw + sizeof(std::max_align_t);
#else
	(void)bIsArray;
	return pRaw;
#endif
}
void Deallocate(void *pBlock, bool bIsArray, const void *pRetAddr) noexcept {
	if(!pBlock){
		return;
	}
#ifdef __MCF_CRT_HEAPDBG_ON
	void *const pRaw = (unsigned char *)pBlock - sizeof(std::max_align_t);
	if(*(std::uintptr_t *)pRaw != GetMagic(bIsArray)){
		MCF_CRT_BailF(
			L"试图使用 operator delete%ls 释放不是由 operator new%ls 分配的内存。\n调用返回地址：%0*zX",
			bIsArray ? L"[]" : L"",
			bIsArray ? L"[]" : L"",
			(int)(sizeof(std::size_t) * 2), (std::size_t)pRetAddr);
	}
#else
	(void)bIsArray;
	void *const pRaw = pBlock ;
#endif
	::__MCF_CRT_HeapFree(pRaw, pRetAddr);
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
