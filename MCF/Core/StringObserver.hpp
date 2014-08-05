// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STRING_OBSERVER_HPP_
#define MCF_STRING_OBSERVER_HPP_

#include "../Containers/VVector.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

template<typename Char>
class StringObserver;

template<typename Char>
class StringObserver {
public:
	enum : std::size_t {
		NPOS = (std::size_t)-1
	};

private:
	static const Char *xEndOf(const Char *pszBegin) noexcept {
		if(pszBegin == nullptr){
			return nullptr;
		}

		const Char *pchEnd = pszBegin;
		while(*pchEnd != Char()){
			++pchEnd;
		}
		return pchEnd;
	}

	static std::size_t xTranslateOffset(std::size_t uLength, std::ptrdiff_t nRaw) noexcept {
		std::ptrdiff_t nOffset = nRaw;
		if(nOffset < 0){
			nOffset += (std::ptrdiff_t)(uLength + 1);
		}

		ASSERT_MSG(nOffset >= 0, L"索引越界。");
		ASSERT_MSG((std::size_t)nOffset <= uLength, L"索引越界。");

		return (std::size_t)nOffset;
	}

	template<typename Iterator>
	static std::size_t xFindRep(const Iterator &itBegin, const Iterator &itEnd, Char chToFind, std::size_t uRepCount) noexcept {
		ASSERT(uRepCount != 0);
		ASSERT((std::size_t)(itEnd - itBegin) >= uRepCount);

		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uRepCount - 1);

		std::size_t uFound = NPOS;

		auto itCur = itBegin;
		do {
			const auto itPartBegin = std::find_if(itCur, itSearchEnd, [chToFind](Char ch) noexcept { return ch == chToFind; });
			if(itPartBegin == itSearchEnd){
				break;
			}
			const auto itPartEnd = itPartBegin + (std::ptrdiff_t)uRepCount;
			itCur = std::find_if(itPartBegin, itPartEnd, [chToFind](Char ch) noexcept { return ch != chToFind; });
			if(itCur == itPartEnd){
				uFound = (std::size_t)(itPartBegin - itBegin);
				break;
			}
			++itCur;
		} while(itCur < itSearchEnd);

		return uFound;
	}

	template<typename Iterator>
	static std::size_t xKmpSearch(
		const Iterator &itBegin,
		const Iterator &itEnd,
		const Iterator &itToFindBegin,
		const Iterator &itToFindEnd
	) noexcept {
		ASSERT(itToFindEnd >= itToFindBegin);
		ASSERT(itEnd - itBegin >= itToFindEnd - itToFindBegin);

		const auto uToFindLen = (std::size_t)(itToFindEnd - itToFindBegin);
		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uToFindLen - 1);

		std::size_t *puKmpTable;

		std::size_t auSmallTable[256];
		if(uToFindLen <= COUNT_OF(auSmallTable)){
			puKmpTable = auSmallTable;
		} else {
			puKmpTable = (std::size_t *)::operator new(sizeof(std::size_t) * uToFindLen, std::nothrow);
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
				itToFindBegin + (std::ptrdiff_t)uToSkip,
				itToFindEnd,
				itCur + (std::ptrdiff_t)uToSkip
			);
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
			::operator delete(puKmpTable);
		}
		return uFound;
	}

private:
	const Char *xm_pchBegin;
	const Char *xm_pchEnd;

public:
#ifdef NDEBUG
	constexpr
