// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_CountOf_HPP_
#define MCF_UTILITIES_CountOf_HPP_

#include <cstddef>

namespace MCF {

template<typename Ty, std::size_t N>
constexpr std::size_t CountOf(const Ty (&)[N]) noexcept {
	return N;
}

}

#endif
