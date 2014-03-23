// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_HPP__
#define __MCF_STRING_HPP__

#include "../../MCFCRT/c/ext/assert.h"
#include "../../MCFCRT/cpp/ext/count_of.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <memory>
#include <new>
#include <utility>
#include <iterator>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>

namespace MCF {

template<typename Char_t>
inline std::size_t StrLen(const Char_t *pszSrc) noexcept {
	auto pchRead = pszSrc;
	while(*pchRead != Char_t()){
		++pchRead;
	}
	return (std::size_t)(pchRead - pszSrc);
}
template<>
inline std::size_t StrLen<char>(const char *pszSrc) noexcept {
	return std::strlen(pszSrc);
}
template<>
inline std::size_t StrLen<wchar_t>(const wchar_t *pszSrc) noexcept {
	return std::wcslen(pszSrc);
}

template<typename Char_t>
inline int StrCmp(const Char_t *pszSrc1, std::size_t uLen1, const Char_t *pszSrc2, std::size_t uLen2) noexcept {
	auto pchRead1 = pszSrc1;
	auto pchRead2 = pszSrc2;
	const auto uLoopCount = std::min(uLen1, uLen2);
	for(std::size_t i = 0; i < uLoopCount; ++i){
		typedef typename std::make_unsigned<Char_t>::type UChar_t;

		const auto uch1 = (UChar_t)*(pchRead1++);
		const auto uch2 = (UChar_t)*(pchRead2++);
		if(uch1 != uch2){
			return (uch1 < uch2) ? -1 : 1;
		}
	}
	if(uLen1 != uLen2){
		return (uLen1 < uLen2) ? -1 : 1;
	}
	return 0;
}
template<>
inline int StrCmp<char>(const char *pszSrc1, std::size_t uLen1, const char *pszSrc2, std::size_t uLen2) noexcept {
	const int nResult = std::memcmp(pszSrc1, pszSrc2, std::min(uLen1, uLen2));
	if(nResult != 0){
		return nResult;
	}
	if(uLen1 != uLen2){
		return (uLen1 < uLen2) ? -1 : 1;
	}
	return 0;
}
template<>
inline int StrCmp<wchar_t>(const wchar_t *pszSrc1, std::size_t uLen1, const wchar_t *pszSrc2, std::size_t uLen2) noexcept {
	const int nResult = std::wmemcmp(pszSrc1, pszSrc2, std::min(uLen1, uLen2));
	if(nResult != 0){
		return nResult;
	}
	if(uLen1 != uLen2){
		return (uLen1 < uLen2) ? -1 : 1;
	}
	return 0;
}

template<typename Char_t>
inline Char_t *StrChr(Char_t *pszSrc, std::size_t uLen, typename std::remove_cv<Char_t>::type chToFind) noexcept {
	auto pchRead = pszSrc;
	for(std::size_t i = 0; i < uLen; ++i){
		if(*pchRead == chToFind){
			return pchRead;
		}
		++pchRead;
	}
	return nullptr;
}
template<>
inline const char *StrChr<const char>(const char *pszSrc, std::size_t uLen, char chToFind) noexcept {
	return (char *)std::memchr(pszSrc, chToFind, uLen);
}
template<>
inline char *StrChr<char>(char *pszSrc, std::size_t uLen, char chToFind) noexcept {
	return (char *)std::memchr(pszSrc, chToFind, uLen);
}
template<>
inline const wchar_t *StrChr<const wchar_t>(const wchar_t *pszSrc, std::size_t uLen, wchar_t chToFind) noexcept {
	return std::wmemchr(pszSrc, chToFind, uLen);
}
template<>
inline wchar_t *StrChr<wchar_t>(wchar_t *pszSrc, std::size_t uLen, wchar_t chToFind) noexcept {
	return std::wmemchr(pszSrc, chToFind, uLen);
}

template<typename Char_t>
inline Char_t *StrRChr(Char_t *pszSrc, std::size_t uLen, typename std::remove_cv<Char_t>::type chToFind) noexcept {
	auto pchRead = pszSrc + uLen;
	for(std::size_t i = 0; i < uLen; ++i){
		--pchRead;
		if(*pchRead == chToFind){
			return pchRead;
		}
	}
	return nullptr;
}

template<typename Iterator_t>
auto Kmp(Iterator_t s, std::size_t slen, Iterator_t w, std::size_t wlen) noexcept
	-> typename std::remove_reference<decltype(*s)>::type *
{
	if(wlen == 0){
		return &*s;
	}

	if(slen < wlen){
		return nullptr;
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
					return &*p;
				}
			} while(++p != till);

