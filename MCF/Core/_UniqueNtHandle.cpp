// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_UniqueNtHandle.hpp"
#include <winternl.h>

namespace MCF {

namespace Impl_UniqueNtHandle {
	void NtHandleCloser::operator()(void *hObject) const noexcept {
		const auto lStatus = ::NtClose(hObject);
		if(!NT_SUCCESS(lStatus)){
			ASSERT_MSG(false, L"::NtClose() 失败。");
		}
	}
}

}
