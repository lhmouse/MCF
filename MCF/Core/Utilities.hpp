// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_HPP_
#define MCF_UTILITIES_HPP_

#include "../../MCFCRT/stdc/ext/memcchr.h"
#include "../../MCFCRT/env/bail.h"
#include <type_traits>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <cstdint>

namespace MCF {

//----------------------------------------------------------------------------
// COUNT_OF
//----------------------------------------------------------------------------
template<typename Ty, std::size_t N>
char (&CountOfHelper_(Ty (&)[N]))[N];

template<typename Ty, std::size_t N>
char (&CountOfHelper_(Ty (&&)[N]))[N];

#define COUNT_OF(a)		(sizeof(::MCF::CountOfHelper_((a))))

//----------------------------------------------------------------------------
// NO_COPY
//----------------------------------------------------------------------------
struct NonCopyable_ {
	constexpr NonCopyable_() noexcept = default;
	~NonCopyable_() = default;

	NonCopyable_(const NonCopyable_ &) = delete;
	void operator=(const NonCopyable_ &) = delete;
	NonCopyable_(NonCopyable_ &&) = delete;
	void operator=(NonCopyable_ &&) = delete;
};

#define NO_COPY			private ::MCF::NonCopyable_

//----------------------------------------------------------------------------
// ABSTRACT / CONCRETE
//----------------------------------------------------------------------------
struct Abstract_ {
	virtual ~Abstract_() noexcept { }
	virtual void MCF_PureAbstractFunction_() = 0;
};

template<class Base_t>
struct Concrete_ : public Base_t {
	static_assert(std::is_base_of<Abstract_, Base_t>::value, "Concreting from non-abstract class?");

	virtual void MCF_PureAbstractFunction_(){ }
};

#define ABSTRACT		private ::MCF::Abstract_
#define CONCRETE(base)	public ::MCF::Concrete_<base>

//----------------------------------------------------------------------------
// Bail
//----------------------------------------------------------------------------
#ifdef NDEBUG
#	define MCF_CPP_NORETURN_IF_NDEBUG	[[noreturn]]
#else
#	define MCF_CPP_NORETURN_IF_NDEBUG
#endif

template<typename... Params_t>
MCF_CPP_NORETURN_IF_NDEBUG inline void Bail(const wchar_t *pwszFormat, const Params_t &... vParams){
	::MCF_CRT_BailF(pwszFormat, vParams...);
}

template<>
MCF_CPP_NORETURN_IF_NDEBUG inline void Bail<>(const wchar_t *pwszDescription){
	::MCF_CRT_Bail(pwszDescription);
}

//----------------------------------------------------------------------------
// ASSERT_NOEXCEPT
//----------------------------------------------------------------------------
template<bool NOEXCEPT>
inline void MCF_NoExceptAssertionFailed() noexcept;

template<>
inline void MCF_NoExceptAssertionFailed<false>() noexcept {
}

#ifdef NDEBUG
template<>
void MCF_NoExceptAssertionFailed<true>() noexcept; // 未定义的。
#else
template<>
inline void __attribute__((__always_inline__)) MCF_NoExceptAssertionFailed<true>() noexcept {
	Bail(L"ASSERT_NOEXCEPT: 假定不会抛出异常，但是确实捕获到了异常。");
}
#endif

#define ASSERT_NOEXCEPT_BEGIN		try {
#define ASSERT_NOEXCEPT_END			} catch(...){ MCF_NoExceptAssertionFailed<true>(); }
#define ASSERT_NOEXCEPT_END_COND(c)	} catch(...){ MCF_NoExceptAssertionFailed<(c)>(); }

//----------------------------------------------------------------------------
// HintNonNull
//----------------------------------------------------------------------------
template<typename T>
inline T *__attribute__((__returns_nonnull__)) HintNonNull(T *p) noexcept {
	ASSERT(p);

	return p;
}

//----------------------------------------------------------------------------
// NullRef
//----------------------------------------------------------------------------
template<typename T>
auto NullRef() noexcept
	-> typename std::enable_if<!std::is_rvalue_reference<T>::value, T &>::type;

template<typename T>
auto NullRef() noexcept
	-> typename std::enable_if<std::is_rvalue_reference<T>::value, T &&>::type;

//----------------------------------------------------------------------------
// Clone
//----------------------------------------------------------------------------
template<typename T>
inline auto Clone(T &&vSrc)
	-> typename std::remove_cv<typename std::remove_reference<T>::type>::type
{
	return typename std::remove_cv<
		typename std::remove_reference<T>::type
	>::type(std::forward<T>(vSrc));
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