			return nullptr;
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

	typename std::remove_reference<decltype(*s)>::type *found = nullptr;

	std::size_t m = 0;
	std::size_t i = 0;
	while(m + i < slen){
		if(w[i] == s[m + i]){
			if(i == wlen - 1){
				found = &*(s + m);
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
template<typename Char_t>
inline Char_t *StrStr(Char_t *s, std::size_t slen, Char_t *w, std::size_t wlen) noexcept {
	return Kmp(s, slen, w, wlen);
}
template<typename Char_t>
inline Char_t *StrRStr(Char_t *s, std::size_t slen, Char_t *w, std::size_t wlen) noexcept {
	typedef std::reverse_iterator<Char_t *> RevIter;
	const auto pchPos = Kmp(RevIter(s + slen), slen, RevIter(w + wlen), wlen);
	if(!pchPos){
		return nullptr;
	}
	return pchPos - wlen + 1;
}

enum class StringEncoding {
	UTF8,
	ANSI,
	UTF16
};

template<typename Char_t, StringEncoding ENCODING>
class GenericString;

typedef GenericString<wchar_t, StringEncoding::UTF16> UnifiedString;

template<typename Char_t, StringEncoding ENCODING>
class GenericString {
	static_assert(std::is_arithmetic<Char_t>::value, "Char_t must be an arithmetic type.");

	template<typename, StringEncoding>
	friend class GenericString;

public:
	enum : std::size_t {
		NPOS = (std::size_t)-1
	};

private:
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

private:
	struct xStorage {
		union {
			struct __attribute__((packed)) {
				Char_t achSmall[(32 - sizeof(Char_t *)) / sizeof(Char_t) - 2];
				Char_t chNull;
				typename std::make_unsigned<Char_t>::type uchSmallLength;
			};
			struct {
				Char_t *pchLargeBegin;
				std::size_t uLargeLength;
				std::size_t uLargeBufferSize;
			};
		};
	} xm_vStorage;

public:
	GenericString() noexcept {
		xm_vStorage.achSmall[0] = Char_t();
		xm_vStorage.chNull = Char_t();
		xm_vStorage.uchSmallLength = 0;
	}
	explicit GenericString(Char_t ch, std::size_t uCount = 1) : GenericString() {
		Assign(ch, uCount);
	}
	GenericString(const Char_t *pszSrc) : GenericString() {
		Assign(pszSrc);
	}
	GenericString(const Char_t *pchSrc, std::size_t uSrcLen) : GenericString() {
		Assign(pchSrc, uSrcLen);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	GenericString(const GenericString<OtherChar_t, OTHER_ENCODING> &rhs) : GenericString() {
		Assign(rhs);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	GenericString(GenericString<OtherChar_t, OTHER_ENCODING> &&rhs) : GenericString() {
		Assign(std::move(rhs));
	}
	GenericString(const GenericString &rhs) : GenericString() {
		Assign(rhs);
	}
	GenericString(GenericString &&rhs) noexcept : GenericString() {
		Assign(std::move(rhs));
	}
	GenericString &operator=(Char_t ch) noexcept {
		Assign(ch, 1);
		return *this;
	}
	GenericString &operator=(const Char_t *pszSrc){
		Assign(pszSrc);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	GenericString &operator=(const GenericString<OtherChar_t, OTHER_ENCODING> &rhs){
		Assign(rhs);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	GenericString &operator=(GenericString<OtherChar_t, OTHER_ENCODING> &&rhs){
		Assign(std::move(rhs));
		return *this;
	}
	GenericString &operator=(const GenericString &rhs){
		Assign(rhs);
		return *this;
	}
	GenericString &operator=(GenericString &&rhs) noexcept {
		Assign(std::move(rhs));
		return *this;
	}
	~GenericString(){
		if(xm_vStorage.chNull != Char_t()){
			delete[] xm_vStorage.pchLargeBegin;
		}
	}

private:
	void xSetEnd(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(xm_vStorage.chNull == Char_t()){
			xm_vStorage.uchSmallLength = uNewSize;
			((Char_t (&)[SIZE_MAX])xm_vStorage.achSmall)[uNewSize] = Char_t();
		} else {
			xm_vStorage.uLargeLength = uNewSize;
			xm_vStorage.pchLargeBegin[uNewSize] = Char_t();
		}
	}
	// 正值向外扩张，负值向内收缩。
	void xSlide(std::ptrdiff_t nDeltaBegin, std::ptrdiff_t nDeltaEnd, bool bSetEnd = true){
		const auto uOldLength = GetLength();
		auto pchOldBuffer = GetCStr();
		auto pchNewBuffer = pchOldBuffer;

		const auto SlideCopy = [=]() noexcept {
			std::size_t uCopyOffsetBegin;
			if(nDeltaBegin >= 0){
				uCopyOffsetBegin = 0;
			} else {
				if((std::size_t)-nDeltaBegin >= uOldLength){
					return;
				}
				uCopyOffsetBegin = nDeltaBegin;
			}

			std::size_t uCopyOffsetEnd;
			if(nDeltaEnd >= 0){
				uCopyOffsetEnd = uOldLength;
			} else {
				if((std::size_t)-nDeltaEnd >= uOldLength){
					return;
				}
				uCopyOffsetEnd = uOldLength + nDeltaEnd;
			}

			if(uCopyOffsetBegin >= uCopyOffsetEnd){
				return;
			}

			const auto pchBegin = pchOldBuffer + uCopyOffsetBegin;
			const auto pchEnd = pchOldBuffer + uCopyOffsetEnd;
			if(pchBegin == pchNewBuffer + nDeltaBegin - uCopyOffsetBegin){
				return;
			}

			if(nDeltaBegin < 0){
				std::copy(pchBegin, pchEnd, pchNewBuffer);
			} else {
				std::copy_backward(pchBegin, pchEnd, pchNewBuffer + nDeltaBegin + uCopyOffsetEnd);
			}
		};

		const std::size_t uNewSize = nDeltaBegin + uOldLength + nDeltaEnd;
		if(uNewSize > GetCapacity()){
			auto uSizeToAlloc = uNewSize + 1;
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & -0x10;
			pchNewBuffer = new Char_t[uSizeToAlloc];

			SlideCopy();

			if(xm_vStorage.chNull == Char_t()){
				xm_vStorage.chNull = Char_t() + 1;
			} else {
				delete[] pchOldBuffer;
			}

			xm_vStorage.pchLargeBegin = pchNewBuffer;
			xm_vStorage.uLargeLength = uOldLength;
			xm_vStorage.uLargeBufferSize = uSizeToAlloc;
		} else {
			SlideCopy();
		}

		if(bSetEnd){
			xSetEnd(uNewSize);
		} else {
			pchNewBuffer[uNewSize] = Char_t();
		}
	}

	// 返回值如果不是 nullptr，就使用返回值拷贝构造。
	// 否则 ucsTemp 为出参，可以移动构造。
	const UnifiedString *xUnify(UnifiedString &ucsTemp) const;
	void xDisunify(const UnifiedString &ucsTemp);
	void xDisunify(UnifiedString &&ucsTemp);

public:
	const Char_t *GetCStr() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]);
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	Char_t *GetCStr() noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]);
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	std::size_t GetLength() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return xm_vStorage.uchSmallLength;
		} else {
			return xm_vStorage.uLargeLength;
		}
	}

	const Char_t *GetData() const noexcept {
		return GetCStr();
	}
	Char_t *GetData() noexcept {
		return GetCStr();
	}
	std::size_t GetSize() const noexcept {
		return GetLength();
	}

	const Char_t *GetBegin() const noexcept {
		return GetCStr();
	}
	Char_t *GetBegin() noexcept {
		return GetCStr();
	}

	const Char_t *GetEnd() const noexcept {
		return GetCStr() + GetLength();
	}
	Char_t *GetEnd() noexcept {
		return GetCStr() + GetLength();
	}

	void Resize(std::size_t uNewSize){
		Reserve(uNewSize);
		xSetEnd(uNewSize);
	}
	void Shrink() noexcept {
		xSetEnd(StrLen(GetCStr()));
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		xSetEnd(0);
	}

	std::size_t GetCapacity() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return COUNT_OF(xm_vStorage.achSmall) - 1;
		} else {
			return xm_vStorage.uLargeBufferSize - 1;
		}
	}
	void Reserve(std::size_t uNewCapacity){
		xSlide(0, uNewCapacity - GetLength(), false);
	}

