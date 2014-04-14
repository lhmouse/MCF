// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_OBSERVER_HPP__
#define __MCF_STRING_OBSERVER_HPP__

#include "../../MCFCRT/c/ext/assert.h"
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename Char_t>
class StringObserver;

template<typename Char_t>
class StringObserverIterator
	: public std::iterator<std::random_access_iterator_tag, const Char_t>
{
public:
	typedef StringObserver<Char_t> Observer;

	friend Observer;

private:
	const Char_t *xm_pchCurrent;
	std::intptr_t xm_nIncrement;

private:
	constexpr StringObserverIterator(const Char_t *pchCurrent, std::intptr_t nDirection) noexcept
		: xm_pchCurrent(pchCurrent)
		, xm_nIncrement((nDirection >= 0) ? 0 : -1)
	{
	}

public:
	constexpr StringObserverIterator() noexcept
		: StringObserverIterator(nullptr, 0)
	{
	}

public:
	explicit operator const Char_t *() const noexcept {
		return xm_pchCurrent;
	}

	bool operator==(const StringObserverIterator &rhs) const noexcept {
		return *this - rhs == 0;
	}
	bool operator!=(const StringObserverIterator &rhs) const noexcept {
		return *this - rhs != 0;
	}

	const Char_t &operator*() const noexcept {
		return *xm_pchCurrent;
	}
	const Char_t *operator->() const noexcept {
		return xm_pchCurrent;
	}

	StringObserverIterator &operator++() noexcept {
		xm_pchCurrent += (xm_nIncrement | 1);
		return *this;
	}
	StringObserverIterator operator++(int) noexcept {
		auto itRet(*this);
		++*this;
		return std::move(itRet);
	}
	StringObserverIterator &operator--() noexcept {
		xm_pchCurrent -= (xm_nIncrement | 1);
		return *this;
	}
	StringObserverIterator operator--(int) noexcept {
		auto itRet(*this);
		--*this;
		return std::move(itRet);
	}

	StringObserverIterator operator+(std::ptrdiff_t rhs) const noexcept {
		return std::move(StringObserverIterator(*this) += rhs);
	}
	StringObserverIterator operator-(std::ptrdiff_t rhs) const noexcept {
		return std::move(StringObserverIterator(*this) -= rhs);
	}
	std::ptrdiff_t operator-(const StringObserverIterator &rhs) const noexcept {
		return ((xm_pchCurrent - rhs.xm_pchCurrent) ^ xm_nIncrement) - xm_nIncrement;
	}

	bool operator<(const StringObserverIterator &rhs) const noexcept {
		return *this - rhs < 0;
	}
	bool operator<=(const StringObserverIterator &rhs) const noexcept {
		return *this - rhs <= 0;
	}
	bool operator>(const StringObserverIterator &rhs) const noexcept {
		return *this - rhs > 0;
	}
	bool operator>=(const StringObserverIterator &rhs) const noexcept {
		return *this - rhs >= 0;
	}

	StringObserverIterator &operator+=(std::ptrdiff_t rhs) noexcept {
		xm_pchCurrent += (rhs ^ xm_nIncrement) - xm_nIncrement;
		return *this;
	}
	StringObserverIterator &operator-=(std::ptrdiff_t rhs) noexcept {
		return *this += -rhs;;
	}

	const Char_t &operator[](std::ptrdiff_t nIndex) const noexcept {
		return *(xm_pchCurrent + nIndex);
	}
};

template<typename Char_t>
StringObserverIterator<Char_t> operator+(
	std::ptrdiff_t lhs,
	const StringObserverIterator<Char_t> &rhs
) noexcept {
	return rhs + lhs;
}

template<typename Char_t>
class StringObserver {
public:
	enum : std::size_t {
		NPOS = (std::size_t)-1
	};

public:
	typedef StringObserverIterator<Char_t> Iterator;

private:
	static const Char_t *xEndOf(const Char_t *pszBegin) noexcept {
		if(pszBegin == nullptr){
			return nullptr;
		}

		const Char_t *pchEnd = pszBegin;
		while(*pchEnd != Char_t()){
			++pchEnd;
		}
		return pchEnd;
	}

	static std::size_t xTranslateOffset(std::size_t uLength, std::ptrdiff_t nRaw) noexcept {
		std::ptrdiff_t nOffset = nRaw;
		if(nOffset < 0){
			nOffset += uLength + 1;
		}
		if(nOffset < 0){
			nOffset = 0;
		} else if((std::size_t)nOffset > uLength){
			nOffset = uLength;
		}
		return (std::size_t)nOffset;
	}

