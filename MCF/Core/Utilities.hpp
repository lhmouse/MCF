// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UTILITIES_HPP__
#define __MCF_UTILITIES_HPP__

#include "String.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

#ifdef NDEBUG
#	define __MCF_CPP_NORETURN_IF_NDEBUG	[[noreturn]]
#else
#	define __MCF_CPP_NORETURN_IF_NDEBUG
#endif

__MCF_CPP_NORETURN_IF_NDEBUG extern void Bail(const wchar_t *pwszDescription);

extern UTF16String GetWin32ErrorDesc(unsigned long ulErrorCode);

template<typename T>
inline auto Clone(T &&vSrc) -> typename std::remove_cv<typename std::remove_reference<T>::type>::type {
	return std::forward<T>(vSrc);
}

template<typename T>
inline void Zero(T &vDst) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::Zero(): Only trivial types are supported");
	__builtin_memset(&vDst, 0, sizeof(vDst));
}

}

#endif
