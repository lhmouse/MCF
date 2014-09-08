// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_BailOut_OUT_HPP_
#define MCF_BailOut_OUT_HPP_

#include "../../MCFCRT/env/bail_out.h"

namespace MCF {

template<typename ...ParamsT>
__MCF_NORETURN_IF_NDEBUG inline
void BailOut(const wchar_t *pwszFormat, const ParamsT &...vParams){
	::MCF_CRT_BailOutF(pwszFormat, vParams...);
}

template<>
__MCF_NORETURN_IF_NDEBUG inline
void BailOut<>(const wchar_t *pwszDescription){
	::MCF_CRT_BailOut(pwszDescription);
}

}

#endif
