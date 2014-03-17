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
	static_assert(std::is_arithmetic<Char_t>::value, "Char_t muse be an arithmetic type.");

	template<typename C, StringEncoding E>
	friend class GenericString;

public:
	typedef Char_t			*PChar_t,	*PStr_t;
	typedef const Char_t	*PCChar_t,	*PCStr_t;

	typedef typename std::make_signed<Char_t>::type		SChar_t, *PSChar_t;
	typedef typename std::make_unsigned<Char_t>::type	UChar_t, *PUChar_t;

	enum : std::size_t {
		NPOS = (std::size_t)-1
	};

private:
	static std::size_t xLen(const Char_t *s) noexcept {
		auto rp = s;
		while(*rp != Char_t()){
			++rp;
		}
		return (std::size_t)(rp - s);
	}

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

	static PChar_t xCopyFwd(PChar_t pchOut, PCChar_t pchBegin, std::size_t uCount) noexcept {
		return std::copy(pchBegin, pchBegin + uCount, pchOut);
	}
	static PChar_t xCopyBwd(PChar_t pchOut, PCChar_t pchBegin, std::size_t uCount) noexcept {
		const auto pchRet = pchOut + uCount;
		std::copy_backward(pchBegin, pchBegin + uCount, pchRet);
		return pchRet;
	}
	static PChar_t xRevCopy(PChar_t pchOut, PCChar_t pchBegin, std::size_t uCount) noexcept {
		return std::reverse_copy(pchBegin, pchBegin + uCount, pchOut);
	}
	static PChar_t xFill(PChar_t pchOut, Char_t ch, std::size_t uCount) noexcept {
		return std::fill_n(pchOut, uCount, ch);
	}

	template<typename ITER_T>
	static PCChar_t xKMPFind(ITER_T s, std::size_t slen, ITER_T w, std::size_t wlen) noexcept {
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

		PCChar_t found = nullptr;

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
			Char_t Small[7 * sizeof(std::size_t) / sizeof(Char_t)];
			struct {
				PChar_t pchBegin;
				std::size_t uCapacity;
			} Large;
		};
		std::size_t uLength;
	} xm_vStorage;

