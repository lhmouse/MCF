// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_HPP_
#define MCF_UTILITIES_HPP_

#include "../../MCFCRT/ext/assert.h"
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
// BITS_OF
//----------------------------------------------------------------------------
#define BITS_OF(x)		(sizeof(x) * __CHAR_BIT__)

//----------------------------------------------------------------------------
// MACRO_TYPE
//----------------------------------------------------------------------------
namespace Impl {
	template<typename>
	struct MacroTypeHelper;

	template<typename Ty>
	struct MacroTypeHelper {
		typedef Ty Type;
	};
}

// 安全地在宏参数中使用带逗号的类型名，例如 MY_MACRO(double, (std::map<int, double>))。
#define MACRO_TYPE(type)	typename ::MCF::Impl::MacroTypeHelper<type>::Type

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

		template<typename... Params_t>
		ConcreteBase(Params_t &&... vParams)
			noexcept(std::is_nothrow_constructible<Base_t, Params_t &&...>::value)
			: Base_t(std::forward<Params_t>(vParams)...)
		{
		}

		virtual void MCF_Impl_PureAbstractFunction_() override {
		}
	};
}

#define ABSTRACT				private ::MCF::Impl::AbstractBase
#define CONCRETE(type)			public ::MCF::Impl::ConcreteBase<MACRO_TYPE(type)>
#define CONCRETE_BASE(type)		::MCF::Impl::ConcreteBase<MACRO_TYPE(type)>

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
	struct DirectConstructTag {
	};
}

}

// 我只能说 GCC 是个白痴！为什么要检查 placement new 的返回值是否为 nullptr？
inline __attribute__((__returns_nonnull__)) void *operator new(std::size_t, void *p, const ::MCF::Impl::DirectConstructTag &){
	return p;
}
inline void operator delete(void *, void *, const ::MCF::Impl::DirectConstructTag &) noexcept {
}