	template<typename IteratorS_t, typename IteratorW_t>
	static std::size_t xKmp(IteratorS_t s, std::size_t slen, IteratorW_t w, std::size_t wlen) noexcept {
		if(wlen == 0){
			return 0;
		}
		if(slen < wlen){
			return NPOS;
		}

		// http://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm
		std::size_t t_sm[64];
		std::size_t *t;
		if(wlen <= COUNT_OF(t_sm)){
			t = t_sm;
		} else {
			t = new(std::nothrow) std::size_t[wlen];
			if(!t){
				// 内存不足，使用暴力搜索方法。
				const auto till = s  + (slen - wlen);
				auto p = s;
				do {
					if(std::equal(p, p + wlen, w)){
						return p - s;
					}
				} while(++p != till);

				return NPOS;
			}
		}

		std::size_t pos = 2;
		std::size_t cnd = 0;

		t[0] = (std::size_t)-1;
		t[1] = 0;

		while(pos < wlen){
			if(w[pos - 1] == w[cnd]){
				++cnd;
				t[pos] = cnd;
				++pos;
			} else if(cnd != 0){
				cnd = t[cnd];
			} else {
				t[pos] = 0;
				++pos;
			}
		}

		std::size_t found = NPOS;

		std::size_t m = 0;
		std::size_t i = 0;
		while(m + i < slen){
			if(w[i] == s[m + i]){
				if(i == wlen - 1){
					found = m;
					break;
				}
				++i;
			} else {
				const auto entry = t[i];
				m += i - entry;
				if(entry != (std::size_t)-1){
					i = entry;
				} else {
					i = 0;
				}
			}
		}

		if(t != t_sm){
			delete[] t;
		}
		return found;
	}

private:
	const Char_t *xm_pchBegin;
	const Char_t *xm_pchEnd;

public:
	constexpr StringObserver(const Char_t *pchBegin, const Char_t *pchEnd) noexcept
		: xm_pchBegin(pchBegin), xm_pchEnd(pchEnd)
	{
	}
	constexpr StringObserver() noexcept
		: StringObserver((const Char_t *)nullptr, nullptr)
	{
	}
	constexpr StringObserver(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringObserver()
	{
	}
	constexpr StringObserver(const Char_t *pchBegin, std::size_t uLen) noexcept
		: StringObserver(pchBegin, pchBegin + uLen)
	{
	}
	template<typename Test_t, typename = typename std::enable_if<std::is_same<Test_t, Char_t>::value>::type>
	constexpr StringObserver(const Test_t *const &pszBegin) noexcept
		: StringObserver(pszBegin, xEndOf(pszBegin))
	{
	}
	template<std::size_t N>
	constexpr StringObserver(const Char_t (&achLiteral)[N]) noexcept
		: StringObserver(achLiteral, achLiteral + N - 1)
	{
	}
	template<std::size_t N>
	constexpr StringObserver(Char_t (&achNonLiteral)[N]) noexcept
		: StringObserver(achNonLiteral, xEndOf(achNonLiteral))
	{
	}
	constexpr StringObserver(const Iterator &itBegin, const Iterator &itEnd) noexcept
		: StringObserver(&*itBegin, *itEnd)
	{
	}

public:
	Iterator GetBegin() const noexcept {
		return Iterator(xm_pchBegin, xm_pchEnd - xm_pchBegin);
	}
	Iterator GetEnd() const noexcept {
		return Iterator(xm_pchEnd, xm_pchEnd - xm_pchBegin);
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

			typedef typename std::make_unsigned<Char_t>::type UChar;

			const auto uchL = (UChar)*itLRead;
			const auto uchR = (UChar)*itRRead;
			if(uchL != uchR){
				return (uchL < uchR) ? -1 : 1;
			}
			++itLRead;
			++itRRead;
		}
	}

	void Assign(const Char_t *pchBegin, const Char_t *pchEnd) noexcept {
		xm_pchBegin = pchBegin;
		xm_pchEnd = pchEnd;
	}
	void Assign(std::nullptr_t, std::nullptr_t = nullptr) noexcept {
		Assign((const Char_t *)nullptr, nullptr);
	}
	void Assign(const Char_t *pchBegin, std::size_t uLen) noexcept {
		Assign(pchBegin, pchBegin + uLen);
	}
	template<typename Test_t, typename = typename std::enable_if<std::is_same<Test_t, Char_t>::value>::type>
	void Assign(const Test_t *const &pszBegin) noexcept {
		Assign(pszBegin, xEndOf(pszBegin));
	}
	template<std::size_t N>
	void Assign(const Char_t (&achLiteral)[N]) noexcept {
		Assign(achLiteral, achLiteral + N - 1);
	}
	template<std::size_t N>
	void Assign(Char_t (&achNonLiteral)[N]) noexcept {
		Assign(achNonLiteral, xEndOf(achNonLiteral));
	}
	void Assign(const Iterator &itBegin, const Iterator &itEnd) noexcept {
		Assign(&*itBegin, &*itEnd);
	}