public:
	GenericString() noexcept {
		xm_vStorage.Small[0] = Char_t();
		std::end(xm_vStorage.Small)[-1] = Char_t();
		xm_vStorage.uLength = 0;
	}
	explicit GenericString(Char_t ch, std::size_t uCount = 1) : GenericString() {
		Assign(ch, uCount);
	}
	GenericString(PCStr_t pszSrc) : GenericString() {
		Assign(pszSrc);
	}
	GenericString(PCChar_t pchSrc, std::size_t uSrcLen) : GenericString() {
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
	GenericString &operator=(Char_t ch){
		Assign(ch, 1);
		return *this;
	}
	GenericString &operator=(PCStr_t pszSrc){
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
		return std::end(xm_vStorage.Small)[-1] == Char_t();
	}

	PChar_t xReserveUnshift(std::size_t uCapacity, std::size_t uUnshift){
		ASSERT(uUnshift + GetLength() < uCapacity);

		const auto pszOldBegin = GetCStr();
		if(uCapacity <= GetCapacity()){
			if(uUnshift != 0){
				xCopyBwd(pszOldBegin + uUnshift, pszOldBegin, GetLength() + 1);
			}
			return pszOldBegin;
		} else {
			auto uRealCapacity = std::max(uCapacity, COUNT_OF(xm_vStorage.Small));
			uRealCapacity += (uRealCapacity >> 1);
			uRealCapacity = (uRealCapacity + 0xF) & -0x10;

 			const PChar_t pchNewStor = new Char_t[uRealCapacity];
			xCopyFwd(pchNewStor + uUnshift, pszOldBegin, GetLength() + 1);
			if(xIsSmall()){
				std::end(xm_vStorage.Small)[-1] = (Char_t)-1;
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
	PCStr_t GetCStr() const noexcept {
		return xIsSmall() ? xm_vStorage.Small : xm_vStorage.Large.pchBegin;
	}
	PStr_t GetCStr() noexcept {
		return xIsSmall() ? xm_vStorage.Small : xm_vStorage.Large.pchBegin;
	}
	bool IsEmpty() const noexcept {
		return GetCStr()[0] == Char_t();
	}
	void Clear() noexcept {
		GetCStr()[0] = Char_t();
	}

	std::size_t GetLength() const noexcept {
		return xm_vStorage.uLength;
	}
	PChar_t Resize(std::size_t uSize){
		const auto pchNewBegin = Reserve(uSize);
		pchNewBegin[uSize] = Char_t();
		xm_vStorage.uLength = uSize;
		return pchNewBegin;
	}
	void Trim() noexcept {
		xm_vStorage.uLength = xLen(GetCStr());
	}

	std::size_t GetCapacity() const {
		return xIsSmall() ? COUNT_OF(xm_vStorage.Small) : xm_vStorage.Large.uCapacity;
	}
	PChar_t Reserve(std::size_t uCapacity){
		return xReserveUnshift(uCapacity + 1, 0);
	}

	void Swap(GenericString &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		typedef std::uintptr_t Block[sizeof(xm_vStorage) / sizeof(std::uintptr_t)];
		std::swap((Block &)xm_vStorage, (Block &)rhs.xm_vStorage);
	}

	void Assign(Char_t ch, std::size_t uCount){
		xFill(Reserve(uCount), ch, uCount)[0] = Char_t();
		xm_vStorage.uLength = uCount;
	}
	void Assign(PCStr_t pszSrc){
		Assign(pszSrc, xLen(pszSrc));
	}
	void Assign(PCStr_t pchSrc, std::size_t uSrcLen){
		xCopyFwd(Reserve(uSrcLen), pchSrc, uSrcLen)[0] = Char_t();
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

		std::begin(rhs.xm_vStorage.Small)[0] = Char_t();
		std::end(rhs.xm_vStorage.Small)[-1] = Char_t();
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

	void Append(Char_t ch, std::size_t uCount = 1){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uCount;
		xFill(Reserve(uNewLength) + uLength, ch, uCount)[0] = Char_t();
		xm_vStorage.uLength = uNewLength;
	}
	void Append(PCStr_t pszSrc){
		Append(pszSrc, xLen(pszSrc));
	}
	void Append(PCChar_t pchSrc, std::size_t uSrcLen){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uSrcLen;
		xCopyFwd(Reserve(uNewLength) + uLength, pchSrc, uSrcLen)[0] = Char_t();
		xm_vStorage.uLength = uNewLength;
	}
	void Append(const GenericString &rhs){
		const auto uLength = GetLength();
		if(&rhs == this){
			const auto uNewLength = uLength * 2;
			const auto pchBegin = Reserve(uNewLength);
			xCopyFwd(pchBegin + uLength, pchBegin, uLength)[0] = Char_t();
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
		pchWrite[uNewLength] = Char_t();
		xm_vStorage.uLength = uNewLength;
	}

	void Unshift(Char_t ch, std::size_t uCount = 1){
		const auto uLength = GetLength();
		const auto uNewLength = uLength + uCount;
		xFill(xReserveUnshift(uNewLength + 1, uCount), ch, uCount);
		xm_vStorage.uLength = uNewLength;
	}
	void Unshift(PCStr_t pszSrc){
		Unshift(pszSrc, xLen(pszSrc));
	}
	void Unshift(PCChar_t pchSrc, std::size_t uSrcLen){
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
			pchWrite[0] = Char_t();
			xm_vStorage.uLength = 0;
		} else {
			const auto uNewLength = xm_vStorage.uLength - uCount;
			xCopyFwd(pchWrite, pchWrite + uCount, uNewLength);
			pchWrite[uNewLength] = Char_t();
			xm_vStorage.uLength = uNewLength;
		}
	}

	int Compare(PCStr_t rhs) const noexcept {
		return Compare(rhs, xLen(rhs));
	}
	int Compare(const GenericString &rhs) const noexcept {
		return Compare(rhs.GetCStr(), rhs.GetLength());
	}
	int Compare(PCStr_t rhs, std::size_t uMaxCount) const noexcept {
		auto pLRead = GetCStr();
		auto uLLen = GetLength();
		auto pRRead = rhs;
		auto uRLen = uMaxCount;
		for(;;){
			if(uLLen == 0){
				return (uRLen == 0) ? 0 : -1;
			} else if(uRLen == 0){
				return 1;
			}

			const auto chL = (UChar_t)*(pLRead++);
			const auto chR = (UChar_t)*(pRRead++);
			if(chL != chR){
				return (chL < chR) ? -1 : 1;
			}
		}
	}
	int Compare(const GenericString &rhs, std::size_t uMaxCount) const noexcept {
		return Compare(rhs.GetCStr(), uMaxCount);
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
			xCopyFwd(strRet.Reserve(uCount), pchBegin + uRealBegin, uCount)[0] = Char_t();
		} else if(uRealBegin > uRealEnd){
			const auto uCount = uRealBegin - uRealEnd;
			xCopyFwd(strRet.Reserve(uCount), pchBegin + uRealEnd, uCount)[0] = Char_t();
		}
		return std::move(strRet);
	}

	// 举例：
	//   FindFirstAfter("def", 3)   返回 3；
	//   FindFirstAfter("def", 4)   返回 NPOS；
	//   FindLastBefore("def", 5)   返回 NPOS；
	//   FindLastBefore("def", 6)   返回 3。
	std::size_t FindFirstAfter(PCStr_t pszToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return FindFirstAfter(pszToFind, xLen(pszToFind), nOffsetBegin);
	}
	std::size_t FindFirstAfter(PCChar_t pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		if(uLenToFind == 0){
			return 0;
		}
		const PCChar_t pchBegin = GetCStr();
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
	std::size_t FindLastBefore(PCStr_t pszToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return FindLastBefore(pszToFind, xLen(pszToFind), nOffsetEnd);
	}
	std::size_t FindLastBefore(PCChar_t pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
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
		typedef std::reverse_iterator<PCChar_t> RPChar_t;
		const auto pchPos = xKMPFind(RPChar_t(pchBegin + uRealEnd), uRealEnd, RPChar_t(pchToFind + uLenToFind), uLenToFind);
		return pchPos ? (std::size_t)(pchPos - pchBegin) : NPOS;
	}

	// 举例：
	//   FindFirstAfter('c', 3)   返回 NPOS；
	//   FindFirstAfter('d', 3)   返回 3；
	//   FindLastBefore('c', 3)   返回 2；
	//   FindLastBefore('d', 3)   返回 NPOS。
	std::size_t FindFirstAfter(Char_t chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
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
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char_t chRep, std::size_t uRepCount = 1){
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
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCStr_t pszRep){
		Replace(nBegin, nEnd, pszRep, xLen(pszRep));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCChar_t pchRep, std::size_t uRepLen){
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
	explicit operator PCStr_t() const noexcept {
		return GetCStr();
	}
	explicit operator PStr_t() noexcept {
		return GetCStr();
	}
	const Char_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"GenericString::operator[]() 失败：索引越界。");

		return GetCStr()[uIndex];
	}
	Char_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"GenericString::operator[]() 失败：索引越界。");

		return GetCStr()[uIndex];
	}

	GenericString &operator+=(Char_t rhs){
		Append(rhs);
		return *this;
	}
	GenericString &operator+=(PCStr_t rhs){
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

	bool operator==(PCStr_t rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator==(const GenericString &rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator!=(PCStr_t rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator!=(const GenericString &rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator<(PCStr_t rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator<(const GenericString &rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator>(PCStr_t rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator>(const GenericString &rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator<=(PCStr_t rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator<=(const GenericString &rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator>=(PCStr_t rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
	bool operator>=(const GenericString &rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
};

template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, const GenericString<Char_t, ENCODING> &rhs){
	GenericString<Char_t, ENCODING> tmp;
	tmp.Reserve(lhs.GetLength() + rhs.GetLength());
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
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
	auto p = lhs;
	while(*p != Char_t()){
		++p;
	}
	GenericString<Char_t, ENCODING> tmp;
	tmp.Reserve((std::size_t)(p - lhs) + rhs.GetLength());
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const Char_t *lhs, GenericString<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(Char_t lhs, const GenericString<Char_t, ENCODING> &rhs){
	GenericString<Char_t, ENCODING> tmp;
	tmp.Reserve(1 + rhs.GetLength());
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(Char_t lhs, GenericString<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, const Char_t *rhs){
	auto p = rhs;
	while(*p != Char_t()){
		++p;
	}
	GenericString<Char_t, ENCODING> tmp;
	tmp.Reserve(lhs.GetLength(), (std::size_t)(p - rhs));
	tmp.Assign(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(GenericString<Char_t, ENCODING> &&lhs, const Char_t *rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char_t, StringEncoding ENCODING>
GenericString<Char_t, ENCODING> operator+(const GenericString<Char_t, ENCODING> &lhs, Char_t rhs){
	auto tmp(lhs);
	tmp.Append(rhs);
	return std::move(tmp);
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
	return rhs >= lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator>(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs <= lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator<=(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs > lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator>=(const Char_t *lhs, const GenericString<Char_t, ENCODING> &rhs) noexcept {
	return rhs < lhs;
}

extern template class GenericString<char,		StringEncoding::UTF8>;
extern template class GenericString<char,		StringEncoding::ANSI>;
extern template class GenericString<wchar_t,	StringEncoding::UTF16>;

typedef GenericString<char,		StringEncoding::UTF8>		UTF8String;
typedef GenericString<char,		StringEncoding::ANSI>		ANSIString;
typedef GenericString<wchar_t,	StringEncoding::UTF16>		UTF16String;

}

#endif
