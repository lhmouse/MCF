// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_COUNT_OF_HPP_
#define MCF_CORE_COUNT_OF_HPP_

#include <cstddef>

namespace MCF {

template<typename T, std::size_t N>
constexpr std::size_t CountOf(const T (&)[N]) noexcept {
	return N;
}

}

#endif
