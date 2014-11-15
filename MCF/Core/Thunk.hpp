// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_THUNK_HPP_
#define MCF_CORE_THUNK_HPP_

#include <memory>
#include <cstddef>
#include "../../MCFCRT/env/thunk.h"

namespace MCF {

struct ThunkDeleter {
	void operator()(void *pThunk) const noexcept {
		::MCF_CRT_DeallocateThunk(pThunk, false);
	}
};
using ThunkPtr = std::unique_ptr<void, ThunkDeleter>;

struct SafeThunkDeleter {
	void operator()(void *pThunk) const noexcept {
		::MCF_CRT_DeallocateThunk(pThunk, true);
	}
};
using SafeThunkPtr = std::unique_ptr<void, SafeThunkDeleter>;

inline ThunkPtr CreateThunk(const void *pInit, std::size_t uSize){
	ThunkPtr pThunk(::MCF_CRT_AllocateThunk(pInit, uSize));
	if(!pThunk){
		throw std::bad_alloc();
	}
	return std::move(pThunk);
}
inline SafeThunkPtr CreateSafeThunk(const void *pInit, std::size_t uSize){
	return SafeThunkPtr(CreateThunk(pInit, uSize).release());
}

}

#endif
