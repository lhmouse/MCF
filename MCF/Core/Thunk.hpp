// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_THUNK_HPP_
#define MCF_CORE_THUNK_HPP_

#include <cstddef>
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

struct ThunkDeleter {
	constexpr const void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(const void *pThunk) const noexcept;
};

extern UniquePtr<const void, ThunkDeleter> CreateThunk(const void *pInit, std::size_t uSize);

}

#endif
