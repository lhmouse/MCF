// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_CLONE_HPP_
#define MCF_UTILITIES_CLONE_HPP_

#include <type_traits>

namespace MCF {

template<typename T>
inline auto Clone(T &&vSrc)
	noexcept(std::is_nothrow_constructible<std::remove_reference_t<T>, T &&>::value)
{
	return std::remove_cv_t<std::remove_reference_t<T>>(std::forward<T>(vSrc));
}

}

#endif
