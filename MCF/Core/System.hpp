// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_SYSTEM_HPP_
#define MCF_CORE_SYSTEM_HPP_

#include "../../MCFCRT/env/system.h"

namespace MCF {

inline std::size_t GetLogicalProcessorCount() noexcept {
	return ::MCF_GetLogicalProcessorCount();
}
inline std::size_t GetPageSize() noexcept {
	return ::MCF_GetPageSize();
}

using WindowsVersion = ::MCF_WindowsVersion;

inline WindowsVersion GetWindowsVersion() noexcept {
	WindowsVersion vVersion;
	::MCF_GetWindowsVersion(&vVersion);
	return vVersion;
}

}

#endif
