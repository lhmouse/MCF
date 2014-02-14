// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_HPP__
#define __MCF_STRING_HPP__

#include "../../MCFCRT/c/ext/assert.h"
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

namespace __MCF {
	template<typename CHAR_T>
	inline std::size_t StrLen(const CHAR_T *s) noexcept {
		auto rp = s;
		while(*rp != CHAR_T()){
			++rp;
		}
		return (std::size_t)(rp - s);
	}
	template<>
	inline std::size_t StrLen<char>(const char *s) noexcept {
		return std::strlen(s);
	}
	template<>
	inline std::size_t StrLen<wchar_t>(const wchar_t *s) noexcept {
		return std::wcslen(s);
	}

	template<typename CHAR_T>
	inline int StrCmp(const CHAR_T *s1, const CHAR_T *s2) noexcept {
		auto rp1 = s1;
		auto rp2 = s2;
		typename std::make_unsigned<CHAR_T>::type ch1, ch2;
		do {
			ch1 = *(rp1++);
			ch2 = *(rp2++);
			if(ch1 != ch2){
				return (ch1 > ch2) ? 1 : -1;
			}
		} while(ch1 != 0);
		return 0;
	}
	template<>
	inline int StrCmp<char>(const char *s1, const char *s2) noexcept {
		return std::strcmp(s1, s2);
	}
	template<>
	inline int StrCmp<wchar_t>(const wchar_t *s1, const wchar_t *s2) noexcept {
		return std::wcscmp(s1, s2);
	}

	template<typename CHAR_T>
	inline int StrNCmp(const CHAR_T *s1, const CHAR_T *s2, std::size_t cnt) noexcept {
		auto rp1 = s1;
		auto rp2 = s2;
		auto i = cnt;
		typename std::make_unsigned<CHAR_T>::type ch1, ch2;
		do {
			if(i == 0){
				break;
			}
			--i;
			ch1 = *(rp1++);
			ch2 = *(rp2++);
			if(ch1 != ch2){
				return (ch1 > ch2) ? 1 : -1;
			}
		} while(ch1 != 0);
		return 0;
	}
	template<>
	inline int StrNCmp<char>(const char *s1, const char *s2, std::size_t cnt) noexcept {
		return std::strncmp(s1, s2, cnt);
	}
	template<>
	inline int StrNCmp<wchar_t>(const wchar_t *s1, const wchar_t *s2, std::size_t cnt) noexcept {
		return std::wcsncmp(s1, s2, cnt);
	}
}

enum class StringEncoding {
	ENC_UTF8,
	ENC_ANSI,
	ENC_UTF16
};

template<typename CHAR_T, StringEncoding CHAR_ENC>
class GenericString;

typedef GenericString<wchar_t, StringEncoding::ENC_UTF16> UnifiedString;

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
	static std::size_t xTranslateOffset(std::size_t uLength, std::ptrdiff_t nBegin) noexcept {
		std::ptrdiff_t nRet = nBegin;
		if(nRet < 0){
			nRet += uLength + 1;
		}
		if(nRet < 0){
			nRet = 0;
		} else if((std::size_t)nRet > uLength){
			nRet = uLength;
		}
		return (std::size_t)nRet;
	}

	static PCHAR_T xCopyFwd(PCHAR_T pchOut, PCCHAR_T pchBegin, std::size_t uCount) noexcept {
		return std::copy(pchBegin, pchBegin + uCount, pchOut);
	}
	static PCHAR_T xCopyBwd(PCHAR_T pchOut, PCCHAR_T pchBegin, std::size_t uCount) noexcept {
		const auto pchRet = pchOut + uCount;
		std::copy_backward(pchBegin, pchBegin + uCount, pchRet);
		return pchRet;
	}
	static PCHAR_T xRevCopy(PCHAR_T pchOut, PCCHAR_T pchBegin, std::size_t uCount) noexcept {
		return std::reverse_copy(pchBegin, pchBegin + uCount, pchOut);
	}
	static PCHAR_T xFill(PCHAR_T pchOut, CHAR_T ch, std::size_t uCount) noexcept {
		return std::fill_n(pchOut, uCount, ch);
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
	struct {
		union {
			CHAR_T Small[7 * sizeof(std::size_t) / sizeof(CHAR_T)];
			struct {
				PCHAR_T pchBegin;
				std::size_t uCapacity;
			} Large;
		};
		std::size_t uLength;
	} xm_vStorage;
public:
	GenericString() noexcept {
		xm_vStorage.Small[0] = CHAR_T();
		std::end(xm_vStorage.Small)[-1] = CHAR_T();
		xm_vStorage.uLength = 0;
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
			delete[] xm_vStorage.Large.pchBegin;
		}
	}
