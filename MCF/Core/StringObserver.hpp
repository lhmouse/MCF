// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_OBSERVER_HPP_
#define MCF_CORE_STRING_OBSERVER_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <functional>
#include <cstddef>

namespace MCF {

enum class StringType {
	NARROW,
	WIDE,
	UTF8,
	UTF16,
	UTF32,
	CESU8,
	ANSI,
};

template<StringType>
struct StringEncodingTrait;

template<>
struct StringEncodingTrait<StringType::NARROW> {
	using Type = char;
};
template<>
struct StringEncodingTrait<StringType::WIDE> {
	using Type = wchar_t;
};

template<>
struct StringEncodingTrait<StringType::UTF8> {
	using Type = char;
};
template<>
struct StringEncodingTrait<StringType::UTF16> {
	using Type = char16_t;
};
template<>
struct StringEncodingTrait<StringType::UTF32> {
	using Type = char32_t;
};
template<>
struct StringEncodingTrait<StringType::CESU8> {
	using Type = char;
};
template<>
struct StringEncodingTrait<StringType::ANSI> {
	using Type = char;
};

namespace Impl {
	constexpr std::size_t NPOS = (std::size_t)-1;

	template<typename CharT>
	const CharT *StrEndOf(const CharT *pszBegin) noexcept {
		ASSERT(pszBegin);

		auto pchEnd = pszBegin;
		while(*pchEnd != CharT()){
			++pchEnd;
		}
		return pchEnd;
	}

	template<typename CharT, typename IteratorT>
	std::size_t StrChrRep(IteratorT itBegin, std::common_type_t<IteratorT> itEnd,
		CharT chToFind, std::size_t uRepCount) noexcept
	{
		ASSERT(uRepCount != 0);
		ASSERT((std::size_t)(itEnd - itBegin) >= uRepCount);

		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uRepCount - 1);

		std::size_t uFound = NPOS;

		auto itCur = itBegin;
		do {
			const auto itPartBegin = std::find_if(itCur, itSearchEnd,
				[chToFind](CharT ch) noexcept { return ch == chToFind; });
			if(itPartBegin == itSearchEnd){
				break;
			}
			const auto itPartEnd = itPartBegin + (std::ptrdiff_t)uRepCount;
			itCur = std::find_if(itPartBegin, itPartEnd,
				[chToFind](CharT ch) noexcept { return ch != chToFind; });
			if(itCur == itPartEnd){
				uFound = (std::size_t)(itPartBegin - itBegin);
				break;
			}
			++itCur;
		} while(itCur < itSearchEnd);

		return uFound;
	}

	template<typename IteratorT, typename ToFindIteratorT>
	std::size_t StrStr(IteratorT itBegin, std::common_type_t<IteratorT> itEnd,
		ToFindIteratorT itToFindBegin, std::common_type_t<ToFindIteratorT> itToFindEnd) noexcept
	{
		ASSERT(itToFindEnd >= itToFindBegin);
		ASSERT(itEnd - itBegin >= itToFindEnd - itToFindBegin);

		const auto uToFindLen = (std::size_t)(itToFindEnd - itToFindBegin);
		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uToFindLen - 1);

		std::size_t *puKmpTable;

		std::size_t auSmallTable[256];
		if(uToFindLen <= COUNT_OF(auSmallTable)){
			puKmpTable = auSmallTable;
		} else {
			puKmpTable = new(std::nothrow) std::size_t[uToFindLen];
			if(!puKmpTable){
				// 内存不足，使用暴力搜索方法。
				for(auto itCur = itBegin; itCur != itSearchEnd; ++itCur){
					if(std::equal(itToFindBegin, itToFindEnd, itCur)){
						return (std::size_t)(itCur - itBegin);
					}
				}
				return NPOS;
			}
		}

		std::size_t uFound = NPOS;

		puKmpTable[0] = 0;
		puKmpTable[1] = 0;

		std::size_t uPos = 2;
		std::size_t uCand = 0;
		while(uPos < uToFindLen){
			if(itToFindBegin[(std::ptrdiff_t)(uPos - 1)] == itToFindBegin[(std::ptrdiff_t)uCand]){
				puKmpTable[uPos++] = ++uCand;
			} else if(uCand != 0){
				uCand = puKmpTable[uCand];
			} else {
				puKmpTable[uPos++] = 0;
			}
		}