#endif
	StringObserver(const Char *pchBegin, const Char *pchEnd) noexcept
		: xm_pchBegin(pchBegin), xm_pchEnd(pchEnd)
	{
#ifndef NDEBUG
		ASSERT(pchBegin <= pchEnd);
#endif
	}
	constexpr StringObserver() noexcept
		: StringObserver((const Char *)nullptr, nullptr)
	{
	}
	constexpr StringObserver(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringObserver()
	{
	}
	constexpr StringObserver(const Char *pchBegin, std::size_t uLen) noexcept
		: StringObserver(pchBegin, pchBegin + uLen)
	{
	}
	constexpr StringObserver(std::initializer_list<Char> vInitList) noexcept
		: StringObserver(vInitList.begin(), vInitList.end())
	{
	}
	explicit constexpr StringObserver(const Char *pszBegin) noexcept
		: StringObserver(pszBegin, xEndOf(pszBegin))
	{
	}
	StringObserver &operator=(std::nullptr_t) noexcept {
		xm_pchBegin = nullptr;
		xm_pchEnd = nullptr;
		return *this;
	}
	StringObserver &operator=(std::initializer_list<Char> vInitList) noexcept {
		xm_pchBegin = vInitList.begin();
		xm_pchEnd = vInitList.end();
		return *this;
	}
	StringObserver &operator=(const Char *pszBegin) noexcept {
		xm_pchBegin = pszBegin;
		xm_pchEnd = xEndOf(pszBegin);
		return *this;
	}

public:
	const Char *GetBegin() const noexcept {
		return xm_pchBegin;
	}
	const Char *GetCBegin() const noexcept {
		return xm_pchBegin;
	}
	const Char *GetEnd() const noexcept {
		return xm_pchEnd;
	}
	const Char *GetCEnd() const noexcept {
		return xm_pchEnd;
	}
	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		Assign(nullptr, nullptr);
	}

	void Swap(StringObserver &rhs) noexcept {
		std::swap(xm_pchBegin, rhs.xm_pchBegin);
		std::swap(xm_pchEnd, rhs.xm_pchEnd);
	}

	int Compare(const StringObserver &rhs) const noexcept {
		auto itLRead = GetBegin();
		const auto itLEnd = GetEnd();
		auto itRRead = rhs.GetBegin();
		const auto itREnd = rhs.GetEnd();
		for(;;){
			const int nResult = 2 - (((itLRead == itLEnd) ? 3 : 0) ^ ((itRRead == itREnd) ? 1 : 0));
			if(nResult != 2){
				return nResult;
			}

			typedef typename std::make_unsigned<Char>::type UChar;

			const auto uchL = (UChar)*itLRead;
			const auto uchR = (UChar)*itRRead;
			if(uchL != uchR){
				return (uchL < uchR) ? -1 : 1;
			}
			++itLRead;
			++itRRead;
		}
	}

	void Assign(const Char *pchBegin, const Char *pchEnd) noexcept {
		ASSERT(pchBegin <= pchEnd);

		xm_pchBegin = pchBegin;
		xm_pchEnd = pchEnd;
	}
	void Assign(std::nullptr_t, std::nullptr_t = nullptr) noexcept {
		Assign((const Char *)nullptr, nullptr);
	}
	void Assign(const Char *pchBegin, std::size_t uLen) noexcept {
		Assign(pchBegin, pchBegin + uLen);
	}
	void Assign(const Char *pszBegin) noexcept {
		Assign(pszBegin, xEndOf(pszBegin));
	}

	// 为了方便理解，想象此处使用的是所谓“插入式光标”：

	// 字符串内容：    a   b   c   d   e   f   g  \0
	// 正光标位置：  0   1   2   3   4   5   6   7
	// 负光标位置： -8  -7  -6  -5  -4  -3  -2  -1

	// 以下均以此字符串为例。

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringObserver Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto pchBegin = GetBegin();
		const auto uLength = GetLength();
		return StringObserver(
			pchBegin + xTranslateOffset(uLength, nBegin),
			pchBegin + xTranslateOffset(uLength, nEnd)
		);
	}

	// 举例：
	//   Find("def", 3)				返回 3；
	//   Find("def", 4)				返回 NPOS；
	//   FindBackward("def", 5)		返回 NPOS；
	//   FindBackward("def", 6)		返回 3。
	std::size_t Find(const StringObserver &obsToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
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

		const auto uPos = xKmpSearch(
			GetBegin() + uRealBegin, GetEnd(),
			obsToFind.GetBegin(), obsToFind.GetEnd()
		);
		return (uPos == NPOS) ? NPOS : (uPos + uRealBegin);
	}
	std::size_t FindBackward(const StringObserver &obsToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
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

		typedef std::reverse_iterator<const Char *> RevIterator;

		const auto uPos = xKmpSearch(
			RevIterator(GetBegin() + uRealEnd), RevIterator(GetBegin()),
			RevIterator(obsToFind.GetBegin()), RevIterator(obsToFind.GetEnd())
		);
		return (uPos == NPOS) ? NPOS : (uRealEnd - uPos - uLenToFind);
	}

	// 举例：
	//   Find('c', 3)			返回 NPOS；
	//   Find('d', 3)			返回 3；
	//   FindBackward('c', 3)	返回 2；
	//   FindBackward('d', 3)	返回 NPOS。
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRepCount == 0){
			return uRealBegin;
		}
		if(uLength < uRepCount){
			return NPOS;
		}
		if(uRealBegin + uRepCount > uLength){
			return NPOS;
		}

		const auto uPos = xFindRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uRepCount);
		return (uPos == NPOS) ? NPOS : (uPos + uRealBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
		if(uRepCount == 0){
			return uRealEnd;
		}
		if(uLength < uRepCount){
			return NPOS;
		}
		if(uRealEnd < uRepCount){
			return NPOS;
		}

		typedef std::reverse_iterator<const Char *> RevIterator;

		const auto uPos = xFindRep(RevIterator(GetBegin() + uRealEnd), RevIterator(GetBegin()), chToFind, uRepCount);
		return (uPos == NPOS) ? NPOS : (uRealEnd - uPos - uRepCount);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return FindRep(chToFind, 1, nOffsetBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return FindRepBackward(chToFind, 1, nOffsetEnd);
	}

	bool DoesOverlapWith(const StringObserver &obs) const noexcept {
		const Char *pchBegin1, *pchEnd1, *pchBegin2, *pchEnd2;
		if(xm_pchBegin <= xm_pchEnd){
			pchBegin1 = xm_pchBegin;
			pchEnd1 = xm_pchEnd;
		} else {
			pchBegin1 = xm_pchEnd + 1;
			pchEnd1 = xm_pchBegin + 1;
		}
		if(obs.xm_pchBegin <= obs.xm_pchEnd){
			pchBegin2 = obs.xm_pchBegin;
			pchEnd2 = obs.xm_pchEnd;
		} else {
			pchBegin2 = obs.xm_pchEnd + 1;
			pchEnd2 = obs.xm_pchBegin + 1;
		}
		return (pchBegin1 < pchEnd2) && (pchBegin2 < pchEnd1);
	}

	template<std::size_t SIZE_HINT>
	VVector<Char, SIZE_HINT> GetNullTerminated() const {
		VVector<Char, SIZE_HINT> vecRet;
		vecRet.Reserve(GetLength() + 1);
		vecRet.CopyToEnd(GetBegin(), GetEnd());
		vecRet.Push(Char());
		return std::move(vecRet);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetSize(), L"索引越界。");

		return GetBegin()[uIndex];
	}

	bool operator==(const StringObserver &rhs) const noexcept {
		if(GetLength() != rhs.GetLength()){
			return false;
		}
		return Compare(rhs) == 0;
	}
	bool operator!=(const StringObserver &rhs) const noexcept {
		if(GetLength() != rhs.GetLength()){
			return true;
		}
		return Compare(rhs) != 0;
	}
	bool operator<(const StringObserver &rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator>(const StringObserver &rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator<=(const StringObserver &rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator>=(const StringObserver &rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
};

template<typename Comparand, typename Char>
bool operator==(const Comparand &lhs, const StringObserver<Char> &rhs) noexcept {
	return rhs == lhs;
}
template<typename Comparand, typename Char>
bool operator!=(const Comparand &lhs, const StringObserver<Char> &rhs) noexcept {
	return rhs != lhs;
}
template<typename Comparand, typename Char>
bool operator<(const Comparand &lhs, const StringObserver<Char> &rhs) noexcept {
	return rhs > lhs;
}
template<typename Comparand, typename Char>
bool operator>(const Comparand &lhs, const StringObserver<Char> &rhs) noexcept {
	return rhs < lhs;
}
template<typename Comparand, typename Char>
bool operator<=(const Comparand &lhs, const StringObserver<Char> &rhs) noexcept {
	return rhs >= lhs;
}
template<typename Comparand, typename Char>
bool operator>=(const Comparand &lhs, const StringObserver<Char> &rhs) noexcept {
	return rhs <= lhs;
}

template<typename Char>
const Char *begin(const StringObserver<Char> &obs) noexcept {
	return obs.GetBegin();
}
template<typename Char>
const Char *cbegin(const StringObserver<Char> &obs) noexcept {
	return obs.GetCBegin();
}

template<typename Char>
const Char *end(const StringObserver<Char> &obs) noexcept {
	return obs.GetEnd();
}
template<typename Char>
const Char *cend(const StringObserver<Char> &obs) noexcept {
	return obs.GetCEnd();
}

template class StringObserver<char>;
template class StringObserver<wchar_t>;

template class StringObserver<char16_t>;
template class StringObserver<char32_t>;

typedef StringObserver<char>		NarrowStringObserver;
typedef StringObserver<wchar_t>		WideStringObserver;

typedef StringObserver<char>		Utf8StringObserver;
typedef StringObserver<char16_t>	Utf16StringObserver;
typedef StringObserver<char32_t>	Utf32StringObserver;

// 字面量运算符。
// 注意 StringObserver 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char>::value, NarrowStringObserver>::type
	operator""_nso()
{
	static Char s_achData[] = {STRING..., '$'};
	return NarrowStringObserver(s_achData, sizeof...(STRING));
}
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, wchar_t>::value, WideStringObserver>::type
	operator""_wso()
{
	static Char s_achData[] = {STRING..., '$'};
	return WideStringObserver(s_achData, sizeof...(STRING));
}

template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char>::value, Utf8StringObserver>::type
	operator""_u8so()
{
	static Char s_achData[] = {STRING..., '$'};
	return Utf8StringObserver(s_achData, sizeof...(STRING));
}
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char16_t>::value, Utf16StringObserver>::type
	operator""_u16so()
{
	static Char s_achData[] = {STRING..., '$'};
	return Utf16StringObserver(s_achData, sizeof...(STRING));
}
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char32_t>::value, Utf32StringObserver>::type
	operator""_u32so()
{
	static Char s_achData[] = {STRING..., '$'};
	return Utf32StringObserver(s_achData, sizeof...(STRING));
}

}

using ::MCF::operator""_nso;
using ::MCF::operator""_wso;
using ::MCF::operator""_u8so;
using ::MCF::operator""_u16so;
using ::MCF::operator""_u32so;

#endif
