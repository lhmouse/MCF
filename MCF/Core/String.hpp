// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STRING_HPP__
#define __MCF_STRING_HPP__

#include "StringObserver.hpp"
#include "../../MCFCRT/cpp/ext/count_of.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include "Utilities.hpp"
#include <memory>
#include <cstdint>

namespace MCF {

enum class StringEncoding {
	UTF8,
	ANSI,
	UTF16,
	UTF32
};

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
	template<class Iterator_t>
	String(const Iterator_t &itBegin, const Iterator_t &itEnd) : String() {
		Assign(itBegin, itEnd);
	}
	template<class Iterator_t>
	String(const Iterator_t &itBegin, std::size_t uLen) : String() {
		Assign(itBegin, uLen);
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

	Char_t *xSnap(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, std::size_t uSize){
		const auto obsRemoved(Slice(nBegin, nEnd));
		const std::size_t uRemovedFrom = (const Char_t *)obsRemoved.GetBegin() - GetBegin();
		const std::size_t uRemovedTo = (const Char_t *)obsRemoved.GetEnd() - GetBegin();

		const auto uOldLength = GetLength();
		const auto uDeltaLength = uSize - (uRemovedTo - uRemovedFrom);
		Resize(uOldLength + uDeltaLength);

		const auto pchBegin = GetCStr();
		if((std::ptrdiff_t)uDeltaLength < 0){
			std::copy(pchBegin + uRemovedTo, pchBegin + uOldLength, GetEnd() - (uOldLength - uRemovedTo));
		} else if((std::ptrdiff_t)uDeltaLength > 0){
			std::copy_backward(pchBegin + uRemovedTo, pchBegin + uOldLength, GetEnd());
		}
		return pchBegin + uRemovedFrom;
	}

	VVector<wchar_t> xUnify() const;
	void xDeunifyAppend(const VVector<wchar_t> &vecUnified);

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
	const Char_t *GetCBegin() const noexcept {
		return GetBegin();
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
	const Char_t *GetCEnd() const noexcept {
		return GetEnd();
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
		Replace(0, -1, ch, uCount);
	}
	template<class Iterator_t>
	void Assign(const Iterator_t &itBegin, const Iterator_t &itEnd){
		Replace(0, -1, itBegin, itEnd);
	}
	template<class Iterator_t>
	void Assign(const Iterator_t &itBegin, std::size_t uLen){
		Replace(0, -1, itBegin, uLen);
	}
	void Assign(const Observer &obs){
		Replace(0, -1, obs);
	}
	void Assign(String &&rhs) noexcept {
		Swap(rhs);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	void Assign(const String<OtherChar_t, OTHER_ENCODING> &rhs){
		Clear();
		Append(rhs);
	}

	void Append(Char_t ch, std::size_t uCount = 1){
		Replace(-1, -1, ch, uCount);
	}
	template<class Iterator_t>
	void Append(const Iterator_t &itBegin, const Iterator_t &itEnd){
		Replace(-1, -1, itBegin, itEnd);
	}
	template<class Iterator_t>
	void Append(const Iterator_t &itBegin, std::size_t uLen){
		Replace(-1, -1, itBegin, uLen);
	}
	void Append(const Observer &obs){
		Replace(-1, -1, obs);
	}
	void Append(const String &str){
		Replace(-1, -1, str.GetObserver());
	}
	void Append(String &&str){
		if(this == &str){
			Append(str);
		} else if(IsEmpty()){
			Assign(std::move(str));
		} else if(GetCapacity() >= str.GetCapacity()){
			Append(str);
		} else {
			Swap(str);
			Unshift(str);
		}
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	void Append(const String<OtherChar_t, OTHER_ENCODING> &str){
		xDeunifyAppend(str.xUnify());
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		Replace(-1 - uCount, -1, Observer());
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
		Replace(0, 0, ch, uCount);
	}
	template<class Iterator_t>
	void Unshift(const Iterator_t &itBegin, const Iterator_t &itEnd){
		Replace(0, 0, itBegin, itEnd);
	}
	template<class Iterator_t>
	void Unshift(const Iterator_t &itBegin, std::size_t uLen){
		Replace(0, 0, itBegin, uLen);
	}
	void Unshift(const Observer &obs){
		Replace(0, 0, obs);
	}
	void Unshift(const String &str){
		Replace(0, 0, str.GetObserver());
	}
	void Unshift(String &&str){
		if(this == &str){
			Append(str);
		} else if(IsEmpty()){
			Assign(std::move(str));
		} else if(GetCapacity() >= str.GetCapacity()){
			Unshift(str);
		} else {
			Swap(str);
			Append(str);
		}
	}
	void Shift(std::size_t uCount = 1) noexcept {
		Replace(0, uCount, Observer());
	}

	Observer Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		return GetObserver().Slice(nBegin, nEnd);
	}

	std::size_t Find(const Observer &obsToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().Find(obsToFind, nOffsetBegin);
	}
	std::size_t Find(Char_t chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().Find(chToFind, nOffsetBegin);
	}
	std::size_t FindBackward(const Observer &obsToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindBackward(obsToFind, nOffsetEnd);
	}
	std::size_t FindBackward(Char_t chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindBackward(chToFind, nOffsetEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char_t chReplacement, std::size_t uCount = 1){
		const auto pchWrite = xSnap(nBegin, nEnd, uCount);
		std::fill_n(pchWrite, uCount, chReplacement);
	}
	template<class Iterator_t>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Iterator_t &itRepBegin, std::size_t uRepLen){
		// 注意，不指向同一个数组的两个指针相互比较是未定义行为。
		if((uRepLen != 0) && ((std::uintptr_t)&*itRepBegin - (std::uintptr_t)GetBegin() <= GetLength() * sizeof(Char_t))){
			// 待替换字符串和当前字符串重叠。
			String strTemp(*this);
			const auto pchWrite = strTemp.xSnap(nBegin, nEnd, uRepLen);
			std::copy_n(itRepBegin, uRepLen, pchWrite);
			Swap(strTemp);
		} else {
			const auto pchWrite = xSnap(nBegin, nEnd, uRepLen);
			std::copy_n(itRepBegin, uRepLen, pchWrite);
		}
	}
	template<class Iterator_t>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Iterator_t &itRepBegin, const Iterator_t &itRepEnd){
		Replace(nBegin, nEnd, itRepBegin, std::distance(itRepBegin, itRepEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Observer &obsReplacement){
		Replace(nBegin, nEnd, obsReplacement.GetBegin(), obsReplacement.GetLength());
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
	String &operator+=(const String &rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(String &&rhs){
		Append(std::move(rhs));
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator+=(const String<OtherChar_t, OTHER_ENCODING> &str){
		Append(str);
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
template<typename Char_t, StringEncoding ENCODING, typename OtherChar_t, StringEncoding OTHER_ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, const String<OtherChar_t, OTHER_ENCODING> &rhs){
	String<Char_t, ENCODING> strRet(lhs);
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
template<typename Char_t, StringEncoding ENCODING, typename OtherChar_t, StringEncoding OTHER_ENCODING>
String<Char_t, ENCODING> operator+(String<Char_t, ENCODING> &&lhs, const String<OtherChar_t, OTHER_ENCODING> &rhs){
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
	return str.GetCBegin();
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
	return str.GetCEnd();
}

extern template class String<char,		StringEncoding::ANSI>;
extern template class String<wchar_t,	StringEncoding::UTF16>;

extern template class String<char,		StringEncoding::UTF8>;
extern template class String<char16_t,	StringEncoding::UTF16>;
extern template class String<char32_t,	StringEncoding::UTF32>;

typedef String<char,		StringEncoding::ANSI>	AnsiString;
typedef String<wchar_t,		StringEncoding::UTF16>	WideString;

typedef String<char,		StringEncoding::UTF8>	Utf8String;
typedef String<char16_t,	StringEncoding::UTF16>	Utf16String;
typedef String<char32_t,	StringEncoding::UTF32>	Utf32String;

}

#endif
