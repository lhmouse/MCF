// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_SYSTEM_HPP_
#define MCF_CORE_SYSTEM_HPP_

#include <cstddef>

namespace MCF {

std::size_t GetLogicalProcessorCount() noexcept;
std::size_t GetPageSize() noexcept;

struct WindowsVersion {
	unsigned uMajor;
	unsigned uMinor;
	unsigned uBuild;
	const wchar_t *pwszServPack;
};

WindowsVersion GetWindowsVersion() noexcept;

}

#endif
