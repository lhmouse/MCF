// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_HPP_
#define MCF_UTILITIES_HPP_

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
namespace Impl {
	template<typename Ty, std::size_t N>
	char (&CountOfHelper(Ty (&)[N]))[N];

	template<typename Ty, std::size_t N>
	char (&CountOfHelper(Ty (&&)[N]))[N];
}

#define COUNT_OF(a)		(sizeof(::MCF::Impl::CountOfHelper((a))))

//----------------------------------------------------------------------------
// NO_COPY
//----------------------------------------------------------------------------
namespace Impl {
	struct NonCopyableBase {
		constexpr NonCopyableBase() noexcept = default;
		~NonCopyableBase() = default;

		NonCopyableBase(const NonCopyableBase &) = delete;
		void operator=(const NonCopyableBase &) = delete;
		NonCopyableBase(NonCopyableBase &&) = delete;
		void operator=(NonCopyableBase &&) = delete;
	};
}

#define NO_COPY			private ::MCF::Impl::NonCopyableBase

//----------------------------------------------------------------------------
// ABSTRACT / CONCRETE
//----------------------------------------------------------------------------
namespace Impl {
	struct AbstractBase {
		virtual ~AbstractBase() noexcept {
		}

		virtual void MCF_Impl_PureAbstractFunction_() = 0;
	};

	template<class Base_t>
	struct ConcreteBase : public Base_t {
		static_assert(std::is_base_of<AbstractBase, Base_t>::value, "Concreting from non-abstract class?");

		virtual void MCF_Impl_PureAbstractFunction_() override {
		}
	};
}

#define ABSTRACT		private ::MCF::Impl::AbstractBase
#define CONCRETE(...)	public ::MCF::Impl::ConcreteBase<__VA_ARGS__>

//----------------------------------------------------------------------------
// Bail
//----------------------------------------------------------------------------
template<typename... Params_t>
#ifdef NDEBUG
[[noreturn]]
#endif
inline void Bail(const wchar_t *pwszFormat, const Params_t &... vParams){
	::MCF_CRT_BailF(pwszFormat, vParams...);
}

template<>
#ifdef NDEBUG
[[noreturn]]
#endif
inline void Bail<>(const wchar_t *pwszDescription){
	::MCF_CRT_Bail(pwszDescription);
}

//----------------------------------------------------------------------------
// ASSERT_NOEXCEPT
//----------------------------------------------------------------------------
namespace Impl {
	__attribute__((error("noexcept assertion failed"), __noreturn__)) void AssertNoexcept() noexcept;
}

#define ASSERT_NOEXCEPT_BEGIN	\
	try {

#define ASSERT_NOEXCEPT_END	\
	} catch(...){	\
		::MCF::Impl::AssertNoexcept();	\
	}

#define ASSERT_NOEXCEPT_END_COND(cond)	\
	} catch(...){	\
		if(cond){	\
			::MCF::Impl::AssertNoexcept();	\
		}	\
		throw;	\
	}

#define FORCE_NOEXCEPT_BEGIN	\
	[&]() noexcept {

#define FORCE_NOEXCEPT_END	\
	}();

#define FORCE_NOEXCEPT(expr)	\
	do { FORCE_NOEXCEPT_BEGIN { expr; } FORCE_NOEXCEPT_END } while(false)

//----------------------------------------------------------------------------
// Clone
//----------------------------------------------------------------------------
template<typename T>
inline auto Clone(T &&vSrc)
	-> typename std::remove_cv<
		typename std::remove_reference<T>::type
	>::type
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

template<typename Tx, typename Ty>
inline int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "MCF::BComp(): Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "MCF::BComp(): Source and destination sizes do not match.");

	return __builtin_memcmp(&vDst, &vSrc, sizeof(vSrc));
}

//----------------------------------------------------------------------------
// CallOnEach
//----------------------------------------------------------------------------
template<typename Function_t>
Function_t &&CallOnEach(Function_t &&fnCallable){
	return std::forward<Function_t>(fnCallable);
}
template<typename Function_t, typename FirstParam_t, typename... Params_t>
Function_t &&CallOnEach(Function_t &&fnCallable, FirstParam_t &&vFirstParam, Params_t &&... vParams){
	fnCallable(std::forward<FirstParam_t>(vFirstParam));
	return CallOnEach(std::forward<Function_t>(fnCallable), std::forward<Params_t>(vParams)...);
}

template<typename Function_t>
Function_t &&CallOnEachBackward(Function_t &&fnCallable){
	return std::forward<Function_t>(fnCallable);
}
template<typename Function_t, typename FirstParam_t, typename... Params_t>
Function_t &&CallOnEachBackward(Function_t &&fnCallable, FirstParam_t &&vFirstParam, Params_t &&... vParams){
	auto &&fnNewCallable = CallOnEachBackward(std::forward<Function_t>(fnCallable), std::forward<Params_t>(vParams)...);
	fnNewCallable(std::forward<FirstParam_t>(vFirstParam));
	return std::forward<Function_t>(fnNewCallable);
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

//----------------------------------------------------------------------------
// Construct / Destruct
//----------------------------------------------------------------------------
namespace Impl {
	struct DirectConstruct {
	};
}

}

// 我只能说 GCC 是个白痴！为什么要检查 placement new 的返回值是否为 nullptr？
inline void *__attribute__((__returns_nonnull__)) operator new(std::size_t, void *p, const ::MCF::Impl::DirectConstruct &){
	return p;
}
inline void operator delete(void *, void *, const ::MCF::Impl::DirectConstruct &) noexcept {
}

namespace MCF {

template<typename Obj_t, typename... Params_t>
inline Obj_t *__attribute__((__returns_nonnull__)) Construct(void *pObj, Params_t &&... vParams)
	noexcept(std::is_nothrow_constructible<Obj_t, Params_t &&...>::value)
{
	return new(pObj, Impl::DirectConstruct()) Obj_t(std::forward<Params_t>(vParams)...);
}
template<typename Obj_t>
inline void Destruct(Obj_t *pObj)
	noexcept(std::is_nothrow_destructible<Obj_t>::value)
{
	pObj->~Obj_t();
}

}

#endif
