// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_BAIL_HPP_
#define MCF_UTILITIES_BAIL_HPP_

#include "../../MCFCRT/env/bail.h"

namespace MCF {

template<typename ...ParamsT>
inline void Bail(const wchar_t *pwszFormat, const ParamsT &...vParams){
	::MCF_CRT_BailF(pwszFormat, vParams...);
}

template<>
inline void Bail<>(const wchar_t *pwszDescription){
	::MCF_CRT_Bail(pwszDescription);
}

}

#endif
