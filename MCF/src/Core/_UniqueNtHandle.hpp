// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UNIQUE_NT_HANDLE_HANDLE_HPP_
#define MCF_CORE_UNIQUE_NT_HANDLE_HANDLE_HPP_

#include "UniqueHandle.hpp"

namespace MCF {

namespace Impl_UniqueNtHandle {
	using Handle = void *;

	struct NtHandleCloser {
		constexpr Handle operator()() const noexcept {
			return nullptr;
		}
		void operator()(Handle hObject) const noexcept;
	};
}

extern template class UniqueHandle<Impl_UniqueNtHandle::NtHandleCloser>;

namespace Impl_UniqueNtHandle {
	using UniqueNtHandle = UniqueHandle<NtHandleCloser>;
}

}

#endif
