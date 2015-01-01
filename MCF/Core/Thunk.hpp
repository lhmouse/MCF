// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_THUNK_HPP_
#define MCF_CORE_THUNK_HPP_

#include <cstddef>
#include "../Core/UniqueHandle.hpp"

namespace MCF {

struct ThunkDeleter {
	constexpr void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(void *pThunk) const noexcept;
};

using ThunkPtr = UniqueHandle<ThunkDeleter>;

extern ThunkPtr CreateThunk(const void *pInit, std::size_t uSize);

}

#endif
