// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UNIQUE_NT_HANDLE_HANDLE_HPP_
#define MCF_CORE_UNIQUE_NT_HANDLE_HANDLE_HPP_

#include "UniqueHandle.hpp"

namespace MCF {

namespace Impl_UniqueNtHandle {
	struct NtHandleCloser {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *hObject) const noexcept;
	};

	using UniqueNtHandle = UniqueHandle<NtHandleCloser>;
}

}

#endif