	void Swap(GenericString &rhs) noexcept {
		if(this != &rhs){
			std::swap(xm_vStorage, rhs.xm_vStorage);
		}
	}

	int Compare(const Char_t *pszComparand) const noexcept {
		return Compare(pszComparand, StrLen(pszComparand));
	}
	int Compare(const GenericString &strComparand) const noexcept {
		return Compare(strComparand.GetCStr(), strComparand.GetLength());
	}
	int Compare(const Char_t *pszComparand, std::size_t uMaxCount) const noexcept {
		return StrCmp(GetCStr(), GetLength(), pszComparand, uMaxCount);
	}
	int Compare(const GenericString &strComparand, std::size_t uMaxCount) const noexcept {
		return Compare(strComparand.GetCStr(), uMaxCount);
	}

	void Assign(Char_t ch, std::size_t uCount = 1){
		Resize(uCount);
		std::fill_n(GetCStr(), uCount, ch);
	}
	void Assign(const Char_t *pszSrc){
		if(pszSrc){
			Assign(pszSrc, StrLen(pszSrc));
		} else {
			Clear();
		}
	}
	void Assign(const Char_t *pchSrc, std::size_t uSrcLen){
		Resize(uSrcLen);
		std::copy_n(pchSrc, uSrcLen, GetCStr());
	}
	void Assign(const GenericString &rhs){
		if(this != &rhs){
			Assign(rhs.GetCStr(), rhs.GetLength());
		}
	}
	void Assign(GenericString &&rhs) noexcept {
		if(this != &rhs){
			Clear();
			Swap(rhs);
		}
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	void Assign(const GenericString<OtherChar_t, OTHER_ENCODING> &rhs){
		UnifiedString ucsTemp;
		const auto pucsUnfied = rhs.xUnify(ucsTemp);
		if(pucsUnfied){
			xDisunify(*pucsUnfied);
		} else {
			xDisunify(std::move(ucsTemp));
		}
	}

	void Append(Char_t ch, std::size_t uCount = 1){
		const auto uOldLength = GetLength();
		xSlide(0, uCount);
		std::fill_n(GetCStr() + uOldLength, uCount, ch);
	}
	void Append(const Char_t *pszSrc){
		if(pszSrc){
			Append(pszSrc, StrLen(pszSrc));
		}
	}
	void Append(const Char_t *pchSrc, std::size_t uSrcLen){
		const auto uOldLength = GetLength();
		xSlide(0, uSrcLen);
		std::copy_n(pchSrc, uSrcLen, GetCStr() + uOldLength);
	}
	void Append(const GenericString &rhs){
		if(this == &rhs){
			const auto uOldLength = GetLength();
			xSlide(0, uOldLength);
			const auto pchBegin = GetCStr();
			std::copy_n(pchBegin, uOldLength, pchBegin + uOldLength);
		} else {
			Append(rhs.GetCStr(), rhs.GetLength());
		}
	}
	void Append(GenericString &&rhs){
		if(this == &rhs){
			Append(rhs);
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Append(rhs);
		} else {
			rhs.Unshift(*this);
			Assign(std::move(rhs));
		}
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		const auto uOldLength = GetLength();
		if(uOldLength <= uCount){
			Clear();
		} else {
			xSetEnd(uOldLength - uCount);
		}
	}

	void Push(Char_t ch){
		Append(ch);
	}
	void Pop(std::size_t uCount = 1) noexcept {
		Truncate(uCount);
	}

	void Unshift(Char_t ch, std::size_t uCount = 1){
		xSlide(uCount, 0);
		std::fill_n(GetCStr(), uCount, ch);
	}
	void Unshift(const Char_t *pszSrc){
		if(pszSrc){
			Unshift(pszSrc, StrLen(pszSrc));
		}
	}
	void Unshift(const Char_t *pchSrc, std::size_t uSrcLen){
		xSlide(uSrcLen, 0);
		std::copy_n(pchSrc, uSrcLen, GetCStr());
	}
	void Unshift(const GenericString &rhs){
		if(this == &rhs){
			Append(rhs);
		} else {
			Unshift(rhs.GetCStr(), rhs.GetLength());
		}
	}
	void Unshift(GenericString &&rhs){
		if(this == &rhs){
			Append(std::move(rhs));
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Unshift(rhs);
		} else {
			rhs.Append(*this);
			Assign(std::move(rhs));
		}
	}
	void Shift(std::size_t uCount = 1) noexcept {
		const auto uOldLength = GetLength();
		if(uOldLength <= uCount){
			Clear();
		} else {
			const auto uNewLength = uOldLength - uCount;
			const auto pchBegin = GetCStr();
			std::copy_n(pchBegin + uCount, uNewLength, pchBegin);
			xSetEnd(uNewLength);
		}
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
	GenericString Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		GenericString strRet;
		const auto pchBegin = GetCStr();
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nBegin);
		const auto uRealEnd = xTranslateOffset(uLength, nEnd);
		if(uRealBegin < uRealEnd){
			strRet.Resize(uRealEnd - uRealBegin);
			std::copy(pchBegin + uRealBegin, pchBegin + uRealEnd, strRet.GetCStr());
		} else if(uRealBegin > uRealEnd){
			strRet.Resize(uRealBegin - uRealEnd);
			std::reverse_copy(pchBegin + uRealEnd, pchBegin + uRealBegin, strRet.GetCStr());
		}
		return std::move(strRet);
	}

