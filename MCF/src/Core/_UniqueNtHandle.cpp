// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_UniqueNtHandle.hpp"
#include <winternl.h>

namespace MCF {

template class UniqueHandle<Impl_UniqueNtHandle::NtHandleCloser>;

namespace Impl_UniqueNtHandle {
	void NtHandleCloser::operator()(Handle hObject) const noexcept {
		const auto lStatus = ::NtClose(hObject);
		MCF_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtClose() 失败。");
	}
}

}
