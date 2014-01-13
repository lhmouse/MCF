// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_HPP__
#define __MCF_STRING_HPP__

#include "../MCFCRT/MCFCRT.h"
#include <algorithm>
#include <memory>
#include <utility>
#include <iterator>
#include <type_traits>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <climits>

namespace MCF {

enum class StringEncoding {
	ENC_UTF8,
	ENC_ANSI,
	ENC_UTF16
};

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
class VVector;

typedef VVector<wchar_t, 256> UNIFIED_CHAR_SEQ;

template<typename CHAR_T, StringEncoding CHAR_ENC>
class GenericString {
	static_assert(std::is_arithmetic<CHAR_T>::value, "What do you want?");

	template<typename OTHER_C, StringEncoding OTHER_E>
	friend class GenericString;
public:
	typedef CHAR_T			*PCHAR_T,	*PSTR_T;
	typedef const CHAR_T	*PCCHAR_T,	*PCSTR_T;

	typedef typename std::make_signed<CHAR_T>::type		SCHAR_T, *PSCHAR_T;
	typedef typename std::make_unsigned<CHAR_T>::type	UCHAR_T, *PUCHAR_T;

	enum : std::size_t {
		NPOS = (std::size_t)-1
	};
private:
	static std::size_t xTranslateOffset(std::size_t uLen, std::ptrdiff_t nBegin) noexcept {
		std::ptrdiff_t nRet = nBegin;
		if(nRet < 0){
			nRet += uLen + 1;
		}
		if(nRet < 0){
			nRet = 0;
		} else if((std::size_t)nRet > uLen){
			nRet = uLen;
		}
		return (std::size_t)nRet;
	}

	static std::size_t xLen(PCSTR_T pszBegin) noexcept {
		auto pchRead = pszBegin;
		while(*pchRead != CHAR_T()){
			++pchRead;
		}
		return (std::size_t)(pchRead - pszBegin);
	}
	static int xCmp(PCSTR_T pszStr1, PCSTR_T pszStr2) noexcept {
		auto pchRead1 = pszStr1;
		auto pchRead2 = pszStr2;
		CHAR_T ch1;
		std::intptr_t nDelta;
		do {
			ch1 = *(pchRead1++);
			nDelta = (std::intptr_t)(UCHAR_T)ch1 - (std::intptr_t)(UCHAR_T)*(pchRead2++);
			if(nDelta != 0){
				return (nDelta >> (sizeof(std::intptr_t) * CHAR_BIT - 1)) | 1;
			}
		} while(ch1 != 0);
		return 0;
	}
	static int xNCmp(PCSTR_T pszStr1, PCSTR_T pszStr2, std::size_t uMaxCount) noexcept {
		auto pchRead1 = pszStr1;
		auto pchRead2 = pszStr2;
		auto uRemaining = uMaxCount;
		CHAR_T ch1;
		std::intptr_t nDelta;
		do {
			if(uRemaining == 0){
				break;
			}
			--uRemaining;
			ch1 = *(pchRead1++);
			nDelta = (std::intptr_t)(UCHAR_T)ch1 - (std::intptr_t)(UCHAR_T)*(pchRead2++);
			if(nDelta != 0){
				return (nDelta >> (sizeof(std::intptr_t) * CHAR_BIT - 1)) | 1;
			}
		} while(ch1 != 0);
		return 0;
	}

	static PCHAR_T xCopyFwd(PCHAR_T pchOut, PCCHAR_T pchBegin, std::size_t uCount) noexcept {
		// GCC 你他妈不用 memmove 会死？
		for(auto i = uCount; i != 0; --i){
			*(pchOut++) = *(pchBegin++);
		}
		return pchOut;
	}
	static PCHAR_T xCopyBwd(PCHAR_T pchOut, PCCHAR_T pchBegin, std::size_t uCount) noexcept {
		pchOut += uCount;
		pchBegin += uCount;
		for(auto i = uCount; i != 0; --i){
			*--pchOut = *--pchBegin;
		}
		return pchOut + uCount;
	}
	static PCHAR_T xRevCopy(PCHAR_T pchOut, PCCHAR_T pchBegin, std::size_t uCount) noexcept {
		pchBegin += uCount;
		for(auto i = uCount; i != 0; --i){
			*(pchOut++) = *--pchBegin;
		}
		return pchOut;
	}
	static PCHAR_T xFill(PCHAR_T pchOut, CHAR_T ch, std::size_t uCount) noexcept {
		for(auto i = uCount; i != 0; --i){
			*(pchOut++) = ch;
		}
		return pchOut;
	}

