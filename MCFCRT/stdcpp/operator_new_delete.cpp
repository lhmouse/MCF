// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.hpp"
#include "../env/heap.h"
#include "../env/heap_dbg.h"
#include "../env/mcfwin.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"
#include <new>
#include <cstddef>

namespace {

#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)

inline std::uintptr_t GetMagic(void *pRaw, bool bIsArray){
	const auto uEncoded = reinterpret_cast<std::uintptr_t>(::EncodePointer(pRaw));
	const auto uMask = static_cast<std::uintptr_t>(bIsArray ? 0xDEADBEEFDEADBEEF : 0xBEEFDEADBEEFDEAD);
	return uEncoded ^ uMask;
}

#endif

static_assert(sizeof(std::uintptr_t) <= alignof(std::max_align_t), "wtf?");

void *Allocate(std::size_t uSize, bool bIsArray, const void *pRetAddr){
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const auto uSizeToAlloc = alignof(std::max_align_t) + uSize;
	if(uSizeToAlloc < uSize){
		throw std::bad_alloc();
	}
#else
	const auto uSizeToAlloc = uSize;
#endif
	void *pRaw = ::__MCFCRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	while(!pRaw){
		const auto pfnHandler = std::get_new_handler();
		if(!pfnHandler){
			throw std::bad_alloc();
		}
		(*pfnHandler)();
		pRaw = ::__MCFCRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	}
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const auto uMagic = GetMagic(pRaw, bIsArray);
	for(std::size_t i = 0; i < alignof(std::max_align_t) / sizeof(std::uintptr_t); ++i){
		static_cast<std::uintptr_t *>(pRaw)[i] = uMagic;
	}
	return static_cast<char *>(pRaw) + alignof(std::max_align_t);
#else
	(void)bIsArray;
	return pRaw;
#endif
}
void *AllocateNoThrow(std::size_t uSize, bool bIsArray, const void *pRetAddr) noexcept {
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const auto uSizeToAlloc = alignof(std::max_align_t) + uSize;
	if(uSizeToAlloc < uSize){
		return nullptr;
	}
#else
	const auto uSizeToAlloc = uSize;
#endif
	void *pRaw = ::__MCFCRT_HeapAlloc(uSizeToAlloc, pRetAddr);
	if(!pRaw){
		try {
			do {
				const auto pfnHandler = std::get_new_handler();
				if(!pfnHandler){
					return nullptr;
				}
				(*pfnHandler)();
				pRaw = ::__MCFCRT_HeapAlloc(uSizeToAlloc, pRetAddr);
			} while(!pRaw);
		} catch(std::bad_alloc &){
			return nullptr;
		}
	}
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	const auto uMagic = GetMagic(pRaw, bIsArray);
	for(std::size_t i = 0; i < alignof(std::max_align_t) / sizeof(std::uintptr_t); ++i){
		static_cast<std::uintptr_t *>(pRaw)[i] = uMagic;
	}
	return static_cast<char *>(pRaw) + alignof(std::max_align_t);
#else
	(void)bIsArray;
	return pRaw;
#endif
}
void Deallocate(void *pBlock, bool bIsArray, const void *pRetAddr) noexcept {
	if(!pBlock){
		return;
	}
#if __MCFCRT_REQUIRE_HEAPDBG_LEVEL(2)
	void *const pRaw = static_cast<char *>(pBlock) - alignof(std::max_align_t);
	const auto uMagic = GetMagic(pRaw, bIsArray);
	const auto uOtherMagic = GetMagic(pRaw, !bIsArray);
	for(std::size_t i = 0; i < alignof(std::max_align_t) / sizeof(std::uintptr_t); ++i){
		const auto uTest = static_cast<std::uintptr_t *>(pRaw)[i];
		if(uTest != uMagic){
			wchar_t awcBuffer[1024];
			wchar_t *pwcWrite;
			if(uTest == uOtherMagic){
				pwcWrite = _MCFCRT_wcpcpy(awcBuffer, L"试图使用 operator delete");
				if(bIsArray){
					pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"[]");
				}
				pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"() 释放由 operator new");
				if(!bIsArray){
					pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"[]");
				}
				pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"() 分配的内存。\n\n返回地址：");
				pwcWrite = _MCFCRT_itow0X(pwcWrite, (std::uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
			} else {
				pwcWrite = _MCFCRT_wcpcpy(awcBuffer, L"在 operator delete");
				if(bIsArray){
					pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"[]");
				}
				pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"() 中侦测到堆损坏。\n\n返回地址：");
				pwcWrite = _MCFCRT_itow0X(pwcWrite, (std::uintptr_t)pRetAddr, sizeof(pRetAddr) * 2);
			}
			*pwcWrite = 0;
			_MCFCRT_Bail(awcBuffer);
		}
	}
#else
	(void)bIsArray;
	void *const pRaw = pBlock ;
#endif
	::__MCFCRT_HeapFree(pRaw, pRetAddr);
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
