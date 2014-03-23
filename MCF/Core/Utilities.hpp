// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UTILITIES_HPP__
#define __MCF_UTILITIES_HPP__

#include "../../MCFCRT/c/ext/memcchr.h"
#include "../../MCFCRT/env/bail.h"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

#ifdef NDEBUG
#	define __MCF_CPP_NORETURN_IF_NDEBUG	[[noreturn]]
#else
#	define __MCF_CPP_NORETURN_IF_NDEBUG
#endif

template<typename... Params_t>
__MCF_CPP_NORETURN_IF_NDEBUG inline void Bail(const wchar_t *pwszFormat, const Params_t &... vParams){
	::__MCF_CRT_BailF(pwszFormat, vParams...);
}

template<>
__MCF_CPP_NORETURN_IF_NDEBUG inline void Bail<>(const wchar_t *pwszDescription){
	::__MCF_CRT_Bail(pwszDescription);
}

template<typename T>
inline auto Clone(T &&vSrc) -> typename std::remove_cv<typename std::remove_reference<T>::type>::type {
	return std::forward<T>(vSrc);
}

template<typename Tx, typename Ty>
inline void BCopy(Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "MCF::BCopy(): Only trivial types are supported");
	static_assert(sizeof(vDst) == sizeof(vSrc), "MCF::BCopy(): Source and destination sizes do not match.");
	__builtin_memcpy(&vDst, &vSrc, sizeof(vDst));
}

template<typename T>
inline void BFill(T &vDst, bool bVal) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::BFill(): Only trivial types are supported");
	__builtin_memset(&vDst, bVal ? -1 : 0, sizeof(vDst));
}

template<typename T>
inline void BZero(T &vDst) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::BZero(): Only trivial types are supported");
	__builtin_memset(&vDst, 0, sizeof(vDst));
}

template<typename T>
inline bool BTest(const T &vSrc) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::BTest(): Only trivial types are supported");
	return ::_memcchr(&vSrc, 0, sizeof(vSrc)) == nullptr;
}

template<typename Tx, typename Ty>
inline int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "MCF::BComp(): Only trivial types are supported");
	static_assert(sizeof(vDst) == sizeof(vSrc), "MCF::BComp(): Source and destination sizes do not match.");
	return __builtin_memcmp(&vDst, &vSrc, sizeof(vDst));
}

}

#endif
