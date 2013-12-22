// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_HPP__
#define __MCF_STRING_HPP__

#include <algorithm>
#include <memory>
#include <utility>
#include <iterator>
#include <type_traits>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace MCF {
	namespace String {
		typedef enum {
			ENC_UTF8,
			ENC_ANSI,
			ENC_UTF16
		} ENCODING;
	}

	typedef VVector<wchar_t, 256> UNIFIED_CHAR_SEQ;

	template<typename C_PARAM, String::ENCODING E_PARAM>
	class GenericString {
		template<typename OTHER_C, String::ENCODING OTHER_E>
		friend class GenericString;
	public:
		typedef C_PARAM CHAR_T;
		enum {
			CHAR_ENC = E_PARAM
		};

		typedef CHAR_T			*PCHAR_T,	*PSTR_T;
		typedef const CHAR_T	*PCCHAR_T,	*PCSTR_T;

		typedef typename std::make_signed<CHAR_T>::type		SCHAR_T, *PSCHAR_T;
		typedef typename std::make_unsigned<CHAR_T>::type	UCHAR_T, *PUCHAR_T;

		enum : std::size_t {
			NPOS = (std::size_t)-1
		};
	private:
		static std::size_t xStrLen(PCSTR_T pszStr) noexcept {
			PCCHAR_T pchEnd = pszStr;
			while(*pchEnd != CHAR_T()){
				++pchEnd;
			}
			return (std::size_t)(pchEnd - pszStr);
		}
		static int xStrCmp(PCSTR_T pszStr1, PCSTR_T pszStr2) noexcept {
			auto s1 = pszStr1;
			auto s2 = pszStr2;
			CHAR_T ch1, ch2;
			do {
				ch1 = *(s1++);
				ch2 = *(s2++);
				if(ch1 != ch2){
					return ((UCHAR_T)ch1 < (UCHAR_T)ch2) ? -1 : 1;
				}
			} while(ch1 != 0);
			return 0;
		}
		static int xStrNCmp(PCSTR_T pszStr1, PCSTR_T pszStr2, std::size_t uMaxCount) noexcept {
			auto s1 = pszStr1;
			auto s2 = pszStr2;
			auto n = uMaxCount;
			if(n != 0){
				CHAR_T ch1, ch2;
				do {
					ch1 = *(s1++);
					ch2 = *(s2++);
					if(ch1 != ch2){
						return ((UCHAR_T)ch1 < (UCHAR_T)ch2) ? -1 : 1;
					}
				} while((--n != 0) && (ch1 != 0));
			}
			return 0;
		}

		static std::size_t xTranslateOffset(std::size_t uLen, std::ptrdiff_t nBegin) noexcept {
			std::ptrdiff_t nRet = nBegin;
			const auto nLength = (std::ptrdiff_t)uLen;
			if(nRet < 0){
				nRet += nLength + 1;
			}
			if(nRet < 0){
				nRet = 0;
			} else if(nRet > nLength){
				nRet = nLength;
			}
			return (std::size_t)nRet;
		}

		static PCHAR_T xCopyN(PCHAR_T pchOut, PCCHAR_T pchIn, std::size_t uCount) noexcept {
			// GCC 你他妈不用 memmove 会死？
			// return std::copy(pchIn, pchIn + uCount, pchOut);
			auto wp = pchOut;
			const auto wend = pchOut + uCount;
			auto rp = pchIn;
			while(wp != wend){
				*(wp++) = *(rp++);
			}
			return wp;
		}
		static void xCopyNBwd(PCHAR_T pchOut, PCCHAR_T pchIn, std::size_t uCount) noexcept {
			// std::copy_backward(pchIn, pchIn + uCount, pchOut + uCount);
			auto wp = pchOut + uCount;
			const auto wbegin = pchOut;
			auto rp = pchIn + uCount;
			while(wp != wbegin){
				*(--wp) = *(--rp);
			}
		}
		static void xRevCopyN(PCHAR_T pchOut, PCCHAR_T pchIn, std::size_t uCount) noexcept {
			std::reverse_copy(pchIn, pchIn + uCount, pchOut);
		}
		static void xFillN(PCHAR_T pchOut, CHAR_T ch, std::size_t uCount) noexcept {
			std::fill(pchOut, pchOut + uCount, ch);
		}

		static PCHAR_T xCopyZFwd(PCHAR_T pchOut, PCCHAR_T pchIn) noexcept {
			auto wp = pchOut;
			auto rp = pchIn;
			CHAR_T ch;
			do {
				ch = *(rp++);
				*(wp++) = ch;
			} while(ch != CHAR_T());
			return wp;
		}
		static void xCopyZBwd(PCHAR_T pchOut, PCCHAR_T pchIn) noexcept {
			xCopyNBwd(pchOut, pchIn, xStrLen(pchIn) + 1);
		}

		template<typename ITER_T>
		static std::size_t xKMPStrStr(ITER_T s, std::size_t slen, ITER_T w, std::size_t wlen) noexcept {
			assert(wlen > 0);

			if(slen < wlen){
				return NPOS;
			}

			// http://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm
			std::size_t t_sm[64];
			std::size_t *t;
			if(wlen <= COUNTOF(t_sm)){
				t = t_sm;
			} else {
				t = new(std::nothrow) std::size_t[wlen];
				if(t == nullptr){
					// 内存不足，使用暴力搜索方法。
					const auto till = s  + (slen - wlen);
					auto p = s;
					do {
						if(std::equal(p, p + wlen, w)){
							return (std::size_t)(p - s);
						}
					} while(++p != till);
					return NPOS;
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
		CHAR_T xm_achShort[16];

		std::unique_ptr<CHAR_T[]> xm_pchLong;
		std::size_t xm_uLongCap;

		PCHAR_T xm_pchFront;
	public:
		GenericString() noexcept {
			xm_achShort[0]	= CHAR_T();

			xm_pchLong		= nullptr;
			xm_uLongCap		= 0;

			xm_pchFront		= xm_achShort;
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
		template<typename OTHER_C, String::ENCODING OTHER_E>
		GenericString(const GenericString<OTHER_C, OTHER_E> &rhs) : GenericString() {
			Assign(rhs);
		}
		template<typename OTHER_C, String::ENCODING OTHER_E>
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
		template<typename OTHER_C, String::ENCODING OTHER_E>
		GenericString &operator=(const GenericString<OTHER_C, OTHER_E> &rhs){
			Assign(rhs);
			return *this;
		}
		template<typename OTHER_C, String::ENCODING OTHER_E>
		GenericString &operator=(GenericString<OTHER_C, OTHER_E> &&rhs) noexcept {
			Assign(std::move(rhs));
			return *this;
		}
	private:
		PCSTR_T xGetBufferBegin() const noexcept {
			return xm_pchLong ? xm_pchLong.get() : std::begin(xm_achShort);
		}
		PSTR_T xGetBufferBegin() noexcept {
			return xm_pchLong ? xm_pchLong.get() : std::begin(xm_achShort);
		}
		PCSTR_T xGetBufferEnd() const noexcept {
			return xm_pchLong ? (xm_pchLong.get() + xm_uLongCap) : std::end(xm_achShort);
		}
		PSTR_T xGetBufferEnd() noexcept {
			return xm_pchLong ? (xm_pchLong.get() + xm_uLongCap) : std::end(xm_achShort);
		}

		std::size_t xGetRealCapacity() const noexcept {
			return xm_pchLong ? xm_uLongCap : COUNTOF(xm_achShort);
		}
		void xRealReserve(std::size_t uNewCap, std::size_t uMoveOffset){
			// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
			std::size_t uRealCap = uNewCap;
			--uRealCap;
			uRealCap |= uRealCap >> 1;
			uRealCap |= uRealCap >> 2;
			uRealCap |= uRealCap >> 4;
			uRealCap |= uRealCap >> 8;
			uRealCap |= uRealCap >> 16;
#ifdef __amd64__
			uRealCap |= uRealCap >> 32;
#endif
			++uRealCap;

			std::unique_ptr<CHAR_T[]> pchOldLong;
			const PCSTR_T pchOld = xm_pchFront;

			if(uRealCap > xGetRealCapacity()){
				pchOldLong.reset(new CHAR_T[uRealCap]);

				const std::size_t uOffset = GetLeadingGaps();
				xm_pchLong.swap(pchOldLong);
				xm_uLongCap = uRealCap;
				xm_pchFront = xm_pchLong.get() + uOffset;
			}
			xm_pchFront += uMoveOffset;

			if(xm_pchFront < pchOld){
				xCopyZFwd(xm_pchFront, pchOld);
			} else if(xm_pchFront > pchOld){
				xCopyZBwd(xm_pchFront, pchOld);
			}
		}

		UNIFIED_CHAR_SEQ xUnify() const;
		void xDisunify(UNIFIED_CHAR_SEQ &&ucsUnified);
	public:
		PCSTR_T GetCStr() const noexcept {
			return xm_pchFront;
		}
		PSTR_T GetCStr() noexcept {
			return xm_pchFront;
		}
		bool IsEmpty() const noexcept {
			return xm_pchFront[0] == CHAR_T();
		}
		void Clear() noexcept {
			xm_pchFront[0] = CHAR_T();
		}

		std::size_t GetLength() const noexcept {
			return xStrLen(xm_pchFront);
		}
		std::size_t GetCapacity() const noexcept {
			return xGetRealCapacity();
		}
		std::size_t GetLeadingGaps() const noexcept {
			return (std::size_t)(xm_pchFront - xGetBufferBegin());
		}
		void Reserve(std::size_t uCapacity){
			xRealReserve(GetLeadingGaps() + uCapacity, 0);
		}
		void Reserve(std::size_t uLeading, std::size_t uCapacity){
			const auto uLeadingGaps = GetLeadingGaps();
			if(uLeading > uLeadingGaps){
				xRealReserve(uLeading + uCapacity, uLeading - uLeadingGaps);
			} else {
				xRealReserve(uLeadingGaps + uCapacity, 0);
			}
		}
		void ReserveInc(std::size_t uTrailing){
			ReserveInc(0, uTrailing);
		}
		void ReserveInc(std::size_t uLeading, std::size_t uTrailing){
			xRealReserve(xGetRealCapacity() + uLeading + uTrailing, uLeading);
		}

		void Assign(CHAR_T ch, std::size_t uCount){
			Reserve(uCount + 1);
			xFillN(xm_pchFront, ch, uCount);
			xm_pchFront[uCount] = CHAR_T();
		}
		void Assign(PCSTR_T pszSrc){
			Assign(pszSrc, xStrLen(pszSrc));
		}
		void Assign(PCSTR_T pchSrc, std::size_t uSrcLen){
			if(uSrcLen == 0){
				Clear();
			} else {
				Reserve(uSrcLen + 1);
				xCopyN(xm_pchFront, pchSrc, uSrcLen)[0] = CHAR_T();
			}
		}
		void Assign(const GenericString &rhs){
			if(&rhs == this){
				return;
			}
			Assign(rhs.GetCStr(), rhs.GetLength());
		}
		void Assign(GenericString &&rhs){
			Swap(rhs);
		}
		template<typename OTHER_C, String::ENCODING OTHER_E>
		void Assign(const GenericString<OTHER_C, OTHER_E> &rhs){
			xDisunify(rhs.xUnify());
		}

		void Swap(GenericString &rhs) noexcept {
			if(&rhs == this){
				return;
			}

			const auto SwapShortWithLong = [](GenericString &strShort, GenericString &strLong) -> void {
				xCopyN(strLong.xm_achShort, strShort.xm_achShort, COUNTOF(strShort.xm_achShort));

				std::swap(strLong.xm_pchLong, strShort.xm_pchLong);
				std::swap(strLong.xm_uLongCap, strShort.xm_uLongCap);
				const auto pchLongFront = strLong.xm_pchFront;
				strLong.xm_pchFront = strShort.xm_pchFront - strShort.xm_achShort + strLong.xm_achShort;
				strShort.xm_pchFront = pchLongFront;
			};

			if(xm_pchLong){
				if(rhs.xm_pchLong){
					std::swap(xm_pchLong, rhs.xm_pchLong);
					std::swap(xm_uLongCap, rhs.xm_uLongCap);
					std::swap(xm_pchFront, rhs.xm_pchFront);
				} else {
					SwapShortWithLong(rhs, *this);
				}
			} else {
				if(rhs.xm_pchLong){
					SwapShortWithLong(*this, rhs);
				} else {
					CHAR_T achTemp[COUNTOF(xm_achShort)];

					xCopyN(achTemp, rhs.xm_achShort, COUNTOF(xm_achShort));
					xCopyN(rhs.xm_achShort, xm_achShort, COUNTOF(xm_achShort));
					xCopyN(xm_achShort, achTemp, COUNTOF(xm_achShort));
				}
			}
		}

		void Append(CHAR_T ch, std::size_t uCount = 1, std::size_t *puContext = nullptr){
			std::size_t uEnd = NPOS;
			if(puContext != nullptr){
				uEnd = *puContext;
			}
			if(uEnd == NPOS){
				uEnd = GetLength();
			}
			const std::size_t uNewEnd = uEnd + uCount;
			if(uNewEnd >= GetCapacity()){
				xRealReserve(uNewEnd + 1, 0);
			}
			xFillN(xm_pchFront + uEnd, ch, uCount);
			xm_pchFront[uNewEnd] = CHAR_T();
			if(puContext != nullptr){
				*puContext = uNewEnd;
			}
		}
		void Append(PCSTR_T pszSrc, std::size_t *puContext = nullptr){
			Append(pszSrc, xStrLen(pszSrc), puContext);
		}
		void Append(PCCHAR_T pchSrc, std::size_t uSrcLen, std::size_t *puContext = nullptr){
			std::size_t uEnd = NPOS;
			if(puContext != nullptr){
				uEnd = *puContext;
			}
			if(uEnd == NPOS){
				uEnd = GetLength();
			}
			const std::size_t uNewEnd = uEnd + uSrcLen;
			if(uNewEnd >= GetCapacity()){
				xRealReserve(uNewEnd + 1, 0);
			}
			xCopyN(xm_pchFront + uEnd, pchSrc, uSrcLen)[0] = CHAR_T();
			if(puContext != nullptr){
				*puContext = uNewEnd;
			}
		}
		void Append(const GenericString &rhs, std::size_t *puContext = nullptr){
			const auto uLen = rhs.GetLength();
			if(&rhs == this){
				xRealReserve(uLen * 2, 0);
			}
			Append(rhs.GetCStr(), uLen, puContext);
		}
		void Truncate(std::size_t uCount, std::size_t *puContext = nullptr) noexcept {
			std::size_t uEnd = NPOS;
			if(puContext != nullptr){
				uEnd = *puContext;
			}
			if(uEnd == NPOS){
				uEnd = GetLength();
			}
			const std::size_t uNewEnd = (uEnd <= uCount) ? 0 : (uEnd - uCount);
			xm_pchFront[uNewEnd] = CHAR_T();
			if(puContext != nullptr){
				*puContext = uNewEnd;
			}
		}

		void Unshift(CHAR_T ch, std::size_t uCount = 1, std::size_t *puContext = nullptr){
			std::size_t uEnd = NPOS;
			if(puContext != nullptr){
				uEnd = *puContext;
			}
			if(uEnd == NPOS){
				uEnd = GetLength();
			}
			const std::size_t uNewEnd = uEnd + uCount;
			const std::size_t uLeadingGaps = GetLeadingGaps();
			if(uCount > uLeadingGaps){
				xRealReserve(uCount + uNewEnd + 1, uCount - uLeadingGaps);
			}
			xm_pchFront -= uCount;
			xFillN(xm_pchFront, ch, uCount);
			if(puContext != nullptr){
				*puContext = uNewEnd;
			}
		}
		void Unshift(PCSTR_T pszSrc, std::size_t *puContext = nullptr){
			Unshift(pszSrc, xStrLen(pszSrc), puContext);
		}
		void Unshift(PCCHAR_T pchSrc, std::size_t uSrcLen, std::size_t *puContext = nullptr){
			std::size_t uEnd = NPOS;
			if(puContext != nullptr){
				uEnd = *puContext;
			}
			if(uEnd == NPOS){
				uEnd = GetLength();
			}
			const std::size_t uNewEnd = uEnd + uSrcLen;
			const std::size_t uLeadingGaps = GetLeadingGaps();
			if(uSrcLen > uLeadingGaps){
				xRealReserve(uSrcLen + uNewEnd + 1, uSrcLen - uLeadingGaps);
			}
			xm_pchFront -= uSrcLen;
			xCopyN(xm_pchFront, pchSrc, uSrcLen);
			if(puContext != nullptr){
				*puContext = uNewEnd;
			}
		}
		void Unshift(const GenericString &rhs, std::size_t *puContext = nullptr){
			const auto uLen = rhs.GetLength();
			if(&rhs == this){
				xRealReserve(uLen * 2, 0);
			}
			Unshift(rhs.GetCStr(), uLen, puContext);
		}
		void Shift(std::size_t uCount, std::size_t *puContext = nullptr) noexcept {
			std::size_t uEnd = NPOS;
			if(puContext != nullptr){
				uEnd = *puContext;
			}
			if(uEnd == NPOS){
				uEnd = GetLength();
			}
			const std::size_t uNewEnd = (uEnd <= uCount) ? 0 : (uEnd - uCount);
			xm_pchFront += uEnd - uNewEnd;
			if(puContext != nullptr){
				*puContext = uNewEnd;
			}
		}

		int Compare(PCSTR_T rhs) const noexcept {
			return xStrCmp(GetCStr(), rhs);
		}
		int Compare(const GenericString &rhs) const noexcept {
			return xStrCmp(GetCStr(), rhs.GetCStr());
		}
		int Compare(PCSTR_T rhs, std::size_t uMaxCount) const noexcept {
			return xStrNCmp(GetCStr(), rhs, uMaxCount);
		}
		int Compare(const GenericString &rhs, std::size_t uMaxCount) const noexcept {
			return xStrNCmp(GetCStr(), rhs.GetCStr(), uMaxCount);
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
			const std::size_t uLen = GetLength();
			const auto uRealBegin = xTranslateOffset(uLen, nBegin);
			const auto uRealEnd = xTranslateOffset(uLen, nEnd);
			if(uRealBegin < uRealEnd){
				const auto uCount = uRealEnd - uRealBegin;
				strRet.Reserve(uCount + 1);
				xCopyN(strRet.xm_pchFront, xm_pchFront + uRealBegin, uCount);
				strRet.xm_pchFront[uCount] = CHAR_T();
			} else if(uRealBegin > uRealEnd){
				const auto uCount = uRealBegin - uRealEnd;
				strRet.Reserve(uCount + 1);
				xRevCopyN(strRet.xm_pchFront, xm_pchFront + uRealEnd, uCount);
				strRet.xm_pchFront[uCount] = CHAR_T();
			}
			return std::move(strRet);
		}

		// 举例：
		//   FindFirstAfter("def", 3)   返回 3；
		//   FindFirstAfter("def", 4)   返回 NPOS；
		//   FindLastBefore("def", 5)   返回 NPOS；
		//   FindLastBefore("def", 6)   返回 3。
		std::size_t FindFirstAfter(PCSTR_T pszToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
			return FindFirstAfter(pszToFind, xStrLen(pszToFind), nOffsetBegin);
		}
		std::size_t FindFirstAfter(PCCHAR_T pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
			if(uLenToFind == 0){
				return 0;
			}
			const std::size_t uLen = GetLength();
			if(uLen < uLenToFind){
				return NPOS;
			}
			const auto uRealBegin = xTranslateOffset(uLen, nOffsetBegin);
			if(uRealBegin + uLenToFind > uLen){
				return NPOS;
			}
/*
			auto pchRead = xm_pchFront + uRealBegin;
			const auto pchEnd = xm_pchFront + uLen - uLenToFind + 1;
			for(;;){
				if(xStrNCmp(pchRead, pchToFind, uLenToFind) == 0){
					return (std::size_t)(pchRead - xm_pchFront);
				}
				if(pchRead == pchEnd){
					return NPOS;
				}
				++pchRead;
			}
*/
			const auto uPos = xKMPStrStr(
				(PCCHAR_T)(xm_pchFront + uRealBegin),
				uLen - uRealBegin,
				pchToFind,
				uLenToFind
			);
			return (uPos == NPOS) ? NPOS : (uPos + uRealBegin);
		}
		std::size_t FindLastBefore(PCSTR_T pszToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
			return FindLastBefore(pszToFind, xStrLen(pszToFind), nOffsetEnd);
		}
		std::size_t FindLastBefore(PCCHAR_T pchToFind, std::size_t uLenToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
			const std::size_t uLen = GetLength();
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
/*
			auto pchRead = xm_pchFront + uRealEnd - uLenToFind;
			for(;;){
				if(xStrNCmp(pchRead, pchToFind, uLenToFind) == 0){
					return (std::size_t)(pchRead - xm_pchFront);
				}
				if(pchRead == xm_pchFront){
					return NPOS;
				}
				--pchRead;
			}
*/
			const auto uPos = xKMPStrStr(
				std::reverse_iterator<PCCHAR_T>(xm_pchFront + uRealEnd),
				uRealEnd,
				std::reverse_iterator<PCCHAR_T>(pchToFind + uLenToFind),
				uLenToFind
			);
			return (uPos == NPOS) ? NPOS : (uRealEnd - uPos - uLenToFind);
		}

		// 举例：
		//   FindFirstAfter('c', 3)   返回 NPOS；
		//   FindFirstAfter('d', 3)   返回 3；
		//   FindLastBefore('c', 3)   返回 2；
		//   FindLastBefore('d', 3)   返回 NPOS。
		std::size_t FindFirstAfter(CHAR_T chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
			const std::size_t uLen = GetLength();
			if(uLen == 0){
				return NPOS;
			}
			const auto uRealBegin = xTranslateOffset(uLen, nOffsetBegin);
			if(uRealBegin == uLen){
				return NPOS;
			}
			auto pchRead = xm_pchFront + uRealBegin;
			const auto pchEnd = xm_pchFront + uLen;
			for(;;){
				if(*pchRead == chToFind){
					return (std::size_t)(pchRead - xm_pchFront);
				}
				if(pchRead == pchEnd){
					return NPOS;
				}
				++pchRead;
			}
		}
		std::size_t FindLastBefore(CHAR_T chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
			const std::size_t uLen = GetLength();
			if(uLen == 0){
				return NPOS;
			}
			const auto uRealEnd = xTranslateOffset(uLen, nOffsetEnd);
			if(uRealEnd == 0){
				return NPOS;
			}
			auto pchRead = xm_pchFront + uRealEnd - 1;
			for(;;){
				if(*pchRead == chToFind){
					return (std::size_t)(pchRead - xm_pchFront);
				}
				if(pchRead == xm_pchFront){
					return NPOS;
				}
				--pchRead;
			}
		}

		// 参考 Slice。
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, CHAR_T chRep, std::size_t uRepCount = 1){
			const std::size_t uLen = GetLength();
			const auto uRealBegin = xTranslateOffset(uLen, nBegin);
			const auto uRealEnd = xTranslateOffset(uLen, nEnd);
			if(uRealBegin < uRealEnd){
				const auto uCount = uRealEnd - uRealBegin;
				if(uRepCount > uCount){
					ReserveInc(0, uRepCount - uCount);
					xCopyZBwd(xm_pchFront + uRealBegin + uRepCount, xm_pchFront + uRealEnd);
				} else if(uRepCount < uCount){
					xCopyZFwd(xm_pchFront + uRealBegin + uRepCount, xm_pchFront + uRealEnd);
				}
				xFillN(xm_pchFront + uRealBegin, chRep, uCount);
			} else if(uRealBegin > uRealEnd){
				const auto uCount = uRealBegin - uRealEnd;
				if(uRepCount > uCount){
					ReserveInc(0, uRepCount - uCount);
					xCopyZBwd(xm_pchFront + uRealEnd + uRepCount, xm_pchFront + uRealBegin);
				} else {
					xCopyZFwd(xm_pchFront + uRealEnd + uRepCount, xm_pchFront + uRealBegin);
				}
				xFillN(xm_pchFront + uRealEnd, chRep, uCount);
			}
		}
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCSTR_T pszRep){
			Replace(nBegin, nEnd, pszRep, xStrLen(pszRep));
		}
		void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, PCCHAR_T pchRep, std::size_t uRepLen){
			const std::size_t uLen = GetLength();
			const auto uRealBegin = xTranslateOffset(uLen, nBegin);
			const auto uRealEnd = xTranslateOffset(uLen, nEnd);
			if(uRealBegin < uRealEnd){
				const auto uCount = uRealEnd - uRealBegin;
				if(uRepLen > uCount){
					ReserveInc(0, uRepLen - uCount);
					xCopyZBwd(xm_pchFront + uRealBegin + uRepLen, xm_pchFront + uRealEnd);
				} else if(uRepLen < uCount){
					xCopyZFwd(xm_pchFront + uRealBegin + uRepLen, xm_pchFront + uRealEnd);
				}
				xCopyN(xm_pchFront + uRealBegin, pchRep, uRepLen);
			} else if(uRealBegin > uRealEnd){
				const auto uCount = uRealBegin - uRealEnd;
				if(uRepLen > uCount){
					ReserveInc(0, uRepLen - uCount);
					xCopyZBwd(xm_pchFront + uRealEnd + uRepLen, xm_pchFront + uRealBegin);
				} else {
					xCopyZFwd(xm_pchFront + uRealEnd + uRepLen, xm_pchFront + uRealBegin);
				}
				xRevCopyN(xm_pchFront + uRealEnd, pchRep, uRepLen);
			}
		}
	public:
		explicit operator bool() const noexcept {
			return !IsEmpty();
		}
		operator PCSTR_T() const noexcept {
			return GetCStr();
		}
		operator PSTR_T() noexcept {
			return GetCStr();
		}
/*		const CHAR_T &operator[](std::size_t uIndex) const noexcept {
			return GetCStr()[uIndex];
		}
		CHAR_T &operator[](std::size_t uIndex) noexcept {
			return GetCStr()[uIndex];
		}*/

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
			const auto uLen = GetLength();
			if(rhs.GetLeadingGaps() < uLen){
				Append(rhs);
			} else {
				rhs.Append(GetCStr(), uLen);
				Assign(std::move(rhs));
			}
			return *this;
		}

		bool operator==(PCSTR_T rhs) const noexcept {
			return Compare(rhs) == 0;
		}
		bool operator==(const GenericString &rhs) const noexcept {
			return Compare(rhs.GetCStr()) == 0;
		}
		bool operator!=(PCSTR_T rhs) const noexcept {
			return Compare(rhs) != 0;
		}
		bool operator!=(const GenericString &rhs) const noexcept {
			return Compare(rhs.GetCStr()) != 0;
		}
		bool operator<(PCSTR_T rhs) const noexcept {
			return Compare(rhs) < 0;
		}
		bool operator<(const GenericString &rhs) const noexcept {
			return Compare(rhs.GetCStr()) < 0;
		}
		bool operator>(PCSTR_T rhs) const noexcept {
			return Compare(rhs) > 0;
		}
		bool operator>(const GenericString &rhs) const noexcept {
			return Compare(rhs.GetCStr()) > 0;
		}
		bool operator<=(PCSTR_T rhs) const noexcept {
			return Compare(rhs) <= 0;
		}
		bool operator<=(const GenericString &rhs) const noexcept {
			return Compare(rhs.GetCStr()) <= 0;
		}
		bool operator>=(PCSTR_T rhs) const noexcept {
			return Compare(rhs) >= 0;
		}
		bool operator>=(const GenericString &rhs) const noexcept {
			return Compare(rhs.GetCStr()) >= 0;
		}
	};

	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(const GenericString<C, E> &lhs, const GenericString<C, E> &rhs){
		return GenericString<C, E>(lhs) += rhs;
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(GenericString<C, E> &&lhs, const GenericString<C, E> &rhs){
		return std::move(lhs += rhs);
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(const GenericString<C, E> &lhs, GenericString<C, E> &&rhs){
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(GenericString<C, E> &&lhs, GenericString<C, E> &&rhs){
		if(lhs.GetCapacity() < rhs.GetCapacity()){
			rhs.Unshift(lhs);
			return std::move(rhs);
		} else {
			return std::move(lhs += rhs);
		}
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(const C *lhs, const GenericString<C, E> &rhs){
		return GenericString<C, E>(lhs) + rhs;
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(const C *lhs, GenericString<C, E> &&rhs){
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(C lhs, const GenericString<C, E> &rhs){
		return GenericString<C, E>(lhs) + rhs;
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(C lhs, GenericString<C, E> &&rhs){
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(const GenericString<C, E> &lhs, const C *rhs){
		return GenericString<C, E>(lhs) += rhs;
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(GenericString<C, E> &&lhs, const C *rhs){
		return std::move(lhs += rhs);
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(const GenericString<C, E> &lhs, C rhs){
		return GenericString<C, E>(lhs) += rhs;
	}
	template<typename C, String::ENCODING E>
	GenericString<C, E> operator+(GenericString<C, E> &&lhs, C rhs){
		return std::move(lhs += rhs);
	}

	template<typename C, String::ENCODING E>
	bool operator==(const C *lhs, const GenericString<C, E> &rhs) noexcept {
		return rhs == lhs;
	}
	template<typename C, String::ENCODING E>
	bool operator!=(const C *lhs, const GenericString<C, E> &rhs) noexcept {
		return rhs != lhs;
	}
	template<typename C, String::ENCODING E>
	bool operator<(const C *lhs, const GenericString<C, E> &rhs) noexcept {
		return rhs >= lhs;
	}
	template<typename C, String::ENCODING E>
	bool operator>(const C *lhs, const GenericString<C, E> &rhs) noexcept {
		return rhs <= lhs;
	}
	template<typename C, String::ENCODING E>
	bool operator<=(const C *lhs, const GenericString<C, E> &rhs) noexcept {
		return rhs > lhs;
	}
	template<typename C, String::ENCODING E>
	bool operator>=(const C *lhs, const GenericString<C, E> &rhs) noexcept {
		return rhs < lhs;
	}

	extern template class GenericString<char,		String::ENC_UTF8>;
	extern template class GenericString<char,		String::ENC_ANSI>;
	extern template class GenericString<wchar_t,	String::ENC_UTF16>;

	typedef GenericString<char,		String::ENC_UTF8>		UTF8String;
	typedef GenericString<char,		String::ENC_ANSI>		ANSIString, NarrowString, NCString;
	typedef GenericString<wchar_t,	String::ENC_UTF16>		UTF16String, WideString, WCString;
}

#endif
