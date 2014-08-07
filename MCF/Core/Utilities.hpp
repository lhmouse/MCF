// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_HPP_
#define MCF_UTILITIES_HPP_

#include "../../MCFCRT/ext/assert.h"
#include "../../MCFCRT/ext/_make_constant.h"
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

#define NO_COPY				private ::MCF::Impl::NonCopyableBase

//----------------------------------------------------------------------------
// ABSTRACT / CONCRETE
//----------------------------------------------------------------------------
namespace Impl {
	class AbstractBase {
	protected:
		virtual ~AbstractBase() noexcept = default;

	private:
		virtual void MCF_PureAbstract_() noexcept = 0;
	};

	template<typename RealBase>
	class ConcreteBase : public RealBase {
	protected:
		template<typename... BaseParams>
		explicit ConcreteBase(BaseParams &&... vBaseParams)
			noexcept(std::is_nothrow_constructible<RealBase, BaseParams &&...>::value)
			: RealBase(std::forward<BaseParams>(vBaseParams)...)
		{
		}

	private:
		void MCF_PureAbstract_() noexcept override {
		}
	};
}

#define ABSTRACT			private ::MCF::Impl::AbstractBase
#define CONCRETE(base)		public ::MCF::Impl::ConcreteBase<base>

#define CONCRETE_INIT(base, ...)	\
	::MCF::Impl::ConcreteBase<base>(__VA_ARGS__)

//----------------------------------------------------------------------------
// Bail
//----------------------------------------------------------------------------
template<typename... Params>
__MCF_NORETURN_IF_NDEBUG inline
void Bail(const wchar_t *pwszFormat, const Params &... vParams){
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
	{	\
		return (expr);	\
	}	\
	FORCE_NOEXCEPT_END	\

//----------------------------------------------------------------------------
// Clone
//----------------------------------------------------------------------------
template<typename T>
inline auto Clone(T &&vSrc)
	noexcept(std::is_nothrow_constructible<
		typename std::remove_reference<T>::type, T &&
		>::value)
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
// CallOnEach / ReverseCallOnEach
//----------------------------------------------------------------------------
template<typename Function, typename First, typename ...Params>
Function &&CallOnEach(Function &&vFunction, First &&vFirst, Params &&... vParams){
	vFunction(std::forward<First>(vFirst));
	CallOnEach(vFunction, std::forward<Params>(vParams)...);
	return std::forward<Function>(vFunction);
}
template<typename Function>
Function &&CallOnEach(Function &&vFunction){
	return std::forward<Function>(vFunction);
}

template<typename Function, typename First, typename ...Params>
Function &&ReverseCallOnEach(Function &&vFunction, First &&vFirst, Params &&... vParams){
	ReverseCallOnEach(vFunction, std::forward<Params>(vParams)...);
	vFunction(std::forward<First>(vFirst));
	return std::forward<Function>(vFunction);
}
template<typename Function>
Function &&ReverseCallOnEach(Function &&vFunction){
	return std::forward<Function>(vFunction);
}

//----------------------------------------------------------------------------
// Min / Max
//----------------------------------------------------------------------------
template<typename Tx, typename Ty, typename Comparator = std::less<void>>
auto constexpr Min(Tx x, Ty y){
	static_assert(
		std::is_scalar<Tx>::value && std::is_scalar<Ty>::value,
		"Only scalar types are supported."
	);
	static_assert(
		std::is_signed<Tx>::value == std::is_signed<Ty>::value,
		"Comparison between signed and unsigned integers."
	);

	return Comparator()(x, y) ? x : y;
}
template<typename Tx, typename Ty, typename Comparator = std::less<void>, typename... More>
auto constexpr Min(Tx &&x, Ty &&y, More &&... vMore){
	return Min(Min(std::forward<Tx>(x), std::forward<Ty>(y)), std::forward<More>(vMore)...);
}

template<typename Tx, typename Ty, typename Comparator = std::less<void>>
auto constexpr Max(Tx x, Ty y){
	static_assert(
		std::is_scalar<Tx>::value && std::is_scalar<Ty>::value,
		"Only scalar types are supported."
	);
	static_assert(
		std::is_signed<Tx>::value == std::is_signed<Ty>::value,
		"Comparison between signed and unsigned integers."
	);

	return Comparator()(x, y) ? y : x;
}
template<typename Tx, typename Ty, typename Comparator = std::less<void>, typename... More>
auto constexpr Max(Tx &&x, Ty &&y, More &&... vMore){
	return Max(Max(std::forward<Tx>(x), std::forward<Ty>(y)), std::forward<More>(vMore)...);
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
inline __attribute__((__returns_nonnull__))
void *operator new(std::size_t, void *p, const ::MCF::Impl::DirectConstructTag &){
	return p;
}
inline void operator delete(void *, void *, const ::MCF::Impl::DirectConstructTag &) noexcept {
}

namespace MCF {

namespace Impl {
	template<typename Object>
	struct DirectConstructor {
		template<typename... Params>
		static Object *Construct(Object *pObject, Params &&... vParams)
			noexcept(std::is_nothrow_constructible<Object, Params &&...>::value)
		{
			return ::new(pObject, DirectConstructTag()) Object(std::forward<Params>(vParams)...);
		}
		static void Destruct(Object *pObject)
			noexcept(std::is_nothrow_destructible<Object>::value)
		{
			pObject->~Object();
		}
	};
}

#define FRIEND_CONSTRUCT_DESTRUCT(type)	\
	friend class ::MCF::Impl::DirectConstructor<type>

template<typename Object, typename... Params>
inline __attribute__((__returns_nonnull__)) Object *Construct(Object *pObject, Params &&... vParams)
	noexcept(std::is_nothrow_constructible<Object, Params &&...>::value)
{
	return Impl::DirectConstructor<Object>::template Construct<Params &&...>(
		pObject, std::forward<Params>(vParams)...
	);
}
template<typename Object>
inline void Destruct(Object *pObject)
	noexcept(std::is_nothrow_destructible<Object>::value)
{
	ASSERT(pObject);

	Impl::DirectConstructor<Object>::Destruct(pObject);
}

//----------------------------------------------------------------------------
// CountLeadingZeroes
//----------------------------------------------------------------------------
inline constexpr unsigned char CountLeadingZeroes(unsigned char by) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clz(by)
		- (sizeof(unsigned int) - sizeof(unsigned char)) * (std::size_t)__CHAR_BIT__);
}
inline constexpr unsigned char CountLeadingZeroes(unsigned short ush) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clz(ush)
		- (sizeof(unsigned int) - sizeof(unsigned short)) * (std::size_t)__CHAR_BIT__);
}
inline constexpr unsigned char CountLeadingZeroes(unsigned int u) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clz(u));
}
inline constexpr unsigned char CountLeadingZeroes(unsigned long ul) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clzl(ul));
}
inline constexpr unsigned char CountLeadingZeroes(unsigned long long ull) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_clzll(ull));
}