namespace MCF {

namespace Impl {
	template<typename Object_t>
	struct DirectConstructor {
		template<typename... Params_t>
		Object_t *Construct(void *pObject, Params_t &&... vParams)
			noexcept(std::is_nothrow_constructible<Object_t, Params_t &&...>::value)
		{
			return ::new(pObject, DirectConstructTag()) Object_t(std::forward<Params_t>(vParams)...);
		}
		void Destruct(Object_t *pObject)
			noexcept(std::is_nothrow_destructible<Object_t>::value)
		{
			pObject->~Object_t();
		}
	};
}

#define FRIEND_CONSTRUCT_DESTRUCT(type)	\
	friend class ::MCF::Impl::DirectConstructor<MACRO_TYPE(type)>

template<typename Object_t, typename... Params_t>
inline __attribute__((__returns_nonnull__)) Object_t *Construct(void *pObject, Params_t &&... vParams)
	noexcept(std::is_nothrow_constructible<Object_t, Params_t &&...>::value)
{
	return Impl::DirectConstructor<Object_t>().template Construct<Params_t &&...>(pObject, std::forward<Params_t>(vParams)...);
}
template<typename Object_t>
inline void Destruct(Object_t *pObject)
	noexcept(std::is_nothrow_destructible<Object_t>::value)
{
	ASSERT(pObject);

	Impl::DirectConstructor<Object_t>().Destruct(pObject);
}

//----------------------------------------------------------------------------
// CountLeadingZeroes
//----------------------------------------------------------------------------
inline unsigned char CountLeadingZeroes(unsigned long long ull) noexcept {
	return (unsigned char)__builtin_clzll(ull);
}
inline unsigned char CountLeadingZeroes(unsigned long ul) noexcept {
	return (unsigned char)__builtin_clzl(ul);
}
inline unsigned char CountLeadingZeroes(unsigned int u) noexcept {
	return (unsigned char)__builtin_clz(u);
}
inline unsigned char CountLeadingZeroes(unsigned short ush) noexcept {
	return (unsigned char)__builtin_clz(ush)
		- (sizeof(unsigned int) - sizeof(unsigned short)) * (std::size_t)__CHAR_BIT__;
}
inline unsigned char CountLeadingZeroes(unsigned char by) noexcept {
	return (unsigned char)__builtin_clz(by)
		- (sizeof(unsigned int) - sizeof(unsigned char)) * (std::size_t)__CHAR_BIT__;
}

//----------------------------------------------------------------------------
// CountTrailingZeroes
//----------------------------------------------------------------------------
inline unsigned char CountTrailingZeroes(unsigned long long ull) noexcept {
	return (unsigned char)__builtin_ctzll(ull);
}
inline unsigned char CountTrailingZeroes(unsigned long ul) noexcept {
	return (unsigned char)__builtin_ctzl(ul);
}
inline unsigned char CountTrailingZeroes(unsigned int u) noexcept {
	return (unsigned char)__builtin_ctz(u);
}
inline unsigned char CountTrailingZeroes(unsigned short ush) noexcept {
	return (unsigned char)__builtin_ctz(ush);
}
inline unsigned char CountTrailingZeroes(unsigned char by) noexcept {
	return (unsigned char)__builtin_ctz(by);
}

//----------------------------------------------------------------------------
// ByteSwap
//----------------------------------------------------------------------------
inline std::uint64_t ByteSwap(std::uint64_t u64) noexcept {
	return __builtin_bswap64(u64);
}
inline std::uint32_t ByteSwap(std::uint32_t u32) noexcept {
	return __builtin_bswap32(u32);
}
inline std::uint16_t ByteSwap(std::uint16_t u16) noexcept {
	return __builtin_bswap16(u16);
}
inline std::uint8_t ByteSwap(std::uint8_t u8) noexcept {
	return u8;
}

//----------------------------------------------------------------------------
// Copy / CopyN / CopyBackward / CopyBackwardN
//----------------------------------------------------------------------------
template<typename OutputBegin_t, typename InputBegin_t, typename InputEnd_t>
inline OutputBegin_t Copy(OutputBegin_t itOutputBegin, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin)>::value)
{
	while(itInputBegin != itInputEnd){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename OutputBegin_t, typename InputBegin_t>
inline OutputBegin_t CopyN(OutputBegin_t itOutputBegin, InputBegin_t itInputBegin, std::size_t uCount)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin)>::value)
{
	for(auto i = uCount; i; --i){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename OutputEnd_t, typename InputBegin_t, typename InputEnd_t>
inline OutputEnd_t CopyBackward(OutputEnd_t itOutputEnd, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd)>::value)
{
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return std::move(itOutputEnd);
}
template<typename OutputEnd_t, typename InputEnd_t>
inline OutputEnd_t CopyBackwardN(OutputEnd_t itOutputEnd, std::size_t uCount, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd)>::value)
{
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return std::move(itOutputEnd);
}

//----------------------------------------------------------------------------
// ReverseCopy / ReverseCopyN / ReverseCopyBackward / ReverseCopyBackwardN
//----------------------------------------------------------------------------
template<typename OutputBegin_t, typename InputBegin_t, typename InputEnd_t>
inline OutputBegin_t ReverseCopy(OutputBegin_t itOutputBegin, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return Copy(
		std::move(itOutputBegin),
		std::reverse_iterator<InputEnd_t>(itInputEnd),
		std::reverse_iterator<InputBegin_t>(itInputBegin)
	);
}
template<typename OutputBegin_t, typename InputEnd_t>
inline OutputBegin_t ReverseCopyN(OutputBegin_t itOutputBegin, std::size_t uCount, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return CopyN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputEnd_t>(itInputEnd),
		uCount
	);
}
template<typename OutputEnd_t, typename InputBegin_t, typename InputEnd_t>
inline OutputEnd_t ReverseCopyBackward(OutputEnd_t itOutputEnd, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return CopyBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputEnd_t>(itInputEnd),
		std::reverse_iterator<InputBegin_t>(itInputBegin)
	);
}
template<typename OutputEnd_t, typename InputBegin_t>
inline OutputEnd_t ReverseCopyBackwardN(OutputEnd_t itOutputEnd, InputBegin_t itInputBegin, std::size_t uCount)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputBegin_t>(itInputBegin)
	);
}

