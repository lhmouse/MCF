// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DEFAULT_ALLOCATOR_HPP_
#define MCF_CORE_DEFAULT_ALLOCATOR_HPP_

#include "../Config.hpp"
#include <new>
#include <cstddef>

namespace MCF {

struct DefaultAllocator {
	__attribute__((__malloc__))
	void *operator()(std::size_t uSize){
		return ::operator new(uSize);
	}
	__attribute__((__malloc__))
	void *operator()(const std::nothrow_t &, std::size_t uSize) noexcept {
		return ::operator new(uSize, std::nothrow);
	}
	void operator()(void *pBlock) noexcept {
		::operator delete(pBlock);
	}
};

}

#endif