private:
	bool xIsSmall() const {
		return std::end(xm_vStorage.Small)[-1] == CHAR_T();
	}

	PCHAR_T xReserveUnshift(std::size_t uCapacity, std::size_t uUnshift){
		ASSERT(uUnshift + GetLength() < uCapacity);

		const auto pszOldBegin = GetCStr();
		if(uCapacity <= GetCapacity()){
			if(uUnshift != 0){
				xCopyBwd(pszOldBegin + uUnshift, pszOldBegin, GetLength() + 1);
			}
			return pszOldBegin;
		} else {
			auto uRealCapacity = std::max(uCapacity, COUNT_OF(xm_vStorage.Small));
			if(sizeof(std::size_t) == 8){
				uRealCapacity = (std::size_t)1 << (64u - __builtin_clzll(uRealCapacity - 1));
			} else {
				uRealCapacity = (std::size_t)1 << (32u - __builtin_clzl(uRealCapacity - 1));
			}
			const PCHAR_T pchNewStor = new CHAR_T[uRealCapacity];
			xCopyFwd(pchNewStor + uUnshift, pszOldBegin, GetLength() + 1);
			if(xIsSmall()){
				std::end(xm_vStorage.Small)[-1] = (CHAR_T)-1;
			} else {
				delete[] xm_vStorage.Large.pchBegin;
			}
			xm_vStorage.Large.pchBegin = pchNewStor;
			xm_vStorage.Large.uCapacity = uRealCapacity;
			return pchNewStor;
		}
	}

	// 返回值可以是 ucsTemp，也可以是 *this，取决于 *this 的类型是否是 UnifiedString。
	// 如果返回 *this，调用者就不能修改这个返回值（例如作为 std::move() 的参数）。
	UnifiedString &xUnify(UnifiedString &ucsTemp) const;
	void xDisunify(const UnifiedString &ucsTemp);
	void xDisunify(UnifiedString &&ucsTemp);