//----------------------------------------------------------------------------
// Move / MoveN / MoveBackward / MoveBackwardN
//----------------------------------------------------------------------------
template<typename OutputBegin_t, typename InputBegin_t, typename InputEnd_t>
inline OutputBegin_t Move(OutputBegin_t itOutputBegin, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin) &&>::value)
{
	return Copy(
		std::move(itOutputBegin),
		std::move_iterator<InputBegin_t>(itInputBegin),
		std::move_iterator<InputEnd_t>(itInputEnd)
	);
}
template<typename OutputBegin_t, typename InputBegin_t>
inline OutputBegin_t MoveN(OutputBegin_t itOutputBegin, InputBegin_t itInputBegin, std::size_t uCount)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin) &&>::value)
{
	return CopyN(
		std::move(itOutputBegin),
		std::move_iterator<InputBegin_t>(itInputBegin),
		uCount
	);
}
template<typename OutputEnd_t, typename InputBegin_t, typename InputEnd_t>
inline OutputEnd_t MoveBackward(OutputEnd_t itOutputEnd, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd) &&>::value)
{
	return CopyBackward(
		std::move(itOutputEnd),
		std::move_iterator<InputBegin_t>(itInputBegin),
		std::move_iterator<InputEnd_t>(itInputEnd)
	);
}
template<typename OutputEnd_t, typename InputEnd_t>
inline OutputEnd_t MoveBackwardN(OutputEnd_t itOutputEnd, std::size_t uCount, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd) &&>::value)
{
	return CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::move_iterator<InputEnd_t>(itInputEnd)
	);
}

//----------------------------------------------------------------------------
// ReverseMove / ReverseMoveN / ReverseMoveBackward / ReverseMoveBackwardN
//----------------------------------------------------------------------------
template<typename OutputBegin_t, typename InputBegin_t, typename InputEnd_t>
inline OutputBegin_t ReverseMove(OutputBegin_t itOutputBegin, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return Move(
		std::move(itOutputBegin),
		std::reverse_iterator<InputEnd_t>(itInputEnd),
		std::reverse_iterator<InputBegin_t>(itInputBegin)
	);
}
template<typename OutputBegin_t, typename InputEnd_t>
inline OutputBegin_t ReverseMoveN(OutputBegin_t itOutputBegin, std::size_t uCount, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return MoveN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputEnd_t>(itInputEnd),
		uCount
	);
}
template<typename OutputEnd_t, typename InputBegin_t, typename InputEnd_t>
inline OutputEnd_t ReverseMoveBackward(OutputEnd_t itOutputEnd, InputBegin_t itInputBegin, InputEnd_t itInputEnd)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return MoveBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputEnd_t>(itInputEnd),
		std::reverse_iterator<InputBegin_t>(itInputBegin)
	);
}
template<typename OutputEnd_t, typename InputBegin_t>
inline OutputEnd_t ReverseMoveBackwardN(OutputEnd_t itOutputEnd, InputBegin_t itInputBegin, std::size_t uCount)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return MoveBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputBegin_t>(itInputBegin)
	);
}

//----------------------------------------------------------------------------
// Fill / FillN
//----------------------------------------------------------------------------
template<typename OutputBegin_t, typename OutputEnd_t, typename... Params_t>
inline OutputBegin_t Fill(OutputBegin_t itOutputBegin, OutputEnd_t itOutputEnd, const Params_t &... vParams)
	noexcept(
		std::is_nothrow_constructible<decltype(*itOutputBegin), const Params_t &...>::value
		&& std::is_nothrow_move_assignable<decltype(*itOutputBegin)>::value
	)
{
	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = typename std::remove_reference<decltype(*itOutputBegin)>::type(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename OutputBegin_t, typename... Params_t>
inline OutputBegin_t FillN(OutputBegin_t itOutputBegin, std::size_t uCount, const Params_t &... vParams)
	noexcept(
		std::is_nothrow_constructible<decltype(*itOutputBegin), const Params_t &...>::value
		&& std::is_nothrow_move_assignable<decltype(*itOutputBegin)>::value
	)
{
	for(auto i = uCount; i; --i){
		*itOutputBegin = typename std::remove_reference<decltype(*itOutputBegin)>::type(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}

}

#endif
