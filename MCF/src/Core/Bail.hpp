// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_BAIL_HPP_
#define MCF_CORE_BAIL_HPP_

#include <MCFCRT/env/bail.h>

namespace MCF {

[[noreturn]] inline void Bail(const wchar_t *pwszDescription) noexcept {
	::_MCFCRT_Bail(pwszDescription);
}

}

#endif
