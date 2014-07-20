// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_HPP_
#define MCF_UTILITIES_HPP_

#include "../../MCFCRT/ext/assert.h"
#include "../../MCFCRT/env/bail.h"
#include <type_traits>
#include <algorithm>
#include <memory>
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

	template<typename R, typename P>
	struct MacroTypeHelper<R(P &)> {
		typedef P Type;
	};
	template<typename R, typename P>
	struct MacroTypeHelper<R(P &&)> {
		typedef P Type;
	};
}

// 安全地在宏参数中使用带逗号的类型名，例如 MY_MACRO(double, (std::map<int, double>))。
#define MACRO_TYPE(type_param)	typename ::MCF::Impl::MacroTypeHelper<void(type_param &&)>::Type

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
		virtual ~AbstractBase() = default;

		virtual void MCF_Impl_PureAbstractFunction_() const noexcept = 0;
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

		virtual void MCF_Impl_PureAbstractFunction_() const noexcept override {
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
__MCF_NORETURN_IF_NDEBUG inline void Bail(const wchar_t *pwszFormat, const Params_t &... vParams){
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
	__attribute__((error("noexcept assertion failed."), __noreturn__)) void AssertNoexcept() noexcept;
}

#define ASSERT_NOEXCEPT_BEGIN	\
	try {

#define ASSERT_NOEXCEPT_END	\
	} catch(...){	\
		::MCF::Impl::AssertNoexcept();	\
		__builtin_unreachable();	\
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
	FORCE_NOEXCEPT_BEGIN	\
		return (expr);	\
	FORCE_NOEXCEPT_END	\

//----------------------------------------------------------------------------
// Clone
//----------------------------------------------------------------------------
template<typename T>
inline auto Clone(T &&vSrc)
	noexcept(
		std::is_nothrow_copy_constructible<
			typename std::remove_reference<T>::type
		>::value
	)
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
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	__builtin_memcpy(&vDst, &vSrc, sizeof(vDst));
}

template<typename T>
inline void BFill(T &vDst, bool bVal) noexcept {
	static_assert(std::is_trivial<T>::value, "Only trivial types are supported.");

	__builtin_memset(&vDst, bVal ? -1 : 0, sizeof(vDst));
}

template<typename Tx, typename Ty>
inline int BComp(const Tx &vDst, const Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	return __builtin_memcmp(&vDst, &vSrc, sizeof(vSrc));
}

template<typename Tx, typename Ty>
inline void BSwap(Tx &vDst, Ty &vSrc) noexcept {
	static_assert(std::is_trivial<Tx>::value && std::is_trivial<Ty>::value, "Only trivial types are supported.");
	static_assert(sizeof(vDst) == sizeof(vSrc), "Source and destination sizes do not match.");

	std::swap(vDst, reinterpret_cast<Tx &>(vSrc));
}

template<typename T>
inline void BZero(T &vDst) noexcept {
	BFill(vDst, false);
}

//----------------------------------------------------------------------------
// CallOnEach / CallOnEachBackward
//----------------------------------------------------------------------------
template<typename Function_t>
Function_t CallOnEach(Function_t &&vFunction){
	return std::move(vFunction);
}
template<typename Function_t, typename FirstParam_t, typename... Params_t>
Function_t CallOnEach(Function_t &&vFunction, FirstParam_t &&vFirstParam, Params_t &&... vParams){
	vFunction(std::forward<FirstParam_t>(vFirstParam));
	return CallOnEach(std::move(vFunction), std::forward<Params_t>(vParams)...);
}

template<typename Function_t>
Function_t CallOnEachBackward(Function_t &&vFunction){
	return std::move(vFunction);
}
template<typename Function_t, typename FirstParam_t, typename... Params_t>
Function_t CallOnEachBackward(Function_t &&vFunction, FirstParam_t &&vFirstParam, Params_t &&... vParams){
	auto vNewFunction = CallOnEachBackward(std::move(vFunction), std::forward<Params_t>(vParams)...);
	vNewFunction(std::forward<FirstParam_t>(vFirstParam));
	return std::move(vNewFunction);
}

//----------------------------------------------------------------------------
// Min / Max
//----------------------------------------------------------------------------
template<typename Tx, typename Ty, typename Comparator_t = std::less<void>>
auto Min(Tx x, Ty y){
	static_assert(std::is_scalar<Tx>::value && std::is_scalar<Ty>::value, "Only scalar types are supported.");
	static_assert(std::is_signed<Tx>::value == std::is_signed<Ty>::value, "Comparison between signed and unsigned integers.");

	return Comparator_t()(x, y) ? x : y;
}
template<typename Tx, typename Ty, typename Comparator_t = std::less<void>, typename... More_t>
auto Min(Tx &&x, Ty &&y, More_t &&... vMore){
	return Min(Min(std::forward<Tx>(x), std::forward<Ty>(y)), std::forward<More_t>(vMore)...);
}

template<typename Tx, typename Ty, typename Comparator_t = std::less<void>>
auto Max(Tx x, Ty y){
	static_assert(std::is_scalar<Tx>::value && std::is_scalar<Ty>::value, "Only scalar types are supported.");
	static_assert(std::is_signed<Tx>::value == std::is_signed<Ty>::value, "Comparison between signed and unsigned integers.");

	return Comparator_t()(x, y) ? y : x;
}
template<typename Tx, typename Ty, typename Comparator_t = std::less<void>, typename... More_t>
auto Max(Tx &&x, Ty &&y, More_t &&... vMore){
	return Max(Max(std::forward<Tx>(x), std::forward<Ty>(y)), std::forward<More_t>(vMore)...);
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
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t Copy(
	OutputIterator_t itOutputBegin,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin)>::value)
{
	while(itInputBegin != itInputEnd){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> CopyN(
	OutputIterator_t itOutputBegin,
	InputIterator_t itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin)>::value)
{
	for(auto i = uCount; i; --i){
		*itOutputBegin = *itInputBegin;
		++itOutputBegin;
		++itInputBegin;
	}
	return std::make_pair(std::move(itOutputBegin), std::move(itInputBegin));
}
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t CopyBackward(
	OutputIterator_t itOutputEnd,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd)>::value)
{
	while(itInputBegin != itInputEnd){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return std::move(itOutputEnd);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> CopyBackwardN(
	OutputIterator_t itOutputEnd,
	std::size_t uCount,
	InputIterator_t itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd)>::value)
{
	for(auto i = uCount; i; --i){
		--itOutputEnd;
		--itInputEnd;
		*itOutputEnd = *itInputEnd;
	}
	return std::make_pair(std::move(itOutputEnd), std::move(itInputEnd));
}

//----------------------------------------------------------------------------
// ReverseCopy / ReverseCopyN / ReverseCopyBackward / ReverseCopyBackwardN
//----------------------------------------------------------------------------
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t ReverseCopy(
	OutputIterator_t itOutputBegin,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return Copy(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator_t>(itInputEnd),
		std::reverse_iterator<InputIterator_t>(itInputBegin)
	);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> ReverseCopyN(
	OutputIterator_t itOutputBegin,
	std::size_t uCount,
	InputIterator_t itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	auto vResult = CopyN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator_t>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t ReverseCopyBackward(
	OutputIterator_t itOutputEnd,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return CopyBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIterator_t>(itInputEnd),
		std::reverse_iterator<InputIterator_t>(itInputBegin)
	);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> ReverseCopyBackwardN(
	OutputIterator_t itOutputEnd,
	InputIterator_t itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	auto vResult = CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIterator_t>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

//----------------------------------------------------------------------------
// Move / MoveN / MoveBackward / MoveBackwardN
//----------------------------------------------------------------------------
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t Move(
	OutputIterator_t itOutputBegin,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin) &&>::value)
{
	return Copy(
		std::move(itOutputBegin),
		std::move_iterator<InputIterator_t>(itInputBegin),
		std::move_iterator<InputIterator_t>(itInputEnd)
	);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> MoveN(
	OutputIterator_t itOutputBegin,
	InputIterator_t itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin) &&>::value)
{
	auto vResult = CopyN(
		std::move(itOutputBegin),
		std::move_iterator<InputIterator_t>(itInputBegin),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t MoveBackward(
	OutputIterator_t itOutputEnd,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd) &&>::value)
{
	return CopyBackward(
		std::move(itOutputEnd),
		std::move_iterator<InputIterator_t>(itInputBegin),
		std::move_iterator<InputIterator_t>(itInputEnd)
	);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> MoveBackwardN(
	OutputIterator_t itOutputEnd,
	std::size_t uCount,
	InputIterator_t itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd) &&>::value)
{
	auto vResult = CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::move_iterator<InputIterator_t>(itInputEnd)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

//----------------------------------------------------------------------------
// ReverseMove / ReverseMoveN / ReverseMoveBackward / ReverseMoveBackwardN
//----------------------------------------------------------------------------
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t ReverseMove(
	OutputIterator_t itOutputBegin,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return Move(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator_t>(itInputEnd),
		std::reverse_iterator<InputIterator_t>(itInputBegin)
	);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> ReverseMoveN(
	OutputIterator_t itOutputBegin,
	std::size_t uCount,
	InputIterator_t itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	auto vResult = MoveN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator_t>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator_t, typename InputIterator_t>
inline OutputIterator_t ReverseMoveBackward(
	OutputIterator_t itOutputEnd,
	InputIterator_t itInputBegin,
	typename std::common_type<InputIterator_t>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return MoveBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIterator_t>(itInputEnd),
		std::reverse_iterator<InputIterator_t>(itInputBegin)
	);
}
template<typename OutputIterator_t, typename InputIterator_t>
inline std::pair<OutputIterator_t, InputIterator_t> ReverseMoveBackwardN(
	OutputIterator_t itOutputEnd,
	InputIterator_t itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	auto vResult = MoveBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIterator_t>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

//----------------------------------------------------------------------------
// Fill / FillN
//----------------------------------------------------------------------------
template<typename OutputIterator_t, typename... Params_t>
inline OutputIterator_t Fill(
	OutputIterator_t itOutputBegin,
	typename std::common_type<OutputIterator_t>::type itOutputEnd,
	const Params_t &... vParams
)
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
template<typename OutputIterator_t, typename... Params_t>
inline OutputIterator_t FillN(
	OutputIterator_t itOutputBegin,
	std::size_t uCount,
	const Params_t &... vParams
)
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

//----------------------------------------------------------------------------
// CallOnTuple / MakeFromTuple / MakeUniqueFromTuple / MakeSharedFromTuple
//----------------------------------------------------------------------------
namespace Impl {
	template<std::size_t CUR, std::size_t END>
	struct CallOnTupleHelper {
		template<typename Function_t, typename... TupleParams_t, typename... Unpacked_t>
		auto operator()(Function_t &&vFunction, const std::tuple<TupleParams_t...> &vTuple, const Unpacked_t &... vUnpacked) const {
			return CallOnTupleHelper<CUR + 1, END>()(vFunction, vTuple, vUnpacked..., std::get<CUR>(vTuple));
		}
		template<typename Function_t, typename... TupleParams_t, typename... Unpacked_t>
		auto operator()(Function_t &&vFunction, std::tuple<TupleParams_t...> &&vTuple, Unpacked_t &&... vUnpacked) const {
			return CallOnTupleHelper<CUR + 1, END>()(vFunction, std::move(vTuple), std::move(vUnpacked)..., std::move(std::get<CUR>(vTuple)));
		}
	};
	template<std::size_t END>
	struct CallOnTupleHelper<END, END> {
		template<typename Function_t, typename... TupleParams_t, typename... Unpacked_t>
		auto operator()(Function_t &&vFunction, const std::tuple<TupleParams_t...> &, const Unpacked_t &... vUnpacked) const {
			return vFunction(vUnpacked...);
		}
		template<typename Function_t, typename... TupleParams_t, typename... Unpacked_t>
		auto operator()(Function_t &&vFunction, std::tuple<TupleParams_t...> &&, Unpacked_t &&... vUnpacked) const {
			return vFunction(std::move(vUnpacked)...);
		}
	};
}

template<typename Function_t, typename... Params_t>
auto CallOnTuple(Function_t vFunction, const std::tuple<Params_t...> &vTuple)
	noexcept(noexcept(
		std::declval<Function_t>()(std::declval<const Params_t &>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(vFunction, vTuple);
}
template<typename Function_t, typename... Params_t>
auto CallOnTuple(Function_t vFunction, std::tuple<Params_t...> &&vTuple)
	noexcept(noexcept(
		std::declval<Function_t>()(std::declval<Params_t &&>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(vFunction, std::move(vTuple));
}

template<class Object_t, typename... Params_t>
auto MakeFromTuple(const std::tuple<Params_t...> &vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<Object_t, const Params_t &...>::value
		&& std::is_nothrow_move_constructible<Object_t>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(
		[](const Params_t &... vParams){
			return Object_t(vParams...);
		},
		vTuple
	);
}
template<class Object_t, typename... Params_t>
auto MakeFromTuple(std::tuple<Params_t...> &&vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<Object_t, Params_t &&...>::value
		&& std::is_nothrow_move_constructible<Object_t>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(
		[](Params_t &&... vParams){
			return Object_t(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class Object_t, typename... Params_t>
auto MakeUniqueFromTuple(const std::tuple<Params_t...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(
		[](const Params_t &... vParams){
			return std::make_unique<Object_t>(vParams...);
		},
		vTuple
	);
}
template<class Object_t, typename... Params_t>
auto MakeUniqueFromTuple(std::tuple<Params_t...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(
		[](Params_t &&... vParams){
			return std::make_unique<Object_t>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class Object_t, typename... Params_t>
auto MakeSharedFromTuple(const std::tuple<Params_t...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(
		[](const Params_t &... vParams){
			return std::make_shared<Object_t>(vParams...);
		},
		vTuple
	);
}
template<class Object_t, typename... Params_t>
auto MakeSharedFromTuple(std::tuple<Params_t...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params_t)>()(
		[](Params_t &&... vParams){
			return std::make_shared<Object_t>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

}

#endif
