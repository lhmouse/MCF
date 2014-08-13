// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_BAIL_HPP_
#define MCF_BAIL_HPP_

#include "../../MCFCRT/env/bail.h"

namespace MCF {

template<typename... Params>
__MCF_NORETURN_IF_NDEBUG inline
void Bail(const wchar_t *pwszFormat, const Params &... vParams){
	::MCF_CRT_BailF(pwszFormat, vParams...);
}

template<>
__MCF_NORETURN_IF_NDEBUG inline
void Bail<>(const wchar_t *pwszDescription){
	::MCF_CRT_Bail(pwszDescription);
}

}

#endif