		auto itCur = itBegin;
		std::size_t uToSkip = 0;
		do {
			const auto vResult = std::mismatch(
				itToFindBegin + (std::ptrdiff_t)uToSkip, itToFindEnd, itCur + (std::ptrdiff_t)uToSkip);
			if(vResult.first == itToFindEnd){
				uFound = (std::size_t)(itCur - itBegin);
				break;
			}
			auto uDelta = (std::size_t)(vResult.first - itToFindBegin);
			uToSkip = puKmpTable[uDelta];
			uDelta -= uToSkip;
			uDelta += (std::size_t)(*vResult.second != *itToFindBegin);
			itCur += (std::ptrdiff_t)uDelta;
		} while(itCur < itSearchEnd);

		if(puKmpTable != auSmallTable){
			delete[] puKmpTable;
		}
		return uFound;
	}
}

template<StringType TYPE_T>
struct StringObserver {
public:
	static constexpr StringType Type = TYPE_T;
	using CharType = typename StringEncodingTrait<TYPE_T>::Type;

	static constexpr std::size_t NPOS = Impl::NPOS;

	static_assert(std::is_integral<CharType>::value, "CharType must be an integral type.");

private:
	// 为了方便理解，想象此处使用的是所谓“插入式光标”：

	// 字符串内容：    a   b   c   d   e   f   g
	// 正光标位置：  0   1   2   3   4   5   6   7
	// 负光标位置： -8  -7  -6  -5  -4  -3  -2  -1

	// 以下均以此字符串为例。
	static std::size_t xTranslateOffset(std::ptrdiff_t nOffset, std::size_t uLength) noexcept {
		auto uRet = (std::size_t)nOffset;
		if(nOffset < 0){
			uRet += uLength + 1;
		}
		ASSERT_MSG(uRet <= uLength, L"索引越界。");
		return uRet;
	}

private:
	const CharType *xm_pchBegin;
	const CharType *xm_pchEnd;

public:
	constexpr StringObserver() noexcept
		: xm_pchBegin(nullptr), xm_pchEnd(nullptr)
	{
	}
	constexpr StringObserver(const CharType *pchBegin, const CharType *pchEnd) noexcept
		: xm_pchBegin(pchBegin), xm_pchEnd(pchEnd)
	{
	}
	constexpr StringObserver(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringObserver()
	{
	}
	constexpr StringObserver(const CharType *pchBegin, std::size_t uLen) noexcept
		: xm_pchBegin(pchBegin), xm_pchEnd(pchBegin + uLen)
	{
	}
	constexpr StringObserver(std::initializer_list<CharType> rhs) noexcept
		: StringObserver(rhs.begin(), rhs.size())
	{
	}
	explicit StringObserver(const CharType *pszBegin) noexcept
		: StringObserver(pszBegin, Impl::StrEndOf(pszBegin))
	{
	}

public:
	const CharType *GetBegin() const noexcept {
		return xm_pchBegin;
	}
	const CharType *GetEnd() const noexcept {
		return xm_pchEnd;
	}
	std::size_t GetSize() const noexcept {
		return (std::size_t)(xm_pchEnd - xm_pchBegin);
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		xm_pchEnd = xm_pchBegin;
	}

	void Swap(StringObserver &rhs) noexcept {
		std::swap(xm_pchBegin, rhs.xm_pchBegin);
		std::swap(xm_pchEnd, rhs.xm_pchEnd);
	}

	int Compare(const StringObserver &rhs) const noexcept {
		auto pLRead = GetBegin();
		const auto pLEnd = GetEnd();
		auto pRRead = rhs.GetBegin();
		const auto pREnd = rhs.GetEnd();
		for(;;){
			const int nLAtEnd = (pLRead == pLEnd) ? 3 : 0;
			const int nRAtEnd = (pRRead == pREnd) ? 1 : 0;
			const int nResult = 2 - (nLAtEnd ^ nRAtEnd);
			if(nResult != 2){
				return nResult;
			}

			using UChar = std::make_unsigned_t<CharType>;
			const auto uchLhs = (UChar)*pLRead;
			const auto uchRhs = (UChar)*pRRead;
			if(uchLhs != uchRhs){
				return (uchLhs < uchRhs) ? -1 : 1;
			}
			++pLRead;
			++pRRead;
		}
	}

	void Assign(const CharType *pchBegin, const CharType *pchEnd) noexcept {
		xm_pchBegin = pchBegin;
		xm_pchEnd = pchEnd;
	}
	void Assign(std::nullptr_t, std::nullptr_t = nullptr) noexcept {
		xm_pchBegin = nullptr;
		xm_pchEnd = nullptr;
	}
	void Assign(const CharType *pchBegin, std::size_t uLen) noexcept {
		Assign(pchBegin, pchBegin + uLen);
	}
	void Assign(std::initializer_list<CharType> rhs) noexcept {
		Assign(rhs.begin(), rhs.end());
	}
	void Assign(const CharType *pszBegin) noexcept {
		Assign(pszBegin, Impl::StrEndOf(pszBegin));
	}

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringObserver Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd) const noexcept {
		const auto uLength = GetLength();
		return StringObserver(xm_pchBegin + xTranslateOffset(nBegin, uLength), xm_pchBegin + xTranslateOffset(nEnd, uLength));
	}