//----------------------------------------------------------------------------
// CountTrailingZeroes
//----------------------------------------------------------------------------
inline constexpr unsigned char CountTrailingZeroes(unsigned char by) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctz(by));
}
inline constexpr unsigned char CountTrailingZeroes(unsigned short ush) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctz(ush));
}
inline constexpr unsigned char CountTrailingZeroes(unsigned int u) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctz(u));
}
inline constexpr unsigned char CountTrailingZeroes(unsigned long ul) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctzl(ul));
}
inline constexpr unsigned char CountTrailingZeroes(unsigned long long ull) noexcept {
	return __MCF_MAKE_CONSTANT((unsigned char)__builtin_ctzll(ull));
}

//----------------------------------------------------------------------------
// ByteSwap
//----------------------------------------------------------------------------
namespace Impl {
	template<std::size_t BIT_COUNT>
	struct ByteSwapHelper {
		static_assert((BIT_COUNT, false), "Not supported.");
	};

	template<>
	struct ByteSwapHelper<8> {
		static constexpr std::uint8_t Do(std::uint8_t uVal) noexcept {
			return uVal;
		}
	};
	template<>
	struct ByteSwapHelper<16> {
		static constexpr std::uint16_t Do(std::uint16_t uVal) noexcept {
			return __MCF_MAKE_CONSTANT(__builtin_bswap16(uVal));
		}
	};
	template<>
	struct ByteSwapHelper<32> {
		static constexpr std::uint32_t Do(std::uint32_t uVal) noexcept {
			return __MCF_MAKE_CONSTANT(__builtin_bswap32(uVal));
		}
	};
	template<>
	struct ByteSwapHelper<64> {
		static constexpr std::uint64_t Do(std::uint64_t uVal) noexcept {
			return __MCF_MAKE_CONSTANT(__builtin_bswap64(uVal));
		}
	};
}