	// 为了方便理解，想象此处使用的是所谓“插入式光标”：

	// 字符串内容：    a   b   c   d   e   f   g  \0
	// 正光标位置：  0   1   2   3   4   5   6   7
	// 负光标位置： -8  -7  -6  -5  -4  -3  -2  -1
	// 以下均以此字符串为例。

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice(-1,  5)   返回 "gf"；
	//   Slice(-1, -5)   返回 "gfed"。
	StringObserver Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uRealBegin = xTranslateOffset(GetLength(), nBegin);
		const auto uRealEnd = xTranslateOffset(GetLength(), nEnd);
		const std::ptrdiff_t nShift = (uRealBegin <= uRealEnd) ? 0 : -1;
		return StringObserver(GetBegin() + uRealBegin + nShift, GetBegin() + uRealEnd + nShift);
	}

	// 举例：
	//   FindFirstAfter("def", 3)   返回 3；
	//   FindFirstAfter("def", 4)   返回 NPOS；
	//   FindLastBefore("def", 5)   返回 NPOS；
	//   FindLastBefore("def", 6)   返回 3。
	std::size_t FindFirstAfter(const StringObserver &obsToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return 0;
		}
		const auto uLength = GetLength();
		if(uLength < uLenToFind){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRealBegin + uLenToFind > uLength){
			return NPOS;
		}

		const auto uPos = xKmp(
			GetBegin() + uRealBegin,
			uLength - uRealBegin,
			obsToFind.GetBegin(),
			uLenToFind
		);
		return (uPos == NPOS) ? NPOS : (uPos + uRealBegin);
	}
	std::size_t FindLastBefore(const StringObserver &obsToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto uLenToFind = obsToFind.GetLength();
		const auto uLength = GetLength();
		if(uLenToFind == 0){
			return uLength;
		}
		if(uLength < uLenToFind){
			return NPOS;
		}
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
		if(uRealEnd < uLenToFind){
			return NPOS;
		}

		typedef std::reverse_iterator<Iterator> RevIterator;

		const auto uPos = xKmp(
			RevIterator(GetBegin() + uRealEnd),
			uRealEnd,
			RevIterator(obsToFind.GetEnd()),
			uLenToFind
		);
		return (uPos == NPOS) ? NPOS : (uRealEnd - uPos - uLenToFind);
	}

	// 举例：
	//   FindFirstAfter('c', 3)   返回 NPOS；
	//   FindFirstAfter('d', 3)   返回 3；
	//   FindLastBefore('c', 3)   返回 2；
	//   FindLastBefore('d', 3)   返回 NPOS。
	std::size_t FindFirstAfter(Char_t chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		if(IsEmpty()){
			return NPOS;
		}
		auto itPos = GetBegin() + xTranslateOffset(GetLength(), nOffsetBegin);
		const auto itEnd = GetEnd();
		while(itPos != itEnd){
			if(*itPos == chToFind){
				return itPos - GetBegin();
			}
			++itPos;
		}
		return NPOS;
	}
	std::size_t FindLastBefore(Char_t chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		if(IsEmpty()){
			return NPOS;
		}
		auto itPos = GetBegin() + xTranslateOffset(GetLength(), nOffsetEnd);
		const auto itBegin = GetBegin();
		while(itPos != itBegin){
			--itPos;
			if(*itPos == chToFind){
				return itPos - GetBegin();
			}
		}
		return NPOS;
	}

	StringObserver GetReverse() const noexcept {
		auto obsRet(*this);
		obsRet.Reverse();
		return std::move(obsRet);
	}
	void Reverse() noexcept {
		const auto nShift = (xm_pchBegin <= xm_pchEnd) ? -1 : 1;
		Assign(xm_pchEnd + nShift, xm_pchBegin + nShift);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char_t &operator[](std::size_t uIndex) const noexcept {
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

template<typename Char_t>
StringObserverIterator<Char_t> begin(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetBegin();
}
template<typename Char_t>
StringObserverIterator<Char_t> cbegin(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetBegin();
}

template<typename Char_t>
StringObserverIterator<Char_t> end(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetEnd();
}
template<typename Char_t>
StringObserverIterator<Char_t> cend(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetEnd();
}

template class StringObserver<char>;
template class StringObserver<wchar_t>;

typedef StringObserver<char>	NarrowStringObserver;
typedef StringObserver<wchar_t>	WideStringObserver;

}

#endif
