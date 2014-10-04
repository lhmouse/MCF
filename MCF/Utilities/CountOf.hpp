// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_COUNT_OF_HPP_
#define MCF_UTILITIES_COUNT_OF_HPP_

#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename Ty, std::size_t N>
	auto CountOfHelper(Ty (&)[N]) -> char(&)[N];

	template<typename Ty, std::size_t N>
	auto CountOfHelper(Ty (&&)[N]) -> char(&&)[N];
}

}

#define COUNT_OF(a)		(sizeof(::MCF::Impl::CountOfHelper(a)))

#endif