	// 举例：
	//   FindFirstAfter("def", 3)   返回 3；
	//   FindFirstAfter("def", 4)   返回 NPOS；
	//   FindLastBefore("def", 5)   返回 NPOS；
	//   FindLastBefore("def", 6)   返回 3。
	std::size_t FindFirstAfter(const Char_t *pszToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return FindFirstAfter(pszToFind, StrLen(pszToFind), nOffsetBegin);
	}
	std::size_t FindFirstAfter(const Char_t *pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		if(uLenToFind == 0){
			return 0;
		}
		const Char_t *pchBegin = GetCStr();
		const auto uLength = GetLength();
		if(uLength < uLenToFind){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRealBegin + uLenToFind > uLength){
			return NPOS;
		}
		const auto pchPos = StrStr(pchBegin + uRealBegin, uLength - uRealBegin, pchToFind, uLenToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}
	std::size_t FindLastBefore(const Char_t *pszToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return FindLastBefore(pszToFind, StrLen(pszToFind), nOffsetEnd);
	}
	std::size_t FindLastBefore(const Char_t *pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto pchBegin = GetCStr();
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
		const auto pchPos = StrRStr(pchBegin, uRealEnd, pchToFind, uLenToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}

	// 举例：
	//   FindFirstAfter('c', 3)   返回 NPOS；
	//   FindFirstAfter('d', 3)   返回 3；
	//   FindLastBefore('c', 3)   返回 2；
	//   FindLastBefore('d', 3)   返回 NPOS。
	std::size_t FindFirstAfter(Char_t chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const Char_t *pchBegin = GetCStr();
		const auto uLength = GetLength();
		if(uLength == 0){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRealBegin + 1 > uLength){
			return NPOS;
		}
		const auto pchPos = StrChr(pchBegin + uRealBegin, uLength - uRealBegin, chToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}
	std::size_t FindLastBefore(Char_t chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto pchBegin = GetCStr();
		const auto uLength = GetLength();
		if(uLength == 0){
			return NPOS;
		}
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
		if(uRealEnd == 0){
			return NPOS;
		}
		const auto pchPos = StrRChr(pchBegin, uRealEnd, chToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}

	// 参考 Slice。
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char_t chRep, std::size_t uRepCount = 1){
		auto pchBegin = GetCStr();
		const auto uOldLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uOldLength, nBegin);
		const auto uRealEnd = xTranslateOffset(uOldLength, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uOldCount = uRealEnd - uRealBegin;
			const auto uNewLength = uOldLength - uOldCount + uRepCount;
			if(uRepCount > uOldCount){
				// xxxYYYzzz
				// xxxWWWWWzzz
				xSlide(0, uRepCount - uOldCount, false);
				pchBegin = GetCStr();
				std::copy_backward(pchBegin + uRealEnd, pchBegin + uOldLength, pchBegin + uNewLength);
			} else if(uRepCount < uOldCount){
				// xxxYYYYYzzz
				// xxxWWWzzz
				std::copy(pchBegin + uRealEnd, pchBegin + uOldLength, pchBegin + uRealBegin + uRepCount);
			}
			std::fill_n(pchBegin + uRealBegin, uRepCount, chRep);
			xSetEnd(uNewLength);
		} else if(uRealBegin > uRealEnd){
			const auto uOldCount = uRealBegin - uRealEnd;
			const auto uNewLength = uOldLength - uOldCount + uRepCount;
			if(uRepCount > uOldCount){
				// xxxYYYzzz
				// xxxWWWWWzzz
				xSlide(0, uRepCount - uOldCount, false);
				pchBegin = GetCStr();
				std::copy_backward(pchBegin + uRealBegin, pchBegin + uOldLength, pchBegin + uNewLength);
			} else if(uRepCount < uOldCount){
				// xxxYYYYYzzz
				// xxxWWWzzz
				std::copy(pchBegin + uRealBegin, pchBegin + uOldLength, pchBegin + uRealEnd + uRepCount);
			}
			std::fill_n(pchBegin + uRealEnd, uRepCount, chRep);
			xSetEnd(uNewLength);
		}
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char_t *pszRep){
		Replace(nBegin, nEnd, pszRep, StrLen(pszRep));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char_t *pchRep, std::size_t uRepLen){
		auto pchBegin = GetCStr();
		const auto uOldLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uOldLength, nBegin);
		const auto uRealEnd = xTranslateOffset(uOldLength, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uOldCount = uRealEnd - uRealBegin;
			const auto uNewLength = uOldLength - uOldCount + uRepLen;
			if(uRepLen > uOldCount){
				// xxxYYYzzz
				// xxxWWWWWzzz
				xSlide(0, uRepLen - uOldCount, false);
				pchBegin = GetCStr();
				std::copy_backward(pchBegin + uRealEnd, pchBegin + uOldLength, pchBegin + uNewLength);
			} else if(uRepLen < uOldCount){
				// xxxYYYYYzzz
				// xxxWWWzzz
				std::copy(pchBegin + uRealEnd, pchBegin + uOldLength, pchBegin + uRealBegin + uRepLen);
			}
			std::copy(pchRep, pchRep + uRepLen, pchBegin + uRealBegin);
			xSetEnd(uNewLength);
		} else if(uRealBegin > uRealEnd){
			const auto uOldCount = uRealBegin - uRealEnd;
			const auto uNewLength = uOldLength - uOldCount + uRepLen;
			if(uRepLen > uOldCount){
				// xxxYYYzzz
				// xxxWWWWWzzz
				xSlide(0, uRepLen - uOldCount, false);
				pchBegin = GetCStr();
				std::copy_backward(pchBegin + uRealBegin, pchBegin + uOldLength, pchBegin + uNewLength);
			} else if(uRepLen < uOldCount){
				// xxxYYYYYzzz
				// xxxWWWzzz
				std::copy(pchBegin + uRealBegin, pchBegin + uOldLength, pchBegin + uRealEnd + uRepLen);
			}
			std::reverse_copy(pchRep, pchRep + uRepLen, pchBegin + uRealEnd);
			xSetEnd(uNewLength);
		}
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const GenericString &strRep){
		Replace(nBegin, nEnd, strRep.GetCStr(), strRep.GetSize());
	}

	void Reverse() noexcept {
		auto pchBegin = GetBegin();
		auto pchEnd = GetEnd();
		if(pchBegin != pchEnd){
			--pchEnd;
			while(pchBegin < pchEnd){
				std::swap(*pchBegin, *pchEnd);
				++pchBegin;
				--pchEnd;
			}
		}
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const Char_t *() const noexcept {
		return GetCStr();
	}
	explicit operator const Char_t *() noexcept {
		return GetCStr();
	}
	const Char_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetCStr()[uIndex];
	}
	Char_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetCStr()[uIndex];
	}

