// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_WIN_HANDLE_INL_
#define MCF_THREAD_WIN_HANDLE_INL_

#include "../StdMCF.hpp"
#include "../Core/UniqueHandle.hpp"
#include <cstddef>

namespace MCF {

namespace Impl {
	struct WinHandleCloser {
		constexpr HANDLE operator()() const noexcept {
			return NULL;
		}
		void operator()(HANDLE hObject) const noexcept {
			::CloseHandle(hObject);
		}
	};
	using UniqueWinHandle =UniqueHandle<WinHandleCloser>;
}

}

#endif
