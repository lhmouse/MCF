// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STRING_HPP_
#define MCF_STRING_HPP_

#include "StringObserver.hpp"
#include "../Containers/VVector.hpp"
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

template<typename Char, StringEncoding ENCODING>
class String;

namespace Impl {
	template<typename Char, StringEncoding ENCODING>
	struct UnicodeConv {
		// 成员函数待定义。此处全部为追加到末尾。
		void operator()(VVector<wchar_t> &vecUnified, const StringObserver<Char> &soSrc) const;
		void operator()(String<Char, ENCODING> &strDst, const VVector<wchar_t> &vecUnified) const;
	};

	template<typename DstChar, StringEncoding DST_ENCODING, typename SrcChar, StringEncoding SRC_ENCODING>
	struct Transcoder {
		void operator()(String<DstChar, DST_ENCODING> &strDst, const StringObserver<SrcChar> &soSrc) const;
	};
	template<typename DstChar, StringEncoding ENCODING, typename SrcChar>
	struct Transcoder<DstChar, ENCODING, SrcChar, ENCODING> {
		void operator()(String<DstChar, ENCODING> &strDst, const StringObserver<SrcChar> &soSrc) const;
	};
}

template<typename Char, StringEncoding ENCODING>
class String {
	static_assert(std::is_integral<Char>::value, "Char must be an integral type.");

	template<typename, StringEncoding>
	friend class String;

public:
	typedef StringObserver<Char> Observer;

	enum : std::size_t {
		NPOS = Observer::NPOS
	};

private:
	union xStorage {
		struct __attribute__((__packed__)) {
			Char achSmall[(4 * sizeof(void *)) / sizeof(Char) - 2];
			Char chNull;
			typename std::make_unsigned<Char>::type uchSmallLength;
		};
		struct {
			Char *pchLargeBegin;
			std::size_t uLargeLength;
			std::size_t uLargeCapacity;
		};
	} xm_vStorage;

