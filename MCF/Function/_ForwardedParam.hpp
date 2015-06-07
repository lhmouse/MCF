// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FORWARDED_PARAM_HPP_
#define MCF_FUNCTION_FORWARDED_PARAM_HPP_

#include <type_traits>

namespace MCF {

template<typename T>
using ForwardedParam = std::conditional_t<!std::is_reference<T>::value && std::is_scalar<std::decay_t<T>>::value, std::decay_t<T>, T &&>;

}

#endif
