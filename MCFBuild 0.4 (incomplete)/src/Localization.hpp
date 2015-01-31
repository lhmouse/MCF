// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2015, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_LOCALIZATION_HPP_
#define MCFBUILD_LOCALIZATION_HPP_

#include <MCF/Core/String.hpp>

namespace MCFBuild {

struct Localization {
	static MCF::WideString &Get(MCF::WideString &wcsAppendsTo, const char *pszKey);

private:
	Localization() = delete;
};

}

#endif
