// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_COUNT_OF_HPP_
#define MCF_UTILITIES_COUNT_OF_HPP_

#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename Ty, std::size_t N>
	constexpr std::size_t CountOfHelper(Ty (&)[N]) noexcept {
		return N;
	}

	template<typename Ty, std::size_t N>
	constexpr std::size_t CountOfHelper(Ty (&&)[N]) noexcept {
		return N;
	}
}

}

#define COUNT_OF(x_)		(::MCF::Impl::CountOfHelper(x_))

#endif