	static_assert(sizeof(xm_vStorage) % 16 == 0, "Bad alignment.");

public:
	String() noexcept {
		xm_vStorage.achSmall[0]		= Char();
		xm_vStorage.chNull			= Char();
		xm_vStorage.uchSmallLength	= 0;
	}
	explicit String(Char ch, std::size_t uCount = 1)
		: String()
	{
		Assign(ch, uCount);
	}
	explicit String(const Char *pszBegin)
		: String()
	{
		Assign(pszBegin);
	}
	template<class Iterator>
	String(Iterator itBegin, Iterator itEnd)
		: String()
	{
		Assign(itBegin, itEnd);
	}
	template<class Iterator>
	String(Iterator itBegin, std::size_t uLen)
		: String()
	{
		Assign(itBegin, uLen);
	}
	explicit String(const Observer &obs)
		: String()
	{
		Assign(obs);
	}
	explicit String(std::initializer_list<Char> vInitList)
		: String()
	{
		Assign(vInitList);
	}
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	explicit String(const String<OtherChar, OTHER_ENCODING> &rhs)
		: String()
	{
		Assign(rhs);
	}
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	explicit String(String<OtherChar, OTHER_ENCODING> &&rhs)
		: String()
	{
		Assign(std::move(rhs));
	}
	String(const String &rhs)
		: String()
	{
		Assign(rhs);
	}
	String(String &&rhs) noexcept
		: String()
	{
		Swap(rhs);
	}
	String &operator=(Char ch) noexcept {
		Assign(ch);
		return *this;
	}
	String &operator=(const Char *pszBegin){
		Assign(pszBegin);
		return *this;
	}
	String &operator=(const Observer &obs){
		Assign(obs);
		return *this;
	}
	String &operator=(std::initializer_list<Char> vInitList){
		Assign(vInitList);
		return *this;
	}
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	String &operator=(const String<OtherChar, OTHER_ENCODING> &rhs){
		Assign(rhs);
		return *this;
	}
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	String &operator=(String<OtherChar, OTHER_ENCODING> &&rhs){
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
	~String() noexcept {
		if(xm_vStorage.chNull != Char()){
			delete[] xm_vStorage.pchLargeBegin;
		}

#ifndef NDEBUG
		std::memset(&xm_vStorage, 0xDD, sizeof(xm_vStorage));
#endif
	}

private:
	Char *xChopAndSplice(
		std::size_t uRemovedBegin,
		std::size_t uRemovedEnd,
		std::size_t uFirstOffset,
		std::size_t uThirdOffset
	){
		ASSERT(uRemovedBegin <= GetLength());
		ASSERT(uRemovedEnd <= GetLength());
		ASSERT(uRemovedBegin <= uRemovedEnd);
		ASSERT(uFirstOffset + uRemovedBegin <= uThirdOffset);

		const auto pchOldBuffer = GetBegin();
		const auto uOldLength = GetLength();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewLength = uThirdOffset + (uOldLength - uRemovedEnd);
		std::size_t uSizeToAlloc = uNewLength + 1;

		if(GetCapacity() < uNewLength){
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & (std::size_t)-0x10;
			if(uSizeToAlloc < uNewLength + 1){
				throw std::bad_alloc();
			}
			pchNewBuffer = new Char[uSizeToAlloc];
		}

	ASSERT_NOEXCEPT_BEGIN
		if(uRemovedBegin != 0){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if(uOldLength != uRemovedEnd){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(xm_vStorage.chNull == Char()){
				xm_vStorage.chNull = Char() + 1;
			} else {
				delete[] pchOldBuffer;
			}

			xm_vStorage.pchLargeBegin = pchNewBuffer;
			xm_vStorage.uLargeLength = uOldLength;
			xm_vStorage.uLargeCapacity = uSizeToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	ASSERT_NOEXCEPT_END
	}
	void xSetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(xm_vStorage.chNull == Char()){
			xm_vStorage.uchSmallLength = uNewSize;
			((Char (&)[SIZE_MAX])xm_vStorage.achSmall)[uNewSize] = Char();
		} else {
			xm_vStorage.uLargeLength = uNewSize;
			xm_vStorage.pchLargeBegin[uNewSize] = Char();
		}
	}

public:
	const Char *GetBegin() const noexcept {
		if(xm_vStorage.chNull == Char()){
			return xm_vStorage.achSmall;
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	Char *GetBegin() noexcept {
		if(xm_vStorage.chNull == Char()){
			return xm_vStorage.achSmall;
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	const Char *GetCBegin() const noexcept {
		return GetBegin();
	}
	const Char *GetEnd() const noexcept {
		if(xm_vStorage.chNull == Char()){
			return xm_vStorage.achSmall + xm_vStorage.uchSmallLength;
		} else {
			return xm_vStorage.pchLargeBegin + xm_vStorage.uLargeLength;
		}
	}
	Char *GetEnd() noexcept {
		if(xm_vStorage.chNull == Char()){
			return xm_vStorage.achSmall + xm_vStorage.uchSmallLength;
		} else {
			return xm_vStorage.pchLargeBegin + xm_vStorage.uLargeLength;
		}
	}
	const Char *GetCEnd() const noexcept {
		return GetEnd();
	}

	const Char *GetStr() const noexcept {
		return GetBegin();
	}
	Char *GetStr() noexcept {
		return GetBegin();
	}
	const Char *GetCStr() const noexcept {
		return GetBegin();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}

	Observer GetObserver() const noexcept {
		return Observer(GetBegin(), GetEnd());
	}

	Char *Resize(std::size_t uNewSize){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			xSetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Truncate(uOldSize - uNewSize);
		}
		return GetData();
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		Resize(uOldSize + uDeltaSize);
		return GetData() + uOldSize;
	}
	void Resize(std::size_t uNewSize, Char ch){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Append(ch, uNewSize - uOldSize);
		} else if(uNewSize < uOldSize){
			Truncate(uOldSize - uNewSize);
		}
	}
	Char *ResizeMore(std::size_t uDeltaSize, Char ch){
		const auto uOldSize = GetSize();
		Resize(uOldSize + uDeltaSize, ch);
		return GetData() + uOldSize;
	}
	void Shrink() noexcept {
		Resize(Observer(GetStr()).GetLength());
	}

	bool IsEmpty() const noexcept {
		return GetBegin() == GetEnd();
	}
	void Clear() noexcept {
		xSetSize(0);
	}

	std::size_t GetCapacity() const noexcept {
		if(xm_vStorage.chNull == Char()){
			return COUNT_OF(xm_vStorage.achSmall);
		} else {
			return xm_vStorage.uLargeCapacity - 1;
		}
	}
	void Reserve(std::size_t uNewCapacity){
		if(uNewCapacity > GetCapacity()){
			const auto uOldLength = GetLength();
			xChopAndSplice(uOldLength, uOldLength, 0, uNewCapacity);
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldLength = GetLength();
		xChopAndSplice(uOldLength, uOldLength, 0, uOldLength + uDeltaCapacity);
	}

	void Swap(String &rhs) noexcept {
		if(this != &rhs){
			std::swap(xm_vStorage, rhs.xm_vStorage);
		}
	}

	int Compare(const Observer &rhs) const noexcept {
		return GetObserver().Compare(rhs);
	}

	void Assign(Char ch, std::size_t uCount = 1){
		Clear();
		Append(ch, uCount);
	}
	void Assign(const Char *pszBegin){
		Assign(Observer(pszBegin));
	}
	template<class Iterator>
	void Assign(Iterator itBegin, Iterator itEnd){
		Assign(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator>
	void Assign(Iterator itBegin, std::size_t uLength){
		Clear();
		Append(itBegin, uLength);
	}
	void Assign(const Observer &obs){
		Assign(obs.GetBegin(), obs.GetEnd());
	}
	void Assign(std::initializer_list<Char> vInitList){
		Assign(Observer(vInitList));
	}
	void Assign(String &&rhs) noexcept {
		Swap(rhs);
	}
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	void Assign(const String<OtherChar, OTHER_ENCODING> &rhs){
		Clear();
		Append(rhs);
	}
	template<StringEncoding OTHER_ENCODING, typename OtherChar>
	void Assign(const StringObserver<OtherChar> &rhs){
		Clear();
		Append<OTHER_ENCODING, OtherChar>(rhs);
	}

	void Append(Char ch, std::size_t uCount = 1){
		const std::size_t uOldLength = GetLength();
		FillN(xChopAndSplice(uOldLength, uOldLength, 0, uOldLength + uCount), uCount, ch);
		xSetSize(uOldLength + uCount);
	}
	void Append(const Char *pszBegin){
		Append(Observer(pszBegin));
	}
	template<class Iterator>
	void Append(Iterator itBegin, Iterator itEnd){
		Append(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator>
	void Append(Iterator itBegin, std::size_t uLength){
		const std::size_t uOldLength = GetLength();
		CopyN(xChopAndSplice(uOldLength, uOldLength, 0, uOldLength + uLength), itBegin, uLength);
		xSetSize(uOldLength + uLength);
	}
	void Append(const Observer &obs){
		Append(obs.GetBegin(), obs.GetEnd());
	}
	void Append(std::initializer_list<Char> vInitList){
		Append(Observer(vInitList));
	}
	void Append(const String &rhs){
		if(&rhs == this){
			const std::size_t uOldLength = GetLength();
			const auto pchWrite = xChopAndSplice(uOldLength, uOldLength, 0, uOldLength * 2);
			Copy(pchWrite, pchWrite - uOldLength, pchWrite);
			xSetSize(uOldLength * 2);
		} else {
			Append(rhs.GetBegin(), rhs.GetEnd());
		}
	}
	void Append(String &&rhs){
		if(&rhs == this){
			Append(rhs);
		} else if(IsEmpty()){
			Assign(std::move(rhs));
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Append(rhs);
		} else {
			Swap(rhs);
			Unshift(rhs);
		}
	}
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	void Append(const String<OtherChar, OTHER_ENCODING> &rhs){
		Impl::Transcoder<Char, ENCODING, OtherChar, OTHER_ENCODING>()(*this, rhs);
	}
	template<StringEncoding OTHER_ENCODING, typename OtherChar>
	void Append(const StringObserver<OtherChar> &rhs){
		Impl::Transcoder<Char, ENCODING, OtherChar, OTHER_ENCODING>()(*this, rhs);
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		ASSERT_MSG(uCount <= GetLength(), L"删除的字符数太多。");

		xSetSize(GetLength() - uCount);
	}

	void Push(Char ch){
		Append(ch);
	}
	void Pop() noexcept {
		Truncate(1);
	}

	void PushNoCheck(Char ch) noexcept {
		ASSERT_MSG(GetLength() < GetCapacity(), L"容器已满。");

		if(xm_vStorage.chNull == Char()){
			xm_vStorage.achSmall[xm_vStorage.uchSmallLength] = ch;
			xm_vStorage.achSmall[++xm_vStorage.uchSmallLength] = Char();
		} else {
			xm_vStorage.pchLargeBegin[xm_vStorage.uLargeLength] = ch;
			xm_vStorage.pchLargeBegin[++xm_vStorage.uLargeLength] = Char();
		}
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(GetLength() != 0, L"容器已空。");

		if(xm_vStorage.chNull == Char()){
			xm_vStorage.achSmall[--xm_vStorage.uchSmallLength] = Char();
		} else {
			xm_vStorage.pchLargeBegin[--xm_vStorage.uLargeLength] = Char();
		}
	}

	void Unshift(Char ch, std::size_t uCount = 1){
		const std::size_t uOldLength = GetLength();
		FillN(xChopAndSplice(0, 0, 0, uCount), uCount, ch);
		xSetSize(uOldLength + uCount);
	}
	void Unshift(const Char *pszBegin){
		Unshift(Observer(pszBegin));
	}
	template<class Iterator>
	void Unshift(Iterator itBegin, Iterator itEnd){
		Unshift(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator>
	void Unshift(Iterator itBegin, std::size_t uLength){
		const std::size_t uOldLength = GetLength();
		CopyN(xChopAndSplice(0, 0, 0, uLength), itBegin, uLength);
		xSetSize(uOldLength + uLength);
	}
	void Unshift(const Observer &obs){
		Unshift(obs.GetBegin(), obs.GetEnd());
	}
	void Unshift(std::initializer_list<Char> vInitList){
		Unshift(Observer(vInitList));
	}
	void Unshift(const String &rhs){
		if(&rhs == this){
			Append(rhs);
		} else {
			Unshift(rhs.GetBegin(), rhs.GetEnd());
		}
	}
	void Unshift(String &&rhs){
		if(&rhs == this){
			Append(rhs);
		} else if(IsEmpty()){
			Assign(std::move(rhs));
		} else if(GetCapacity() >= rhs.GetCapacity()){
			Unshift(rhs);
		} else {
			Swap(rhs);
			Append(rhs);
		}
	}
	void Shift(std::size_t uCount = 1) noexcept {
		ASSERT_MSG(uCount <= GetLength(), L"删除的字符数太多。");

		const std::size_t uOldLength = GetLength();
		const auto pchBegin = GetBegin();
		Copy(pchBegin, pchBegin + uCount, pchBegin + uOldLength);
		xSetSize(uOldLength - uCount);
	}

	Observer Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().Slice(nBegin, nEnd);
	}
	String SliceStr(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		return String(Slice(nBegin, nEnd));
	}

	std::size_t Find(const Observer &obsToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().Find(obsToFind, nOffsetBegin);
	}
	std::size_t FindBackward(const Observer &obsToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindBackward(obsToFind, nOffsetEnd);
	}
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().FindRep(chToFind, uRepCount, nOffsetBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindRepBackward(chToFind, uRepCount, nOffsetEnd);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().Find(chToFind, nOffsetBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindBackward(chToFind, nOffsetEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chReplacement, std::size_t uCount = 1){
		const auto obsCurrent(GetObserver());
		const std::size_t uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(
			uRemovedBegin, uRemovedEnd,
			0, uRemovedBegin + uCount
		);
		FillN(pchWrite, uCount, chReplacement);
		xSetSize(uRemovedBegin + uCount + (uOldLength - uRemovedEnd));
	}
	template<class Iterator>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Iterator itRepBegin, Iterator itRepEnd){
		Replace(nBegin, nEnd, itRepBegin, (std::size_t)std::distance(itRepBegin, itRepEnd));
	}
	template<class Iterator>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Iterator itRepBegin, std::size_t uRepLen){
		const auto obsCurrent(GetObserver());
		const std::size_t uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		// 注意：不指向同一个数组的两个指针相互比较是未定义行为。
		if((uRepLen != 0) && ((std::uintptr_t)&*itRepBegin - (std::uintptr_t)obsCurrent.GetBegin() <= uOldLength * sizeof(Char))){
			// 待替换字符串和当前字符串重叠。
			String strTemp(*this);
			const auto pchWrite = strTemp.xChopAndSplice(
				uRemovedBegin, uRemovedEnd,
				0, uRemovedBegin + uRepLen
			);
			CopyN(pchWrite, itRepBegin, uRepLen);
			Swap(strTemp);
		} else {
			const auto pchWrite = xChopAndSplice(
				uRemovedBegin, uRemovedEnd,
				0, uRemovedBegin + uRepLen
			);
			CopyN(pchWrite, itRepBegin, uRepLen);
		}
		xSetSize(uRemovedBegin + uRepLen + (uOldLength - uRemovedEnd));
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
	typedef Char value_type;

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Push(std::forward<Param>(vParam));
	}
	// std::front_insert_iterator
	template<typename Param>
	void push_front(Param &&vParam){
		Unshift(std::forward<Param>(vParam));
	}

public:
	operator Observer() const noexcept {
		return GetObserver();
	}

	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const Char *() const noexcept {
		return GetStr();
	}
	explicit operator Char *() noexcept {
		return GetStr();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetStr()[uIndex];
	}
	Char &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetStr()[uIndex];
	}

	String &operator+=(const Observer &rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(Char rhs){
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
	template<typename OtherChar, StringEncoding OTHER_ENCODING>
	String &operator+=(const String<OtherChar, OTHER_ENCODING> &rhs){
		Append(rhs);
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

template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> operator+(const String<Char, ENCODING> &lhs, const StringObserver<Char> &rhs){
	String<Char, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> operator+(const String<Char, ENCODING> &lhs, Char rhs){
	String<Char, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + 1);
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char, StringEncoding ENCODING, typename OtherChar, StringEncoding OTHER_ENCODING>
String<Char, ENCODING> operator+(const String<Char, ENCODING> &lhs, const String<OtherChar, OTHER_ENCODING> &rhs){
	return lhs + rhs.GetObserver();
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> &&operator+(String<Char, ENCODING> &&lhs, const StringObserver<Char> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> &&operator+(String<Char, ENCODING> &&lhs, Char rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char, StringEncoding ENCODING, typename OtherChar, StringEncoding OTHER_ENCODING>
String<Char, ENCODING> &&operator+(String<Char, ENCODING> &&lhs, const String<OtherChar, OTHER_ENCODING> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}

template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> operator+(const StringObserver<Char> &lhs, const String<Char, ENCODING> &rhs){
	String<Char, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> operator+(Char lhs, const String<Char, ENCODING> &rhs){
	String<Char, ENCODING> strRet;
	strRet.Reserve(1 + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> &&operator+(const StringObserver<Char> &lhs, String<Char, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> &&operator+(Char lhs, String<Char, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char, StringEncoding ENCODING>
String<Char, ENCODING> &&operator+(String<Char, ENCODING> &&lhs, String<Char, ENCODING> &&rhs){
	if(lhs.GetCapacity() >= rhs.GetCapacity()){
		lhs.Append(rhs);
		return std::move(lhs);
	} else {
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
}

template<typename Comparand, typename Char, StringEncoding ENCODING>
bool operator==(const Comparand &lhs, const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetObserver() == lhs;
}
template<typename Comparand, typename Char, StringEncoding ENCODING>
bool operator!=(const Comparand &lhs, const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetObserver() != lhs;
}
template<typename Comparand, typename Char, StringEncoding ENCODING>
bool operator<(const Comparand &lhs, const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetObserver() > lhs;
}
template<typename Comparand, typename Char, StringEncoding ENCODING>
bool operator>(const Comparand &lhs, const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetObserver() < lhs;
}
template<typename Comparand, typename Char, StringEncoding ENCODING>
bool operator<=(const Comparand &lhs, const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetObserver() >= lhs;
}
template<typename Comparand, typename Char, StringEncoding ENCODING>
bool operator>=(const Comparand &lhs, const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetObserver() <= lhs;
}

template<typename Char, StringEncoding ENCODING>
const Char *begin(const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetBegin();
}
template<typename Char, StringEncoding ENCODING>
Char *begin(String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetBegin();
}
template<typename Char, StringEncoding ENCODING>
const Char *cbegin(const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetCBegin();
}

template<typename Char, StringEncoding ENCODING>
const Char *end(const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetEnd();
}
template<typename Char, StringEncoding ENCODING>
Char *end(String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetEnd();
}
template<typename Char, StringEncoding ENCODING>
const Char *cend(const String<Char, ENCODING> &rhs) noexcept {
	return rhs.GetCEnd();
}

namespace Impl {
	template<typename DstChar, StringEncoding DST_ENCODING, typename SrcChar, StringEncoding SRC_ENCODING>
	void Transcoder<DstChar, DST_ENCODING, SrcChar, SRC_ENCODING>::operator()(
		String<DstChar, DST_ENCODING> &strDst,
		const StringObserver<SrcChar> &soSrc
	) const {
		VVector<wchar_t> vecUnified;
		UnicodeConv<SrcChar, SRC_ENCODING>()(vecUnified, soSrc);
		UnicodeConv<DstChar, DST_ENCODING>()(strDst, vecUnified);
	}
	template<typename DstChar, StringEncoding ENCODING, typename SrcChar>
	void Transcoder<DstChar, ENCODING, SrcChar, ENCODING>::operator()(
		String<DstChar, ENCODING> &strDst,
		const StringObserver<SrcChar> &soSrc
	) const {
		strDst.Append(soSrc.GetBegin(), soSrc.GetSize());
	}
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

// 串行化。
class StreamBuffer;

extern void operator>>=(const Utf8String &u8sSource, StreamBuffer &sbufSink);
extern void operator<<=(Utf8String &u8sSink, StreamBuffer &sbufSource);

template<typename Char, StringEncoding ENCODING>
void operator>>=(const String<Char, ENCODING> &strSource, StreamBuffer &sbufSink){
	Utf8String(strSource) >>= sbufSink;
}
template<typename Char, StringEncoding ENCODING>
void operator<<=(String<Char, ENCODING> &strSink, StreamBuffer &sbufSource){
	Utf8String u8sTemp;
	u8sTemp <<= sbufSource;
	strSink.Assign(u8sTemp);
}

// 字面量运算符。
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char>::value, const AnsiString &>::type
	operator""_as()
{
	static AnsiString s_strRet({STRING...});
	return s_strRet;
}
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, wchar_t>::value, const WideString &>::type
	operator""_ws()
{
	static WideString s_strRet({STRING...});
	return s_strRet;
}

template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char>::value, const Utf8String &>::type
	operator""_u8s()
{
	static Utf8String s_strRet({STRING...});
	return s_strRet;
}
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char16_t>::value, const Utf16String &>::type
	operator""_u16s()
{
	static Utf16String s_strRet({STRING...});
	return s_strRet;
}
template<typename Char, Char... STRING>
typename std::enable_if<std::is_same<Char, char32_t>::value, const Utf32String &>::type
	operator""_u32s()
{
	static Utf32String s_strRet({STRING...});
	return s_strRet;
}

}

using ::MCF::operator""_as;
using ::MCF::operator""_ws;
using ::MCF::operator""_u8s;
using ::MCF::operator""_u16s;
using ::MCF::operator""_u32s;

#endif