	// 举例：
	//   Find("def", 3)				返回 3；
	//   Find("def", 4)				返回 NPOS；
	//   FindBackward("def", 5)		返回 NPOS；
	//   FindBackward("def", 6)		返回 3。
	std::size_t Find(const StringObserver &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(nBegin, uLength);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealBegin;
		}
		if(uLength < uLenToFind){
			return NPOS;
		}
		if(uRealBegin + uLenToFind > uLength){
			return NPOS;
		}
		const auto uPos = Impl::StrStr(GetBegin() + uRealBegin, GetEnd(), obsToFind.GetBegin(), obsToFind.GetEnd());
		if(uPos == NPOS){
			return NPOS;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindBackward(const StringObserver &obsToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(nEnd, uLength);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealEnd;
		}
		if(uLength < uLenToFind){
			return NPOS;
		}
		if(uRealEnd < uLenToFind){
			return NPOS;
		}
		std::reverse_iterator<const CharType *> itBegin(GetBegin() + uRealEnd), itEnd(GetBegin()),
			itToFindBegin(obsToFind.GetEnd()), itToFindEnd(obsToFind.GetBegin());
		const auto uPos = Impl::StrStr(itBegin, itEnd, itToFindBegin, itToFindEnd);
		if(uPos == NPOS){
			return NPOS;
		}
		return uRealEnd - uPos - uLenToFind;
	}

	// 举例：
	//   Find('c', 3)			返回 NPOS；
	//   Find('d', 3)			返回 3；
	//   FindBackward('c', 3)	返回 2；
	//   FindBackward('d', 3)	返回 NPOS。
	std::size_t FindRep(CharType chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(nBegin, uLength);
		if(uRepCount == 0){
			return uRealBegin;
		}
		if(uLength < uRepCount){
			return NPOS;
		}
		if(uRealBegin < uRepCount){
			return NPOS;
		}
		const auto uPos = Impl::StrChrRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uRepCount);
		if(uPos == NPOS){
			return NPOS;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindRepBackward(CharType chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(nEnd, uLength);
		if(uRepCount == 0){
			return uRealEnd;
		}
		if(uLength < uRepCount){
			return NPOS;
		}
		if(uRealEnd < uRepCount){
			return NPOS;
		}
		std::reverse_iterator<const CharType *> itBegin(GetBegin() + uRealEnd), itEnd(GetBegin());
		const auto uPos = Impl::StrChrRep(itBegin, itEnd, chToFind, uRepCount);
		if(uPos == NPOS){
			return NPOS;
		}
		return uRealEnd - uPos - uRepCount;
	}
	std::size_t Find(CharType chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return FindRep(chToFind, 1, nBegin);
	}
	std::size_t FindBackward(CharType chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return FindRepBackward(chToFind, 1, nEnd);
	}

	bool DoesOverlapWith(const StringObserver &rhs) const noexcept {
		return std::less<void>()(xm_pchBegin, rhs.xm_pchEnd) && std::less<void>()(rhs.xm_pchBegin, xm_pchEnd);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const CharType &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetBegin()[uIndex];
	}
};

template<StringType TYPE_T>
bool operator==(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return false;
	}
	return lhs.Compare(rhs) == 0;
}
template<StringType TYPE_T>
bool operator!=(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return true;
	}
	return lhs.Compare(rhs) != 0;
}
template<StringType TYPE_T>
bool operator<(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.Compare(rhs) < 0;
}
template<StringType TYPE_T>
bool operator>(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.Compare(rhs) > 0;
}
template<StringType TYPE_T>
bool operator<=(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.Compare(rhs) <= 0;
}
template<StringType TYPE_T>
bool operator>=(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.Compare(rhs) >= 0;
}

