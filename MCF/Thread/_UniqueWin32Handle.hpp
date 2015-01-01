// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_WIN32_HANDLE_HPP_
#define MCF_THREAD_WIN32_HANDLE_HPP_

#include "../Core/UniqueHandle.hpp"

namespace MCF {

struct Win32HandleCloser {
	constexpr void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(void *hObject) const noexcept;
};

using UniqueWin32Handle = UniqueHandle<Win32HandleCloser>;

}

#endif