	GenericString &operator+=(Char_t rhs){
		Append(rhs);
		return *this;
	}
	GenericString &operator+=(const Char_t *rhs){
		Append(rhs);
		return *this;
	}
	GenericString &operator+=(const GenericString &rhs){
		Append(rhs);
		return *this;
	}
	GenericString &operator+=(GenericString &&rhs){
		Append(std::move(rhs));
		return *this;
	}

	bool operator==(const Char_t *rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator==(const GenericString &rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator!=(const Char_t *rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator!=(const GenericString &rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator<(const Char_t *rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator<(const GenericString &rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator>(const Char_t *rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator>(const GenericString &rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator<=(const Char_t *rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator<=(const GenericString &rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator>=(const Char_t *rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
	bool operator>=(const GenericString &rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
};

template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, const GenericString<Char_t, ENCODING> &rhs){
	const auto uLLen = lhs.GetLength();
	const auto uRLen = rhs.GetLength();
	GenericString<Char_t, ENCODING> strRet;
	strRet.Reserve(uLLen + uRLen);
	strRet.Assign(lhs.GetCStr(), uLLen);
	strRet.Append(rhs.GetCStr(), uRLen);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(GenericString<Char_t, ENCODING> &&lhs, const GenericString<Char_t, ENCODING> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, GenericString<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(GenericString<Char_t, ENCODING> &&lhs, GenericString<Char_t, ENCODING> &&rhs){
	if(lhs.GetCapacity() >= rhs.GetCapacity()){
		lhs.Append(rhs);
		return std::move(lhs);
	} else {
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs){
	const auto uLLen = StrLen(lhs);
	const auto uRLen = rhs.GetLength();
	GenericString<Char_t, ENCODING> strRet;
	strRet.Reserve(uLLen + uRLen);
	strRet.Assign(lhs, uLLen);
	strRet.Append(rhs.GetCStr(), uRLen);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const Char_t *lhs, GenericString<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(Char_t lhs, const GenericString<Char_t, ENCODING> &rhs){
	const auto uRLen = rhs.GetLength();
	GenericString<Char_t, ENCODING> strRet;
	strRet.Reserve(1 + uRLen);
	strRet.Assign(lhs);
	strRet.Append(rhs.GetCStr(), uRLen);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(Char_t lhs, GenericString<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, const Char_t *rhs){
	const auto uLLen = lhs.GetLength();
	const auto uRLen = StrLen(rhs);
	GenericString<Char_t, ENCODING> strRet;
	strRet.Reserve(uLLen + uRLen);
	strRet.Assign(lhs.GetCStr(), uLLen);
	strRet.Append(rhs, uRLen);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(GenericString<Char_t, ENCODING> &&lhs, const Char_t *rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, Char_t rhs){
	const auto uLLen = lhs.GetLength();
	GenericString<Char_t, ENCODING> strRet;
	strRet.Reserve(uLLen + 1);
	strRet.Assign(lhs.GetCStr(), uLLen);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(GenericString<Char_t, ENCODING> &&lhs, Char_t rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}

template<typename Char_t, StringEncoding ENCODING>
bool operator==(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs == lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator!=(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs != lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator<(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs > lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator>(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs < lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator<=(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs >= lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator>=(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs <= lhs;
}

template<typename Char_t, StringEncoding ENCODING>
const Char_t *begin(const GenericString<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}
template<typename Char_t, StringEncoding ENCODING>
Char_t *begin(GenericString<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}
template<typename Char_t, StringEncoding ENCODING>
const Char_t *cbegin(const GenericString<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}

template<typename Char_t, StringEncoding ENCODING>
const Char_t *end(const GenericString<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}
template<typename Char_t, StringEncoding ENCODING>
Char_t *end(GenericString<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}
template<typename Char_t, StringEncoding ENCODING>
const Char_t *cend(const GenericString<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}

extern template class GenericString<char,		StringEncoding::UTF8>;
extern template class GenericString<char,		StringEncoding::ANSI>;
extern template class GenericString<wchar_t,	StringEncoding::UTF16>;

typedef GenericString<char,		StringEncoding::UTF8>		Utf8String;
typedef GenericString<char,		StringEncoding::ANSI>		ANSIString;
typedef GenericString<wchar_t,	StringEncoding::UTF16>		Utf16String;

}

#endif