public:
	PCSTR_T GetCStr() const noexcept {
		return xIsSmall() ? xm_vStorage.Small : xm_vStorage.Large.pchBegin;
	}
	PSTR_T GetCStr() noexcept {
		return xIsSmall() ? xm_vStorage.Small : xm_vStorage.Large.pchBegin;
	}
	bool IsEmpty() const noexcept {
		return GetCStr()[0] == CHAR_T();
	}
	void Clear() noexcept {
		GetCStr()[0] = CHAR_T();
	}

	std::size_t GetLength() const noexcept {
		return xm_vStorage.uLength;
	}
	PCHAR_T Resize(std::size_t uSize){
		const auto pchNewBegin = Reserve(uSize);
		pchNewBegin[uSize] = CHAR_T();
		xm_vStorage.uLength = uSize;
		return pchNewBegin;
	}
	void Trim() noexcept {
		xm_vStorage.uLength = __MCF::StrLen(GetCStr());
	}

	std::size_t GetCapacity() const {
		return xIsSmall() ? COUNT_OF(xm_vStorage.Small) : xm_vStorage.Large.uCapacity;
	}
	PCHAR_T Reserve(std::size_t uCapacity){
		return xReserveUnshift(uCapacity + 1, 0);
	}

	void Swap(GenericString &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		typedef std::uintptr_t BLOCK[sizeof(xm_vStorage) / sizeof(std::uintptr_t)];
		std::swap((BLOCK &)xm_vStorage, (BLOCK &)rhs.xm_vStorage);
	}

	void Assign(CHAR_T ch, std::size_t uCount){
		xFill(Reserve(uCount), ch, uCount)[0] = CHAR_T();
		xm_vStorage.uLength = uCount;
	}
	void Assign(PCSTR_T pszSrc){
		Assign(pszSrc, __MCF::StrLen(pszSrc));
	}
	void Assign(PCSTR_T pchSrc, std::size_t uSrcLen){
		xCopyFwd(Reserve(uSrcLen), pchSrc, uSrcLen)[0] = CHAR_T();
		xm_vStorage.uLength = uSrcLen;
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

		__builtin_memcpy(&xm_vStorage, &rhs.xm_vStorage, sizeof(xm_vStorage));

		std::begin(rhs.xm_vStorage.Small)[0] = CHAR_T();
		std::end(rhs.xm_vStorage.Small)[-1] = CHAR_T();
		rhs.xm_vStorage.uLength = 0;
	}
	template<typename OTHER_C, StringEncoding OTHER_E>
	void Assign(const GenericString<OTHER_C, OTHER_E> &rhs){
		UnifiedString ucsTemp;
		auto &ucsUnfied = rhs.xUnify(ucsTemp);
		if((const void *)&ucsUnfied == (const void *)&rhs){
			xDisunify(rhs);
		} else {
			xDisunify(std::move(ucsUnfied));
		}
	}

	void Append(CHAR_T ch, std::size_t uCount = 1){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uCount;
		xFill(Reserve(uNewLength) + uLength, ch, uCount)[0] = CHAR_T();
		xm_vStorage.uLength = uNewLength;
	}
	void Append(PCSTR_T pszSrc){
		Append(pszSrc, __MCF::StrLen(pszSrc));
	}
	void Append(PCCHAR_T pchSrc, std::size_t uSrcLen){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uSrcLen;
		xCopyFwd(Reserve(uNewLength) + uLength, pchSrc, uSrcLen)[0] = CHAR_T();
		xm_vStorage.uLength = uNewLength;
	}
	void Append(const GenericString &rhs){
		const auto uLength = GetLength();
		if(&rhs == this){
			const auto uNewLength = uLength * 2;
			const auto pchBegin = Reserve(uNewLength);
			xCopyFwd(pchBegin + uLength, pchBegin, uLength)[0] = CHAR_T();
			xm_vStorage.uLength = uNewLength;
		} else {
			Append(rhs.GetCStr(), rhs.GetLength());
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
	void Pop(std::size_t uCount = 1) noexcept {
		const auto uLength = GetLength();
		const auto pchWrite = GetCStr();
		const auto uNewLength = (uCount >= uLength) ? 0 : (uLength - uCount);
		pchWrite[uNewLength] = CHAR_T();
		xm_vStorage.uLength = uNewLength;
	}

	void Unshift(CHAR_T ch, std::size_t uCount = 1){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uCount;
		xFill(xReserveUnshift(uNewLength + 1, uCount), ch, uCount);
		xm_vStorage.uLength = uNewLength;
	}
	void Unshift(PCSTR_T pszSrc){
		Unshift(pszSrc, __MCF::StrLen(pszSrc));
	}
	void Unshift(PCCHAR_T pchSrc, std::size_t uSrcLen){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uSrcLen;
		xCopyFwd(xReserveUnshift(uNewLength + 1, uSrcLen), pchSrc, uSrcLen);
		xm_vStorage.uLength = uNewLength;
	}
	void Unshift(const GenericString &rhs){
		if(&rhs == this){
			Append(rhs);
		} else {
			Unshift(rhs.GetCStr(), rhs.GetLength());
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
	void Shift(std::size_t uCount = 1) noexcept {
		const auto uLength = GetLength();
		const auto pchWrite = GetCStr();
		if(uCount >= uLength){
			pchWrite[0] = CHAR_T();
			xm_vStorage.uLength = 0;
		} else {
			const auto uNewLength = xm_vStorage.uLength - uCount;
			xCopyFwd(pchWrite, pchWrite + uCount, uNewLength);
			pchWrite[uNewLength] = CHAR_T();
			xm_vStorage.uLength = uNewLength;
		}
	}

	int Compare(PCSTR_T rhs) const noexcept {
		return __MCF::StrCmp(GetCStr(), rhs);
	}
	int Compare(const GenericString &rhs) const noexcept {
		return __MCF::StrCmp(GetCStr(), rhs.GetCStr());
	}
	int Compare(PCSTR_T rhs, std::size_t uMaxCount) const noexcept {
		return __MCF::StrNCmp(GetCStr(), rhs, uMaxCount);
	}
	int Compare(const GenericString &rhs, std::size_t uMaxCount) const noexcept {
		return __MCF::StrNCmp(GetCStr(), rhs.GetCStr(), uMaxCount);
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
			const auto uCount = uRealEnd - uRealBegin;
			xCopyFwd(strRet.Reserve(uCount), pchBegin + uRealBegin, uCount)[0] = CHAR_T();
		} else if(uRealBegin > uRealEnd){
			const auto uCount = uRealBegin - uRealEnd;
			xCopyFwd(strRet.Reserve(uCount), pchBegin + uRealEnd, uCount)[0] = CHAR_T();
		}
		return std::move(strRet);
	}

	// 举例：
	//   FindFirstAfter("def", 3)   返回 3；
	//   FindFirstAfter("def", 4)   返回 NPOS；
	//   FindLastBefore("def", 5)   返回 NPOS；
	//   FindLastBefore("def", 6)   返回 3。
	std::size_t FindFirstAfter(PCSTR_T pszToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return FindFirstAfter(pszToFind, __MCF::StrLen(pszToFind), nOffsetBegin);
	}
	std::size_t FindFirstAfter(PCCHAR_T pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		if(uLenToFind == 0){
			return 0;
		}
		const PCCHAR_T pchBegin = GetCStr();
		const auto uLength = GetLength();
		if(uLength < uLenToFind){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRealBegin + uLenToFind > uLength){
			return NPOS;
		}
		const auto pchPos = xKMPFind(pchBegin + uRealBegin, uLength - uRealBegin, pchToFind, uLenToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}
	std::size_t FindLastBefore(PCSTR_T pszToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return FindLastBefore(pszToFind, __MCF::StrLen(pszToFind), nOffsetEnd);
	}
	std::size_t FindLastBefore(PCCHAR_T pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
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
		typedef std::reverse_iterator<PCCHAR_T> RPCHAR_T;
		const auto pchPos = xKMPFind(RPCHAR_T(pchBegin + uRealEnd), uRealEnd, RPCHAR_T(pchToFind + uLenToFind), uLenToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}

	// 举例：
	//   FindFirstAfter('c', 3)   返回 NPOS；
	//   FindFirstAfter('d', 3)   返回 3；
	//   FindLastBefore('c', 3)   返回 2；
	//   FindLastBefore('d', 3)   返回 NPOS。
	std::size_t FindFirstAfter(CHAR_T chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto pchBegin = GetCStr();
		const auto uLength = GetLength();
		if(uLength == 0){
			return NPOS;
		}
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRealBegin == uLength){
			return NPOS;
		}
		auto pchRead = pchBegin + uRealBegin;
		const auto pchEnd = pchBegin + uLength;
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
		const auto uLength = GetLength();
		if(uLength == 0){
			return NPOS;
		}
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
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
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nBegin);
		const auto uRealEnd = xTranslateOffset(uLength, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uOldCount = uRealEnd - uRealBegin;
			if(uRepCount > uOldCount){
				const auto pRepDst = Reserve(uLength + (uRepCount - uOldCount)) + uRealBegin;
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLength - uRealEnd + 1);
				xFill(pRepDst, chRep, uRepCount);
			} else if(uRepCount < uOldCount){
				const auto pRepDst = pchBegin + uRealBegin;
				xFill(pRepDst, chRep, uRepCount);
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLength - uRealEnd + 1);
			}
		} else if(uRealBegin > uRealEnd){
			const auto uOldCount = uRealBegin - uRealEnd;
			if(uRepCount > uOldCount){
				const auto pRepDst = Reserve(uLength + (uRepCount - uOldCount)) + uRealEnd;
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLength - uRealBegin + 1);
				xFill(pRepDst, chRep, uRepCount);
			} else if(uRepCount < uOldCount){
				const auto pRepDst = pchBegin + uRealEnd;
				xFill(pRepDst, chRep, uRepCount);
				xCopyBwd(pRepDst + uRepCount, pRepDst + uOldCount, uLength - uRealBegin + 1);
			}
		}
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCSTR_T pszRep){
		Replace(nBegin, nEnd, pszRep, __MCF::StrLen(pszRep));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCCHAR_T pchRep, std::size_t uRepLen){
		const auto pchBegin = GetCStr();
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nBegin);
		const auto uRealEnd = xTranslateOffset(uLength, nEnd);
		if(uRealBegin < uRealEnd){
			const auto uOldCount = uRealEnd - uRealBegin;
			if(uRepLen > uOldCount){
				const auto pRepDst = Reserve(uLength + (uRepLen - uOldCount)) + uRealBegin;
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLength - uRealEnd + 1);
				xCopyFwd(pRepDst, pchRep, uRepLen);
			} else if(uRepLen < uOldCount){
				const auto pRepDst = pchBegin + uRealBegin;
				xCopyFwd(pRepDst, pchRep, uRepLen);
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLength - uRealEnd + 1);
			}
		} else if(uRealBegin > uRealEnd){
			const auto uOldCount = uRealBegin - uRealEnd;
			if(uRepLen > uOldCount){
				const auto pRepDst = Reserve(uLength + (uRepLen - uOldCount)) + uRealEnd;
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLength - uRealBegin + 1);
				xRevCopy(pRepDst, pchRep, uRepLen);
			} else if(uRepLen < uOldCount){
				const auto pRepDst = pchBegin + uRealEnd;
				xRevCopy(pRepDst, pchRep, uRepLen);
				xCopyBwd(pRepDst + uRepLen, pRepDst + uOldCount, uLength - uRealBegin + 1);
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
		ASSERT_MSG(uIndex <= GetLength(), L"GenericString::operator[]() 失败：索引越界。");

		return GetCStr()[uIndex];
	}
	CHAR_T &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"GenericString::operator[]() 失败：索引越界。");

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
	GenericString<CHAR_T, CHAR_ENC> tmp;
	tmp.Reserve(lhs.GetLength() + rhs.GetLength());
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	if(lhs.GetCapacity() >= rhs.GetCapacity()){
		lhs.Append(rhs);
		return std::move(lhs);
	} else {
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const CHAR_T *lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	auto p = lhs;
	while(*p != CHAR_T()){
		++p;
	}
	GenericString<CHAR_T, CHAR_ENC> tmp;
	tmp.Reserve((std::size_t)(p - lhs) + rhs.GetLength());
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const CHAR_T *lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(CHAR_T lhs, const GenericString<CHAR_T, CHAR_ENC> &rhs){
	GenericString<CHAR_T, CHAR_ENC> tmp;
	tmp.Reserve(1 + rhs.GetLength());
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(CHAR_T lhs, GenericString<CHAR_T, CHAR_ENC> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, const CHAR_T *rhs){
	auto p = rhs;
	while(*p != CHAR_T()){
		++p;
	}
	GenericString<CHAR_T, CHAR_ENC> tmp;
	tmp.Reserve(lhs.GetLength(), (std::size_t)(p - rhs));
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, const CHAR_T *rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(const GenericString<CHAR_T, CHAR_ENC> &lhs, CHAR_T rhs){
	auto tmp(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename CHAR_T, StringEncoding CHAR_ENC>
GenericString<CHAR_T, CHAR_ENC> operator+(GenericString<CHAR_T, CHAR_ENC> &&lhs, CHAR_T rhs){
	lhs.Append(rhs);
	return std::move(lhs);
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
typedef GenericString<char,		StringEncoding::ENC_ANSI>		ANSIString;
typedef GenericString<wchar_t,	StringEncoding::ENC_UTF16>		UTF16String;

}

#endif
