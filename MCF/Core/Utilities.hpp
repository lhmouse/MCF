// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UTILITIES_HPP__
#define __MCF_UTILITIES_HPP__

#include "../../MCFCRT/c/ext/memcchr.h"
#include "../../MCFCRT/env/bail.h"
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cstdint>

namespace MCF {

//----------------------------------------------------------------------------
// NoCopy
//----------------------------------------------------------------------------
namespace __MCF {
	struct NonCopyable {
		constexpr NonCopyable() noexcept = default;
		~NonCopyable() = default;

		NonCopyable(const NonCopyable &) = delete;
		void operator=(const NonCopyable &) = delete;
		NonCopyable(NonCopyable &&) = delete;
		void operator=(NonCopyable &&) = delete;
	};
}

#define NO_COPY			private ::MCF::__MCF::NonCopyable

//----------------------------------------------------------------------------
// Abstract
//----------------------------------------------------------------------------
namespace __MCF {
	struct Abstract {
		virtual ~Abstract(){ }
		virtual void __MCF_AbstractFunction() = 0;
	};

	template<class Base_t>
	struct Concrete : public Base_t {
		static_assert(std::is_base_of<Abstract, Base_t>::value, "Concreting from non-abstract class?");

		virtual void __MCF_AbstractFunction(){ }
	};
}

#define ABSTRACT		private ::MCF::__MCF::Abstract
#define CONCRETE(base)	public ::MCF::__MCF::Concrete<base>

//----------------------------------------------------------------------------
// Bail
//----------------------------------------------------------------------------
#ifdef NDEBUG
#	define __MCF_CPP_NORETURN_IF_NDEBUG	[[noreturn]]
#else
#	define __MCF_CPP_NORETURN_IF_NDEBUG
#endif

template<typename... Params_t>
__MCF_CPP_NORETURN_IF_NDEBUG inline void Bail(const wchar_t *pwszFormat, const Params_t &... vParams){
	::MCF_CRT_BailF(pwszFormat, vParams...);
}

template<>
__MCF_CPP_NORETURN_IF_NDEBUG inline void Bail<>(const wchar_t *pwszDescription){
	::MCF_CRT_Bail(pwszDescription);
}

//----------------------------------------------------------------------------
// Clone
//----------------------------------------------------------------------------
template<typename T>
inline auto Clone(T &&vSrc)
	-> typename std::remove_cv<typename std::remove_reference<T>::type>::type
{
	return std::forward<T>(vSrc);
}

//----------------------------------------------------------------------------
// 二进制操作
//----------------------------------------------------------------------------
template<typename Tx, typename Ty>
inline void BCopy(Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "MCF::BCopy(): Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "MCF::BCopy(): Source and destination sizes do not match.");

	__builtin_memcpy(&vDst, &vSrc, sizeof(vDst));
}

template<typename T>
inline void BFill(T &vDst, bool bVal) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::BFill(): Only trivial types are supported.");

	__builtin_memset(&vDst, bVal ? -1 : 0, sizeof(vDst));
}

template<typename T>
inline void BZero(T &vDst) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::BZero(): Only trivial types are supported.");

	__builtin_memset(&vDst, 0, sizeof(vDst));
}

template<typename T>
inline bool BTest(const T &vSrc) noexcept {
	static_assert(std::is_trivial<T>::value, "MCF::BTest(): Only trivial types are supported.");

	return ::_memcchr(&vSrc, 0, sizeof(vSrc)) == nullptr;
}

template<typename Tx, typename Ty>
inline int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "MCF::BComp(): Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "MCF::BComp(): Source and destination sizes do not match.");

	return __builtin_memcmp(&vDst, &vSrc, sizeof(vDst));
}

//----------------------------------------------------------------------------
// 泛型工具
//----------------------------------------------------------------------------
template<typename Function_t>
void CallOnEach(Function_t && /* fnCallable */){
	// 空的。
}
template<typename Function_t, typename FirstParam_t, typename... Params_t>
void CallOnEach(Function_t &&fnCallable, FirstParam_t &&vFirstParam, Params_t &&... vParams){
	fnCallable(std::forward<FirstParam_t>(vFirstParam));
	CallOnEach(std::forward<Function_t>(fnCallable), std::forward<Params_t>(vParams)...);
}

template<typename Function_t>
void CallOnEachBackward(Function_t && /* fnCallable */){
	// 空的。
}
template<typename Function_t, typename FirstParam_t, typename... Params_t>
void CallOnEachBackward(Function_t &&fnCallable, FirstParam_t &&vFirstParam, Params_t &&... vParams){
	CallOnEachBackward(std::forward<Function_t>(fnCallable), std::forward<Params_t>(vParams)...);
	fnCallable(std::forward<FirstParam_t>(vFirstParam));
}

//----------------------------------------------------------------------------
// Min / Max
//----------------------------------------------------------------------------
template<typename Tx, typename Ty>
typename std::common_type<Tx, Ty>::type Min(Tx op1, Ty op2){
	static_assert(std::is_scalar<Tx>::value && std::is_scalar<Ty>::value, "MCF::Min(): Only scalar types are supported");
	static_assert(std::is_signed<Tx>::value ^ !std::is_signed<Ty>::value, "MCF::Min(): Comparison between signed and unsigned integers.");

	return std::min<typename std::common_type<Tx, Ty>::type>(op1, op2);
}

template<typename Tx, typename Ty>
typename std::common_type<Tx, Ty>::type Max(Tx op1, Ty op2){
	static_assert(std::is_scalar<Tx>::value && std::is_scalar<Ty>::value, "MCF::Min(): Only scalar types are supported");
	static_assert(std::is_signed<Tx>::value ^ !std::is_signed<Ty>::value, "MCF::Min(): Comparison between signed and unsigned integers.");

	return std::max<typename std::common_type<Tx, Ty>::type>(op1, op2);
}

}

#endif