template<StringType TYPE_T>
void swap(StringObserver<TYPE_T> &lhs, StringObserver<TYPE_T> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType TYPE_T>
auto begin(const StringObserver<TYPE_T> &lhs) noexcept {
	return lhs.GetBegin();
}
template<StringType TYPE_T>
auto cbegin(const StringObserver<TYPE_T> &lhs) noexcept {
	return lhs.GetBegin();
}
template<StringType TYPE_T>
auto end(const StringObserver<TYPE_T> &lhs) noexcept {
	return lhs.GetEnd();
}
template<StringType TYPE_T>
auto cend(const StringObserver<TYPE_T> &lhs) noexcept {
	return lhs.GetEnd();
}

extern template class StringObserver<StringType::NARROW>;
extern template class StringObserver<StringType::WIDE>;
extern template class StringObserver<StringType::UTF8>;
extern template class StringObserver<StringType::UTF16>;
extern template class StringObserver<StringType::UTF32>;
extern template class StringObserver<StringType::CESU8>;
extern template class StringObserver<StringType::ANSI>;

using NarrowStringObserver		= StringObserver<StringType::NARROW>;
using WideStringObserver		= StringObserver<StringType::WIDE>;
using Utf8StringObserver		= StringObserver<StringType::UTF8>;
using Utf16StringObserver		= StringObserver<StringType::UTF16>;
using Utf32StringObserver		= StringObserver<StringType::UTF32>;
using Cesu8StringObserver		= StringObserver<StringType::CESU8>;
using AnsiStringObserver		= StringObserver<StringType::ANSI>;

// 字面量运算符。
// 注意 StringObserver 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
template<typename CharT, CharT ...STRING_T>
[[deprecated("Be warned that encodings of narrow string literals vary from compilers to compilers "
	"and might even depend on encodings of source files on g++.")]]
extern inline auto operator""_nso() noexcept {
	static constexpr char s_achData[] = { STRING_T..., '$' };
	return NarrowStringObserver(s_achData, sizeof...(STRING_T));
}
template<typename CharT, CharT ...STRING_T>
extern inline auto operator""_wso() noexcept {
	static constexpr wchar_t s_awcData[] = { STRING_T..., '$' };
	return WideStringObserver(s_awcData, sizeof...(STRING_T));
}
template<typename CharT, CharT ...STRING_T>
extern inline auto operator""_u8so() noexcept {
	static constexpr char s_au8cData[] = { STRING_T..., '$' };
	return Utf8StringObserver(s_au8cData, sizeof...(STRING_T));
}
template<typename CharT, CharT ...STRING_T>
extern inline auto operator""_u16so() noexcept {
	static constexpr char16_t s_au16cData[] = { STRING_T..., '$' };
	return Utf16StringObserver(s_au16cData, sizeof...(STRING_T));
}
template<typename CharT, CharT ...STRING_T>
extern inline auto operator""_u32so() noexcept {
	static constexpr char32_t s_au32cData[] = { STRING_T..., '$' };
	return Utf32StringObserver(s_au32cData, sizeof...(STRING_T));
}

// MultiIndexMap
template<StringType TYPE_T>
struct StringObserverTripleComparator {
	int operator()(const StringObserver<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) const noexcept {
		return lhs.Compare(rhs);
	}
	template<typename ComparandT>
	int operator()(const StringObserver<TYPE_T> &lhs, const ComparandT &rhs) const noexcept {
		return lhs.Compare(rhs);
	}
	template<typename ComparandT>
	int operator()(const ComparandT &lhs, const StringObserver<TYPE_T> &rhs) const noexcept {
		return -rhs.Compare(lhs);
	}
};

template<StringType TYPE_T>
	StringObserverTripleComparator<TYPE_T> GetDefaultComparator(const StringObserver<TYPE_T> &) noexcept;

}

using ::MCF::operator""_nso;
using ::MCF::operator""_wso;
using ::MCF::operator""_u8so;
using ::MCF::operator""_u16so;
using ::MCF::operator""_u32so;

#endif