	static void xCopyZFwd(PCHAR_T pchOut, PCCHAR_T pchBegin) noexcept {
		while((*(pchOut++) = *(pchBegin++)) != CHAR_T()); // 空的。
	}
	static void xCopyZBwd(PCHAR_T pchOut, PCCHAR_T pchBegin) noexcept {
		xCopyBwd(pchOut, pchBegin, xLen(pchBegin) + 1);
	}

	template<typename ITER_T>
	static PCCHAR_T xKMPFind(ITER_T s, std::size_t slen, ITER_T w, std::size_t wlen) noexcept {
		ASSERT(wlen > 0);

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
			if(t == nullptr){
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

		t[0] = NPOS;
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

		PCCHAR_T found = nullptr;

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
				if(entry != NPOS){
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
	union {
		struct LARGE {
			PCHAR_T pchBegin;
			std::size_t uCapacity;

			void *(Padding_DoNotUse[2]);
		} Large;

		CHAR_T Small[sizeof(LARGE) / sizeof(CHAR_T)];
	} xm_Storage;

	static_assert(sizeof(xm_Storage) % sizeof(std::uintptr_t) == 0, "");
public:
	GenericString() noexcept {
		xm_Storage.Small[0] = CHAR_T();
		std::end(xm_Storage.Small)[-1] = CHAR_T();
	}
	explicit GenericString(std::size_t uInitCap) : GenericString() {
		Reserve(uInitCap);
	}
	explicit GenericString(CHAR_T ch, std::size_t uCount = 1) : GenericString() {
		Assign(ch, uCount);
	}
	GenericString(PCSTR_T pszSrc) : GenericString() {
		Assign(pszSrc);
	}
	GenericString(PCCHAR_T pchSrc, std::size_t uSrcLen) : GenericString() {
		Assign(pchSrc, uSrcLen);
	}
	GenericString(const GenericString &rhs) : GenericString() {
		Assign(rhs);
	}
	GenericString(GenericString &&rhs) noexcept : GenericString() {
		Assign(std::move(rhs));
	}
	template<typename OTHER_C, StringEncoding OTHER_E>
	GenericString(const GenericString<OTHER_C, OTHER_E> &rhs) : GenericString() {
		Assign(rhs);
	}
	template<typename OTHER_C, StringEncoding OTHER_E>
	GenericString(GenericString<OTHER_C, OTHER_E> &&rhs) noexcept : GenericString() {
		Assign(std::move(rhs));
	}
	GenericString &operator=(CHAR_T ch){
		Assign(ch, 1);
		return *this;
	}
	GenericString &operator=(PCSTR_T pszSrc){
		Assign(pszSrc);
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
	template<typename OTHER_C, StringEncoding OTHER_E>
	GenericString &operator=(const GenericString<OTHER_C, OTHER_E> &rhs){
		Assign(rhs);
		return *this;
	}
	template<typename OTHER_C, StringEncoding OTHER_E>
	GenericString &operator=(GenericString<OTHER_C, OTHER_E> &&rhs) noexcept {
		Assign(std::move(rhs));
		return *this;
	}
	~GenericString(){
		if(!xIsSmall()){
			delete[] xm_Storage.Large.pchBegin;
		}
	}
private:
	bool xIsSmall() const {
		return std::end(xm_Storage.Small)[-1] == CHAR_T();
	}

	PCHAR_T xReserveUnshift(std::size_t uCapacity, std::size_t uUnshift){
		ASSERT(uUnshift + 1 < uCapacity);

		const auto pszOldBegin = GetCStr();
		if(uCapacity <= GetCapacity()){
			xCopyZBwd(pszOldBegin + uUnshift, pszOldBegin);
			return pszOldBegin;
		} else {
			std::size_t uRealCapacity = std::max(uCapacity, COUNT_OF(xm_Storage.Small));
			if(sizeof(std::size_t) == 8){
				uRealCapacity = (std::size_t)1 << (64u - __builtin_clzll(uRealCapacity - 1));
			} else {
				uRealCapacity = (std::size_t)1 << (32u - __builtin_clzl(uRealCapacity - 1));
			}
			const PCHAR_T pchNewStor = new CHAR_T[uRealCapacity];
			xCopyZFwd(pchNewStor + uUnshift, pszOldBegin);	// 没必要用 xCopyZBwd()，因为它需要扫描两次。
			if(xIsSmall()){
				std::end(xm_Storage.Small)[-1] = (CHAR_T)-1;
			} else {
				delete[] xm_Storage.Large.pchBegin;
			}
			xm_Storage.Large.pchBegin = pchNewStor;
			xm_Storage.Large.uCapacity = uRealCapacity;
			return pchNewStor;
		}
	}

	UNIFIED_CHAR_SEQ xUnify() const;
	void xDisunify(UNIFIED_CHAR_SEQ &&ucsUnified);
public:
	PCSTR_T GetCStr() const noexcept {
		return xIsSmall() ? xm_Storage.Small : xm_Storage.Large.pchBegin;
	}
	PSTR_T GetCStr() noexcept {
		return xIsSmall() ? xm_Storage.Small : xm_Storage.Large.pchBegin;
	}
	bool IsEmpty() const noexcept {
		return GetCStr()[0] == CHAR_T();
	}
	void Clear() noexcept {
		GetCStr()[0] = CHAR_T();
	}

	std::size_t GetLength() const noexcept {
		return xLen(GetCStr());
	}
	std::size_t GetCapacity() const {
		return xIsSmall() ? COUNT_OF(xm_Storage.Small) : xm_Storage.Large.uCapacity;
	}
	PCHAR_T Reserve(std::size_t uCapacity){
		return xReserveUnshift(uCapacity, 0);
	}

	void Swap(GenericString &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		for(std::size_t i = 0; i < sizeof(xm_Storage) / sizeof(std::uintptr_t); ++i){
			std::swap(((std::uintptr_t *)&xm_Storage)[i], ((std::uintptr_t *)&rhs.xm_Storage)[i]);
		}
	}

	void Assign(CHAR_T ch, std::size_t uCount){
		xFill(Reserve(uCount + 1), ch, uCount)[0] = CHAR_T();
	}
	void Assign(PCSTR_T pszSrc){
		Assign(pszSrc, xLen(pszSrc));
	}
	void Assign(PCSTR_T pchSrc, std::size_t uSrcLen){
		xCopyFwd(Reserve(uSrcLen + 1), pchSrc, uSrcLen)[0] = CHAR_T();
	}
	void Assign(const GenericString &rhs){
		if(&rhs == this){
			return;
		}
		Assign(rhs.GetCStr(), rhs.GetLength());
	}
	void Assign(GenericString &&rhs){
		if(&rhs == this){
			return;
		}
		__builtin_memcpy(&xm_Storage, &rhs.xm_Storage, sizeof(xm_Storage));
		std::end(rhs.xm_Storage.Small)[-1] = CHAR_T();
	}
	template<typename OTHER_C, StringEncoding OTHER_E>
	void Assign(const GenericString<OTHER_C, OTHER_E> &rhs){
		xDisunify(rhs.xUnify());
	}

	void Append(CHAR_T ch, std::size_t uCount = 1){
		const std::size_t uLen = GetLength();
		xFill(Reserve(uLen + uCount + 1) + uLen, ch, uCount)[0] = CHAR_T();
	}
	void Append(PCSTR_T pszSrc){
		Append(pszSrc, xLen(pszSrc));
	}
	void Append(PCCHAR_T pchSrc, std::size_t uSrcLen){
		const std::size_t uLen = GetLength();
		xCopyFwd(Reserve(uLen + uSrcLen + 1) + uLen, pchSrc, uSrcLen)[0] = CHAR_T();
	}
	void Append(const GenericString &rhs){
		const std::size_t uLen = GetLength();
		if(&rhs == this){
			const auto pchBegin = Reserve(uLen * 2 + 1);
			*xCopyFwd(pchBegin + uLen, pchBegin, uLen) = CHAR_T();
		} else {
			Append(rhs.GetCStr(), uLen);
		}
	}
	void Append(GenericString &&rhs){
		if(&rhs == this){
			Append(rhs);
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Append(rhs);
		} else {
			rhs.Unshift(*this);
			Assign(std::move(rhs));
		}
	}
	void Truncate(std::size_t uCount) noexcept {
		const std::size_t uLen = GetLength();
		const auto pchWrite = GetCStr();
		if(uCount >= uLen){
			pchWrite[0] = CHAR_T();
		} else {
			pchWrite[uLen - uCount] = CHAR_T();
		}
	}

	void Unshift(CHAR_T ch, std::size_t uCount = 1){
		const std::size_t uLen = GetLength();
		xFill(xReserveUnshift(uLen + uCount + 1, uCount), ch, uCount);
	}
	void Unshift(PCSTR_T pszSrc){
		Unshift(pszSrc, xLen(pszSrc));
	}
	void Unshift(PCCHAR_T pchSrc, std::size_t uSrcLen){
		const std::size_t uLen = GetLength();
		xCopyFwd(xReserveUnshift(uLen + uSrcLen + 1, uSrcLen), pchSrc, uSrcLen);
	}
	void Unshift(const GenericString &rhs){
		if(&rhs == this){
			Append(rhs);
		} else {
			Unshift(rhs.GetCStr(), GetLength());
		}
	}
	void Unshift(GenericString &&rhs){
		if(&rhs == this){
			Append(rhs);
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Unshift(rhs);
		} else {
			rhs.Append(*this);
			Assign(std::move(rhs));
		}
	}
	void Shift(std::size_t uCount) noexcept {
		const std::size_t uLen = GetLength();
		const auto pchWrite = GetCStr();
		if(uCount >= uLen){
			pchWrite[0] = CHAR_T();
		} else {
			xCopyZFwd(pchWrite, pchWrite + uCount);
		}
	}

	int Compare(PCSTR_T rhs) const noexcept {
		return xCmp(GetCStr(), rhs);
	}
	int Compare(const GenericString &rhs) const noexcept {
		return xCmp(GetCStr(), rhs.GetCStr());
	}
	int Compare(PCSTR_T rhs, std::size_t uMaxCount) const noexcept {
		return xNCmp(GetCStr(), rhs, uMaxCount);
	}
	int Compare(const GenericString &rhs, std::size_t uMaxCount) const noexcept {
		return xNCmp(GetCStr(), rhs.GetCStr(), uMaxCount);
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
		const std::size_t uLen = xLen(pchBegin);
		const auto uRealBegin = xTranslateOffset(uLen, nBegin);
		const auto uRealEnd = xTranslateOffset(uLen, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uCount = uRealEnd - uRealBegin;
			xCopyFwd(strRet.Reserve(uCount + 1), pchBegin + uRealBegin, uCount)[0] = CHAR_T();
		} else if(uRealBegin > uRealEnd){
			const auto uCount = uRealBegin - uRealEnd;
			xCopyFwd(strRet.Reserve(uCount + 1), pchBegin + uRealEnd, uCount)[0] = CHAR_T();
		}
		return std::move(strRet);
	}

	// 举例：
	//   FindFirstAfter("def", 3)   返回 3；
	//   FindFirstAfter("def", 4)   返回 NPOS；
	//   FindLastBefore("def", 5)   返回 NPOS；
	//   FindLastBefore("def", 6)   返回 3。
	std::size_t FindFirstAfter(PCSTR_T pszToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return FindFirstAfter(pszToFind, xLen(pszToFind), nOffsetBegin);
	}
	std::size_t FindFirstAfter(PCCHAR_T pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		if(uLenToFind == 0){
			return 0;
		}
		const PCCHAR_T pchBegin = GetCStr();
		const std::size_t uLen = xLen(pchBegin);
		if(uLen < uLenToFind){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLen, nOffsetBegin);
		if(uRealBegin + uLenToFind > uLen){
			return NPOS;
		}
		const auto pchPos = xKMPFind(pchBegin + uRealBegin, uLen - uRealBegin, pchToFind, uLenToFind);
		return (pchPos == nullptr) ? NPOS : (std::size_t)(pchPos - pchBegin);
	}
	std::size_t FindLastBefore(PCSTR_T pszToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return FindLastBefore(pszToFind, xLen(pszToFind), nOffsetEnd);
	}
	std::size_t FindLastBefore(PCCHAR_T pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto pchBegin = GetCStr();
		const std::size_t uLen = xLen(pchBegin);
		if(uLenToFind == 0){
			return uLen;
		}
		if(uLen < uLenToFind){
			return NPOS;
		}
		const auto uRealEnd = xTranslateOffset(uLen, nOffsetEnd);
		if(uRealEnd < uLenToFind){
			return NPOS;
		}
		typedef std::reverse_iterator<PCCHAR_T> RPCHAR_T;
		const auto pchPos = xKMPFind(RPCHAR_T(pchBegin + uRealEnd), uRealEnd, RPCHAR_T(pchToFind + uLenToFind), uLenToFind);
		return (pchPos == nullptr) ? NPOS : (std::size_t)(pchPos - pchBegin);
	}

	// 举例：
	//   FindFirstAfter('c', 3)   返回 NPOS；
	//   FindFirstAfter('d', 3)   返回 3；
	//   FindLastBefore('c', 3)   返回 2；
	//   FindLastBefore('d', 3)   返回 NPOS。
	std::size_t FindFirstAfter(CHAR_T chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto pchBegin = GetCStr();
		const std::size_t uLen = xLen(pchBegin);
		if(uLen == 0){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLen, nOffsetBegin);
		if(uRealBegin == uLen){
			return NPOS;
		}
		auto pchRead = pchBegin + uRealBegin;
		const auto pchEnd = pchBegin + uLen;
		for(;;){
			if(*pchRead == chToFind){
				return (std::size_t)(pchRead - pchBegin);
			}
			if(pchRead == pchEnd){
				return NPOS;
			}
			++pchRead;
		}
	}
	std::size_t FindLastBefore(CHAR_T chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto pchBegin = GetCStr();
		const std::size_t uLen = xLen(pchBegin);
		if(uLen == 0){
			return NPOS;
		}
		const auto uRealEnd = xTranslateOffset(uLen, nOffsetEnd);
		if(uRealEnd == 0){
			return NPOS;
		}
		auto pchRead = pchBegin + uRealEnd - 1;
		for(;;){
			if(*pchRead == chToFind){
				return (std::size_t)(pchRead - pchBegin);
			}
			if(pchRead == pchBegin){
				return NPOS;
			}
			--pchRead;
		}
	}

	// 参考 Slice。
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, CHAR_T chRep, std::size_t uRepCount = 1){
		const auto pchBegin = GetCStr();
		const std::size_t uLen = xLen(pchBegin);
		const auto uRealBegin = xTranslateOffset(uLen, nBegin);
		const auto uRealEnd = xTranslateOffset(uLen, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uOldCount = uRealEnd - uRealBegin;
			if(uRepCount > uOldCount){
				const auto pRepDst = Reserve(uLen + (uRepCount - uOldCount) + 1) + uRealBegin;
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLen - uRealEnd + 1);
				xFill(pRepDst, chRep, uRepCount);
			} else if(uRepCount < uOldCount){
				const auto pRepDst = pchBegin + uRealBegin;
				xFill(pRepDst, chRep, uRepCount);
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLen - uRealEnd + 1);
			}
		} else if(uRealBegin > uRealEnd){
			const auto uOldCount = uRealBegin - uRealEnd;
			if(uRepCount > uOldCount){
				const auto pRepDst = Reserve(uLen + (uRepCount - uOldCount) + 1) + uRealEnd;
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLen - uRealBegin + 1);
				xFill(pRepDst, chRep, uRepCount);
			} else if(uRepCount < uOldCount){
				const auto pRepDst = pchBegin + uRealEnd;
				xFill(pRepDst, chRep, uRepCount);
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLen - uRealBegin + 1);
			}
		}
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCSTR_T pszRep){
		Replace(nBegin, nEnd, pszRep, xLen(pszRep));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCCHAR_T pchRep, std::size_t uRepLen){
		const auto pchBegin = GetCStr();
		const std::size_t uLen = xLen(pchBegin);
		const auto uRealBegin = xTranslateOffset(uLen, nBegin);
		const auto uRealEnd = xTranslateOffset(uLen, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uOldCount = uRealEnd - uRealBegin;
			if(uRepLen > uOldCount){
				const auto pRepDst = Reserve(uLen + (uRepLen - uOldCount) + 1) + uRealBegin;
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLen - uRealEnd + 1);
				xCopyFwd(pRepDst, pchRep, uRepLen);
			} else if(uRepLen < uOldCount){
				const auto pRepDst = pchBegin + uRealBegin;
				xCopyFwd(pRepDst, pchRep, uRepLen);
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLen - uRealEnd + 1);
			}
		} else if(uRealBegin > uRealEnd){
			const auto uOldCount = uRealBegin - uRealEnd;
			if(uRepLen > uOldCount){
				const auto pRepDst = Reserve(uLen + (uRepLen - uOldCount) + 1) + uRealEnd;
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLen - uRealBegin + 1);
				xRevCopy(pRepDst, pchRep, uRepLen);
			} else if(uRepLen < uOldCount){
				const auto pRepDst = pchBegin + uRealEnd;
				xRevCopy(pRepDst, pchRep, uRepLen);
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLen - uRealBegin + 1);
			}
		}
	}