inline constexpr unsigned char ByteSwap(unsigned char by) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned char)>::Do(by);
}
inline constexpr unsigned short ByteSwap(unsigned short ush) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned short)>::Do(ush);
}
inline constexpr unsigned int ByteSwap(unsigned int u) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned int)>::Do(u);
}
inline constexpr unsigned long ByteSwap(unsigned long ul) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned long)>::Do(ul);
}
inline constexpr unsigned long long ByteSwap(unsigned long long ull) noexcept {
	return Impl::ByteSwapHelper<BITS_OF(unsigned long long)>::Do(ull);
}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#	define BYTE_SWAP_IF_BE(expr)	(::MCF::ByteSwap(expr))
#	define BYTE_SWAP_IF_LE(expr)	(expr)
#else
#	define BYTE_SWAP_IF_BE(expr)	(expr)
#	define BYTE_SWAP_IF_LE(expr)	(::MCF::ByteSwap(expr))
#endif

//----------------------------------------------------------------------------
// Copy / CopyN / CopyBackward / CopyBackwardN
//----------------------------------------------------------------------------
template<typename OutputIterator, typename InputIterator>
inline OutputIterator Copy(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
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
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> CopyN(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
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
template<typename OutputIterator, typename InputIterator>
inline OutputIterator CopyBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
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
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> CopyBackwardN(
	OutputIterator itOutputEnd,
	std::size_t uCount,
	InputIterator itInputEnd
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
template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseCopy(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return Copy(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseCopyN(
	OutputIterator itOutputBegin,
	std::size_t uCount,
	InputIterator itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	auto vResult = CopyN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseCopyBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return CopyBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseCopyBackwardN(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	auto vResult = CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

//----------------------------------------------------------------------------
// Move / MoveN / MoveBackward / MoveBackwardN
//----------------------------------------------------------------------------
template<typename OutputIterator, typename InputIterator>
inline OutputIterator Move(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin) &&>::value)
{
	return Copy(
		std::move(itOutputBegin),
		std::move_iterator<InputIterator>(itInputBegin),
		std::move_iterator<InputIterator>(itInputEnd)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> MoveN(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputBegin) &&>::value)
{
	auto vResult = CopyN(
		std::move(itOutputBegin),
		std::move_iterator<InputIterator>(itInputBegin),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator MoveBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd) &&>::value)
{
	return CopyBackward(
		std::move(itOutputEnd),
		std::move_iterator<InputIterator>(itInputBegin),
		std::move_iterator<InputIterator>(itInputEnd)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> MoveBackwardN(
	OutputIterator itOutputEnd,
	std::size_t uCount,
	InputIterator itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputEnd) &&>::value)
{
	auto vResult = CopyBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::move_iterator<InputIterator>(itInputEnd)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

//----------------------------------------------------------------------------
// ReverseMove / ReverseMoveN / ReverseMoveBackward / ReverseMoveBackwardN
//----------------------------------------------------------------------------
template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseMove(
	OutputIterator itOutputBegin,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	return Move(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseMoveN(
	OutputIterator itOutputBegin,
	std::size_t uCount,
	InputIterator itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputBegin), decltype(*itInputEnd)>::value)
{
	auto vResult = MoveN(
		std::move(itOutputBegin),
		std::reverse_iterator<InputIterator>(itInputEnd),
		uCount
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}
template<typename OutputIterator, typename InputIterator>
inline OutputIterator ReverseMoveBackward(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	typename std::common_type<InputIterator>::type itInputEnd
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	return MoveBackward(
		std::move(itOutputEnd),
		std::reverse_iterator<InputIterator>(itInputEnd),
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
}
template<typename OutputIterator, typename InputIterator>
inline std::pair<OutputIterator, InputIterator> ReverseMoveBackwardN(
	OutputIterator itOutputEnd,
	InputIterator itInputBegin,
	std::size_t uCount
)
	noexcept(std::is_nothrow_assignable<decltype(*itOutputEnd), decltype(*itInputBegin)>::value)
{
	auto vResult = MoveBackwardN(
		std::move(itOutputEnd),
		uCount,
		std::reverse_iterator<InputIterator>(itInputBegin)
	);
	return std::make_pair(std::move(vResult.first), vResult.second.base());
}

//----------------------------------------------------------------------------
// Fill / FillN
//----------------------------------------------------------------------------
template<typename OutputIterator, typename... Params>
inline OutputIterator Fill(
	OutputIterator itOutputBegin,
	typename std::common_type<OutputIterator>::type itOutputEnd,
	const Params &... vParams
)
	noexcept(
		std::is_nothrow_constructible<decltype(*itOutputBegin), const Params &...>::value
		&& std::is_nothrow_move_assignable<decltype(*itOutputBegin)>::value
	)
{
	while(itOutputBegin != itOutputEnd){
		*itOutputBegin = typename std::remove_reference<decltype(*itOutputBegin)>::type(vParams...);
		++itOutputBegin;
	}
	return std::move(itOutputBegin);
}
template<typename OutputIterator, typename... Params>
inline OutputIterator FillN(
	OutputIterator itOutputBegin,
	std::size_t uCount,
	const Params &... vParams
)
	noexcept(
		std::is_nothrow_constructible<decltype(*itOutputBegin), const Params &...>::value
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
		template<typename Function, typename... TupleParams, typename... Unpacked>
		auto operator()(
			Function &&vFunction, const std::tuple<TupleParams...> &vTuple,
			const Unpacked &... vUnpacked
		) const {
			return CallOnTupleHelper<CUR + 1, END>()(
				vFunction, vTuple,
				vUnpacked..., std::get<CUR>(vTuple)
			);
		}
		template<typename Function, typename... TupleParams, typename... Unpacked>
		auto operator()(
			Function &&vFunction, std::tuple<TupleParams...> &&vTuple,
			Unpacked &&... vUnpacked
		) const {
			return CallOnTupleHelper<CUR + 1, END>()(
				vFunction, std::move(vTuple),
				std::move(vUnpacked)..., std::move(std::get<CUR>(vTuple))
			);
		}
	};
	template<std::size_t END>
	struct CallOnTupleHelper<END, END> {
		template<typename Function, typename... TupleParams, typename... Unpacked>
		auto operator()(
			Function &&vFunction, const std::tuple<TupleParams...> &,
			const Unpacked &... vUnpacked
		) const {
			return vFunction(vUnpacked...);
		}
		template<typename Function, typename... TupleParams, typename... Unpacked>
		auto operator()(
			Function &&vFunction, std::tuple<TupleParams...> &&,
			Unpacked &&... vUnpacked
		) const {
			return vFunction(std::move(vUnpacked)...);
		}
	};
}

template<typename Function, typename... Params>
auto CallOnTuple(Function vFunction, const std::tuple<Params...> &vTuple)
	noexcept(noexcept(
		std::declval<Function>()(std::declval<const Params &>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(vFunction, vTuple);
}
template<typename Function, typename... Params>
auto CallOnTuple(Function vFunction, std::tuple<Params...> &&vTuple)
	noexcept(noexcept(
		std::declval<Function>()(std::declval<Params &&>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(vFunction, std::move(vTuple));
}

template<class Object, typename... Params>
auto MakeFromTuple(const std::tuple<Params...> &vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<Object, const Params &...>::value
		&& std::is_nothrow_move_constructible<Object>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(
		[](const Params &... vParams){
			return Object(vParams...);
		},
		vTuple
	);
}
template<class Object, typename... Params>
auto MakeFromTuple(std::tuple<Params...> &&vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<Object, Params &&...>::value
		&& std::is_nothrow_move_constructible<Object>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(
		[](Params &&... vParams){
			return Object(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class Object, typename... Params>
auto MakeUniqueFromTuple(const std::tuple<Params...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(
		[](const Params &... vParams){
			return std::make_unique<Object>(vParams...);
		},
		vTuple
	);
}
template<class Object, typename... Params>
auto MakeUniqueFromTuple(std::tuple<Params...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(
		[](Params &&... vParams){
			return std::make_unique<Object>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class Object, typename... Params>
auto MakeSharedFromTuple(const std::tuple<Params...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(
		[](const Params &... vParams){
			return std::make_shared<Object>(vParams...);
		},
		vTuple
	);
}
template<class Object, typename... Params>
auto MakeSharedFromTuple(std::tuple<Params...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>()(
		[](Params &&... vParams){
			return std::make_shared<Object>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

}

#endif
