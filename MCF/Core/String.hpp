// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_HPP__
#define __MCF_STRING_HPP__

#include "StringObserver.hpp"
#include "../../MCFCRT/cpp/ext/count_of.hpp"
#include "Utilities.hpp"
#include <memory>
#include <new>
#include <cstdint>
#include <cstring>
#include <cwchar>

namespace MCF {

enum class StringEncoding {
	UTF8,
	ANSI,
	UTF16
};

template<typename Char_t, StringEncoding ENCODING>
class String;

typedef String<wchar_t, StringEncoding::UTF16> UnifiedString;

template<typename Char_t, StringEncoding ENCODING>
class String {
	static_assert(std::is_arithmetic<Char_t>::value, "Char_t must be an arithmetic type.");

	template<typename, StringEncoding>
	friend class String;

public:
	typedef StringObserver<Char_t> Observer;

	enum : std::size_t {
		NPOS = Observer::NPOS
	};

private:
	union xStorage {
		struct __attribute__((packed)) {
			Char_t achSmall[32 / sizeof(Char_t) - 2];
			Char_t chNull;
			typename std::make_unsigned<Char_t>::type uchSmallLength;
		};
		struct {
			Char_t *pchLargeBegin;
			std::size_t uLargeLength;
			std::size_t uLargeBufferSize;
		};
	} xm_vStorage;

public:
	constexpr String() noexcept
		: xm_vStorage{{{Char_t()}, Char_t(), 0}}
	{
	}
	explicit String(Char_t ch, std::size_t uCount = 1) : String() {
		Assign(ch, uCount);
	}
	String(const Char_t *pchBegin, const Char_t *pchEnd) : String() {
		Assign(pchBegin, pchEnd);
	}
	String(const Char_t *pchSrc, std::size_t uSrcLen) : String() {
		Assign(pchSrc, uSrcLen);
	}
	explicit String(const Observer &obs) : String() {
		Assign(obs);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	explicit String(const String<OtherChar_t, OTHER_ENCODING> &rhs) : String() {
		Assign(rhs);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	explicit String(String<OtherChar_t, OTHER_ENCODING> &&rhs) : String() {
		Assign(std::move(rhs));
	}
	String(const String &rhs) : String() {
		Assign(rhs);
	}
	String(String &&rhs) noexcept : String() {
		Assign(std::move(rhs));
	}
	String &operator=(Char_t ch) noexcept {
		Assign(ch, 1);
		return *this;
	}
	String &operator=(const Observer &obs){
		Assign(obs);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator=(const String<OtherChar_t, OTHER_ENCODING> &rhs){
		Assign(rhs);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator=(String<OtherChar_t, OTHER_ENCODING> &&rhs){
		Assign(std::move(rhs));
		return *this;
	}
	String &operator=(const String &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(String &&rhs) noexcept {
		Assign(std::move(rhs));
		return *this;
	}
	~String(){
		if(xm_vStorage.chNull != Char_t()){
			delete[] xm_vStorage.pchLargeBegin;
		}
	}

private:
	void xSetSize(std::size_t uNewSize) noexcept {
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

		const std::size_t uNewSize = nDeltaBegin + uOldLength + nDeltaEnd;
		std::size_t uSizeToAlloc = (uNewSize > GetCapacity()) ? (uNewSize + 1) : 0;
		if(uSizeToAlloc != 0){
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & -0x10;
			pchNewBuffer = new Char_t[uSizeToAlloc];
		}

		do {
			std::size_t uCopyOffsetBegin = 0;
			if(nDeltaBegin < 0){
				if((std::size_t)-nDeltaBegin >= uOldLength){
					break;
				}
				uCopyOffsetBegin -= nDeltaBegin;
			}

			std::size_t uCopyOffsetEnd = uOldLength;
			if(nDeltaEnd < 0){
				if((std::size_t)-nDeltaEnd >= uOldLength){
					break;
				}
				uCopyOffsetEnd += nDeltaEnd;
			}

			if(uCopyOffsetBegin >= uCopyOffsetEnd){
				break;
			}
			if(nDeltaBegin < 0){
				std::copy(
					pchOldBuffer + uCopyOffsetBegin,
					pchOldBuffer + uCopyOffsetEnd,
					pchNewBuffer
				);
			} else {
				std::copy_backward(
					pchOldBuffer + uCopyOffsetBegin,
					pchOldBuffer + uCopyOffsetEnd,
					pchNewBuffer + uCopyOffsetEnd + nDeltaBegin
				);
			}
		} while(false);

		if(uSizeToAlloc != 0){
			if(xm_vStorage.chNull == Char_t()){
				xm_vStorage.chNull = Char_t() + 1;
			} else {
				delete[] pchOldBuffer;
			}

			xm_vStorage.pchLargeBegin = pchNewBuffer;
			xm_vStorage.uLargeLength = uOldLength;
			xm_vStorage.uLargeBufferSize = uSizeToAlloc;
		}

		if(bSetEnd){
			xSetSize(uNewSize);
		} else {
			pchNewBuffer[uOldLength] = Char_t();
		}
	}

	// 返回值如果不是 nullptr，就使用返回值拷贝构造。
	// 否则 ucsTemp 为出参，可以移动构造。
	const UnifiedString *xUnify(UnifiedString &ucsTemp) const;
	// 追加到末尾。
	void xDisunify(const UnifiedString &ucsTemp);
	void xDisunify(UnifiedString &&ucsTemp);

public:
	const Char_t *GetBegin() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]);
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	Char_t *GetBegin() noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]);
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	const Char_t *GetEnd() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]) + xm_vStorage.uchSmallLength;
		} else {
			return xm_vStorage.pchLargeBegin + xm_vStorage.uLargeLength;
		}
	}
	Char_t *GetEnd() noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]) + xm_vStorage.uchSmallLength;
		} else {
			return xm_vStorage.pchLargeBegin + xm_vStorage.uLargeLength;
		}
	}

	const Char_t *GetCStr() const noexcept {
		return GetBegin();
	}
	Char_t *GetCStr() noexcept {
		return GetBegin();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const Char_t *GetData() const noexcept {
		return GetBegin();
	}
	Char_t *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		return GetEnd() - GetBegin();
	}

	Observer GetObserver() const noexcept {
		return Observer(GetBegin(), GetEnd());
	}

	void Resize(std::size_t uNewSize){
		Reserve(uNewSize);
		xSetSize(uNewSize);
	}
	void Shrink() noexcept {
		xSetSize(Observer(GetCStr()).GetLength());
	}

	bool IsEmpty() const noexcept {
		return GetBegin() == GetEnd();
	}
	void Clear() noexcept {
		xSetSize(0);
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

	void Swap(String &rhs) noexcept {
		if(this != &rhs){
			std::swap(xm_vStorage, rhs.xm_vStorage);
		}
	}

	int Compare(const Observer &rhs) const noexcept {
		return GetObserver().Compare(rhs);
	}

	void Assign(Char_t ch, std::size_t uCount = 1){
		Resize(uCount);
		std::fill_n(GetCStr(), uCount, ch);
	}
	void Assign(const Char_t *pchBegin, const Char_t *pchEnd){
		Assign(pchBegin, pchEnd - pchBegin);
	}
	void Assign(const Char_t *pchSrc, std::size_t uSrcLen){
		Resize(uSrcLen);
		std::copy_n(pchSrc, uSrcLen, GetCStr());
	}
	void Assign(const Observer &obs){
		Resize(obs.GetLength());
		std::copy(obs.GetBegin(), obs.GetEnd(), GetCStr());
	}
	void Assign(String &&rhs) noexcept {
		if(this != &rhs){
			Clear();
			Swap(rhs);
		}
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	void Assign(const String<OtherChar_t, OTHER_ENCODING> &rhs){
		UnifiedString ucsTemp;
		const auto pucsUnfied = rhs.xUnify(ucsTemp);
		Clear();
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
	void Append(const Char_t *pchBegin, const Char_t *pchEnd){
		Append(pchBegin, pchEnd - pchBegin);
	}
	void Append(const Char_t *pchSrc, std::size_t uSrcLen){
		const auto uOldLength = GetLength();
		xSlide(0, uSrcLen);
		std::copy_n(pchSrc, uSrcLen, GetCStr() + uOldLength);
	}
	void Append(const Observer &obs){
		const auto uOldLength = GetLength();
		xSlide(0, obs.GetLength());
		std::copy(obs.GetBegin(), obs.GetEnd(), GetCStr() + uOldLength);
	}
	void Append(String &&rhs){
		if(this == &rhs){
			Append(rhs);
		} else if(IsEmpty()){
			Assign(std::move(rhs));
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Append(rhs);
		} else {
			rhs.Unshift(*this);
			Assign(std::move(rhs));
		}
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	void Append(const String<OtherChar_t, OTHER_ENCODING> &rhs){
		UnifiedString ucsTemp;
		const auto pucsUnfied = rhs.xUnify(ucsTemp);
		if(pucsUnfied){
			xDisunify(*pucsUnfied);
		} else {
			xDisunify(std::move(ucsTemp));
		}
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		const auto uOldLength = GetLength();
		if(uOldLength <= uCount){
			Clear();
		} else {
			xSetSize(uOldLength - uCount);
		}
	}

	void Push(Char_t ch){
		Append(ch);
	}
	void Pop() noexcept {
		Truncate(1);
	}

	void PushNoCheck(Char_t ch) noexcept {
		ASSERT_MSG(GetLength() < GetCapacity(), L"String::PushNoCheck() 失败：容器已满。");

		if(xm_vStorage.chNull == Char_t()){
			xm_vStorage.achSmall[xm_vStorage.uchSmallLength] = ch;
			xm_vStorage.achSmall[++xm_vStorage.uchSmallLength] = Char_t();
		} else {
			xm_vStorage.pchLargeBegin[xm_vStorage.uLargeLength] = ch;
			xm_vStorage.pchLargeBegin[++xm_vStorage.uLargeLength] = Char_t();
		}
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(GetLength() != 0, L"String::PopNoCheck() 失败：容器已空。");

		if(xm_vStorage.chNull == Char_t()){
			xm_vStorage.achSmall[--xm_vStorage.uchSmallLength] = Char_t();
		} else {
			xm_vStorage.pchLargeBegin[--xm_vStorage.uLargeLength] = Char_t();
		}
	}

	void Unshift(Char_t ch, std::size_t uCount = 1){
		xSlide(uCount, 0);
		std::fill_n(GetCStr(), uCount, ch);
	}
	void Unshift(const Char_t *pchBegin, const Char_t *pchEnd){
		Unshift(pchBegin, pchEnd - pchBegin);
	}
	void Unshift(const Char_t *pchSrc, std::size_t uSrcLen){
		xSlide(uSrcLen, 0);
		std::copy_n(pchSrc, uSrcLen, GetCStr());
	}
	void Unshift(const Observer &obs){
		xSlide(obs.GetLength(), 0);
		std::copy(obs.GetBegin(), obs.GetEnd(), GetCStr());
	}
	void Unshift(String &&rhs){
		if(this == &rhs){
			Append(std::move(rhs));
		} else if(IsEmpty()){
			Assign(std::move(rhs));
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
			xSetSize(uNewLength);
		}
	}

	Observer Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		return GetObserver().Slice(nBegin, nEnd);
	}

	std::size_t FindFirstAfter(const Observer &obsToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().FindFirstAfter(obsToFind, nOffsetBegin);
	}
	std::size_t FindFirstAfter(Char_t chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().FindFirstAfter(chToFind, nOffsetBegin);
	}
	std::size_t FindLastBefore(const Observer &obsToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindLastBefore(obsToFind, nOffsetEnd);
	}
	std::size_t FindLastBefore(Char_t chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindLastBefore(chToFind, nOffsetEnd);
	}
/*
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
			xSetSize(uNewLength);
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
			xSetSize(uNewLength);
		}
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
			xSetSize(uNewLength);
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
			xSetSize(uNewLength);
		}
	}
*/
	Observer GetReverse() const noexcept {
		auto obsRet(GetObserver());
		obsRet.Reverse();
		return std::move(obsRet);
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
	operator Observer() const noexcept {
		return GetObserver();
	}

	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const Char_t *() const noexcept {
		return GetCStr();
	}
	explicit operator Char_t *() noexcept {
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

	String &operator+=(const Observer &rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(Char_t rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(String &&rhs){
		Append(std::move(rhs));
		return *this;
	}

	bool operator==(const Observer &rhs) const noexcept {
		return GetObserver() == rhs;
	}
	bool operator!=(const Observer &rhs) const noexcept {
		return GetObserver() != rhs;
	}
	bool operator<(const Observer &rhs) const noexcept {
		return GetObserver() < rhs;
	}
	bool operator>(const Observer &rhs) const noexcept {
		return GetObserver() > rhs;
	}
	bool operator<=(const Observer &rhs) const noexcept {
		return GetObserver() <= rhs;
	}
	bool operator>=(const Observer &rhs) const noexcept {
		return GetObserver() >= rhs;
	}
};

template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, const StringObserver<Char_t> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, Char_t rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + 1);
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(String<Char_t, ENCODING> &&lhs, const StringObserver<Char_t> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(String<Char_t, ENCODING> &&lhs, Char_t rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}

template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(Char_t lhs, const String<Char_t, ENCODING> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(1 + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const StringObserver<Char_t> &lhs, String<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(Char_t lhs, String<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}

template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(StringObserver<Char_t> &&lhs, StringObserver<Char_t> &&rhs){
	if(lhs.GetCapacity() >= rhs.GetCapacity()){
		lhs.Append(rhs);
		return std::move(lhs);
	} else {
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
}

template<typename Char_t, StringEncoding ENCODING>
const Char_t *begin(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}
template<typename Char_t, StringEncoding ENCODING>
Char_t *begin(String<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}
template<typename Char_t, StringEncoding ENCODING>
const Char_t *cbegin(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}

template<typename Char_t, StringEncoding ENCODING>
const Char_t *end(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}
template<typename Char_t, StringEncoding ENCODING>
Char_t *end(String<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}
template<typename Char_t, StringEncoding ENCODING>
const Char_t *cend(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}

template class String<char,		StringEncoding::UTF8>;
template class String<char,		StringEncoding::ANSI>;
template class String<wchar_t,	StringEncoding::UTF16>;

typedef String<char,	StringEncoding::UTF8>	Utf8String;
typedef String<char,	StringEncoding::ANSI>	AnsiString;
typedef String<wchar_t,	StringEncoding::UTF16>	Utf16String;

}

#endif