public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator PCSTR_T() const noexcept {
		return GetCStr();
	}
	explicit operator PSTR_T() noexcept {
		return GetCStr();
	}
	const CHAR_T &operator[](std::size_t uIndex) const noexcept {
		return GetCStr()[uIndex];
	}
	CHAR_T &operator[](std::size_t uIndex) noexcept {
		return GetCStr()[uIndex];
	}

	GenericString &operator+=(CHAR_T rhs){
		Append(rhs);
		return *this;
	}
	GenericString &operator+=(PCSTR_T rhs){
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

	bool operator==(PCSTR_T rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator==(const GenericString &rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator!=(PCSTR_T rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator!=(const GenericString &rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator<(PCSTR_T rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator<(const GenericString &rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator>(PCSTR_T rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator>(const GenericString &rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator<=(PCSTR_T rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator<=(const GenericString &rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator>=(PCSTR_T rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
	bool operator>=(const GenericString &rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
};

template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	return GenericString<CHAR_T, CHAR_ENC>(lhs) += rhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	return std::move(lhs += rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	return std::move(lhs += rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	return GenericString<CHAR_T, CHAR_ENC>(lhs) + rhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const CHAR_T *lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(CHAR_T lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	return GenericString<CHAR_T, CHAR_ENC>(lhs) + rhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(CHAR_T lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, const CHAR_T *rhs){
	return GenericString<CHAR_T, CHAR_ENC>(lhs) += rhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, const CHAR_T *rhs){
	return std::move(lhs += rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, CHAR_T rhs){
	return GenericString<CHAR_T, CHAR_ENC>(lhs) += rhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, CHAR_T rhs){
	return std::move(lhs += rhs);
}

template<typename CHAR_T, StringEncoding CHAR_ENC>
bool operator==(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs) noexcept {
	return rhs == lhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
bool operator!=(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs) noexcept {
	return rhs != lhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
bool operator<(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs) noexcept {
	return rhs >= lhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
bool operator>(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs) noexcept {
	return rhs <= lhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
bool operator<=(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs) noexcept {
	return rhs > lhs;
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
bool operator>=(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs) noexcept {
	return rhs < lhs;
}

extern template class GenericString<char,		StringEncoding::ENC_UTF8>;
extern template class GenericString<char,		StringEncoding::ENC_ANSI>;
extern template class GenericString<wchar_t,	StringEncoding::ENC_UTF16>;

typedef GenericString<char,		StringEncoding::ENC_UTF8>		UTF8String;
typedef GenericString<char,		StringEncoding::ENC_ANSI>		ANSIString, NarrowString, NCString;
typedef GenericString<wchar_t,	StringEncoding::ENC_UTF16>		UTF16String, WideString, WCString;

}

#endif
