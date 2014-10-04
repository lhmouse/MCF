// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_HPP_
#define MCF_CORE_STRING_HPP_

#include "StringObserver.hpp"
#include "../Containers/VVector.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/BitsOf.hpp"
#include "../Utilities/CountLeadingTrailingZeroes.hpp"
#include "../Utilities/Algorithms.hpp"
#include "../Utilities/StaticAssertNoexcept.hpp"
#include <type_traits>
#include <memory>
#include <cstdint>

namespace MCF {

enum class StringEncoding {
	UTF8,
	ANSI,
	UTF16,
	UTF32
};

template<typename CharT, StringEncoding ENCODING_T>
class String;

namespace Impl {
	template<typename CharT, StringEncoding ENCODING_T>
	struct UnicodeConv {
		// 成员函数待定义。此处全部为追加到末尾。
		void operator()(VVector<wchar_t> &vecUnified, const StringObserver<CharT> &soSrc) const;
		void operator()(String<CharT, ENCODING_T> &strDst, const VVector<wchar_t> &vecUnified) const;
	};

	template<typename DstCharT, StringEncoding DST_ENCODING_T,
		typename SrcCharT, StringEncoding SRC_ENCODING_T>
	struct Transcoder {
		void operator()(String<DstCharT, DST_ENCODING_T> &strDst, const StringObserver<SrcCharT> &soSrc) const;
	};
	template<typename DstCharT, StringEncoding ENCODING_T,
		typename SrcCharT>
	struct Transcoder<DstCharT, ENCODING_T, SrcCharT, ENCODING_T> {
		void operator()(String<DstCharT, ENCODING_T> &strDst, const StringObserver<SrcCharT> &soSrc) const;
	};
}

template<typename CharT, StringEncoding ENCODING_T>
class String {
	static_assert(std::is_integral<CharT>::value, "CharT must be an integral type.");

	template<typename, StringEncoding>
	friend class String;

public:
	typedef StringObserver<CharT> Observer;

	enum : std::size_t {
		NPOS = Observer::NPOS
	};

private:
	union xStorage {
		struct __attribute__((__packed__)) {
			CharT achData[(4 * sizeof(void *)) / sizeof(CharT) - 2];
			CharT chNull;
			std::make_unsigned_t<CharT> uchLength;
		} vSmall;
		struct {
			CharT *pchBegin;
			std::size_t uLength;
			std::size_t uCapacity;
		} vLarge;
	} xm_vStorage;

public:
	String() noexcept {
		xm_vStorage.vSmall.achData[0]	= CharT();
		xm_vStorage.vSmall.chNull		= CharT();
		xm_vStorage.vSmall.uchLength	= 0;
	}
	explicit String(CharT ch, std::size_t uCount = 1)
		: String()
	{
		Assign(ch, uCount);
	}
	explicit String(const CharT *pszBegin)
		: String()
	{
		Assign(pszBegin);
	}
	template<class IteratorT>
	String(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: String()
	{
		Assign(itBegin, itEnd);
	}
	template<class IteratorT>
	String(IteratorT itBegin, std::size_t uLen)
		: String()
	{
		Assign(itBegin, uLen);
	}
	explicit String(const Observer &obs)
		: String()
	{
		Assign(obs);
	}
	explicit String(std::initializer_list<CharT> vInitList)
		: String()
	{
		Assign(vInitList);
	}
	template<typename SrcCharT, StringEncoding SRC_ENCODING_T>
	explicit String(const String<SrcCharT, SRC_ENCODING_T> &rhs)
		: String()
	{
		Assign(rhs);
	}
	template<typename SrcCharT, StringEncoding SRC_ENCODING_T>
	explicit String(String<SrcCharT, SRC_ENCODING_T> &&rhs)
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
	String &operator=(CharT ch) noexcept {
		Assign(ch);
		return *this;
	}
	String &operator=(const CharT *pszBegin){
		Assign(pszBegin);
		return *this;
	}
	String &operator=(const Observer &obs){
		Assign(obs);
		return *this;
	}
	String &operator=(std::initializer_list<CharT> vInitList){
		Assign(vInitList);
		return *this;
	}
	template<typename SrcCharT, StringEncoding SRC_ENCODING_T>
	String &operator=(const String<SrcCharT, SRC_ENCODING_T> &rhs){
		Assign(rhs);
		return *this;
	}
	template<typename SrcCharT, StringEncoding SRC_ENCODING_T>
	String &operator=(String<SrcCharT, SRC_ENCODING_T> &&rhs){
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
		if(xm_vStorage.vSmall.chNull != CharT()){
			delete[] xm_vStorage.vLarge.pchBegin;
		}

#ifndef NDEBUG
		std::memset(&xm_vStorage, 0xDD, sizeof(xm_vStorage));
#endif
	}

private:
	CharT *xChopAndSplice(
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
			pchNewBuffer = new CharT[uSizeToAlloc];
		}

	STATIC_ASSERT_NOEXCEPT_BEGIN
		if(uRemovedBegin != 0){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer,
				uRemovedBegin * sizeof(CharT));
		}
		if(uOldLength != uRemovedEnd){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd,
				(uOldLength - uRemovedEnd) * sizeof(CharT));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(xm_vStorage.vSmall.chNull == CharT()){
				xm_vStorage.vSmall.chNull = CharT() + 1;
			} else {
				delete[] pchOldBuffer;
			}

			xm_vStorage.vLarge.pchBegin = pchNewBuffer;
			xm_vStorage.vLarge.uLength = uOldLength;
			xm_vStorage.vLarge.uCapacity = uSizeToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	STATIC_ASSERT_NOEXCEPT_END
	}
	void xSetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(xm_vStorage.vSmall.chNull == CharT()){
			xm_vStorage.vSmall.uchLength = uNewSize;
			((CharT (&)[SIZE_MAX])xm_vStorage.vSmall.achData)[uNewSize] = CharT();
		} else {
			xm_vStorage.vLarge.uLength = uNewSize;
			xm_vStorage.vLarge.pchBegin[uNewSize] = CharT();
		}
	}

public:
	const CharT *GetBegin() const noexcept {
		if(xm_vStorage.vSmall.chNull == CharT()){
			return xm_vStorage.vSmall.achData;
		} else {
			return xm_vStorage.vLarge.pchBegin;
		}
	}
	CharT *GetBegin() noexcept {
		if(xm_vStorage.vSmall.chNull == CharT()){
			return xm_vStorage.vSmall.achData;
		} else {
			return xm_vStorage.vLarge.pchBegin;
		}
	}
	const CharT *GetCBegin() const noexcept {
		return GetBegin();
	}
	const CharT *GetEnd() const noexcept {
		if(xm_vStorage.vSmall.chNull == CharT()){
			return xm_vStorage.vSmall.achData + xm_vStorage.vSmall.uchLength;
		} else {
			return xm_vStorage.vLarge.pchBegin + xm_vStorage.vLarge.uLength;
		}
	}
	CharT *GetEnd() noexcept {
		if(xm_vStorage.vSmall.chNull == CharT()){
			return xm_vStorage.vSmall.achData + xm_vStorage.vSmall.uchLength;
		} else {
			return xm_vStorage.vLarge.pchBegin + xm_vStorage.vLarge.uLength;
		}
	}
	const CharT *GetCEnd() const noexcept {
		return GetEnd();
	}

	const CharT *GetStr() const noexcept {
		return GetBegin();
	}
	CharT *GetStr() noexcept {
		return GetBegin();
	}
	const CharT *GetCStr() const noexcept {
		return GetBegin();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const CharT *GetData() const noexcept {
		return GetBegin();
	}
	CharT *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}

	Observer GetObserver() const noexcept {
		return Observer(GetBegin(), GetEnd());
	}

	CharT *Resize(std::size_t uNewSize){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			xSetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Truncate(uOldSize - uNewSize);
		}
		return GetData();
	}
	CharT *ResizeFront(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		xChopAndSplice(uOldSize, uOldSize, uDeltaSize, uOldSize + uDeltaSize);
		xSetSize(uOldSize + uDeltaSize);
		return GetData();
	}
	CharT *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		xChopAndSplice(uOldSize, uOldSize, 0, uOldSize + uDeltaSize);
		xSetSize(uOldSize + uDeltaSize);
		return GetData() + uOldSize;
	}
	void Resize(std::size_t uNewSize, CharT ch){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Append(ch, uNewSize - uOldSize);
		} else if(uNewSize < uOldSize){
			Truncate(uOldSize - uNewSize);
		}
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
		if(xm_vStorage.vSmall.chNull == CharT()){
			return COUNT_OF(xm_vStorage.vSmall.achData);
		} else {
			return xm_vStorage.vLarge.uCapacity - 1;
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

	void Assign(CharT ch, std::size_t uCount = 1){
		Clear();
		Append(ch, uCount);
	}
	void Assign(const CharT *pszBegin){
		Assign(Observer(pszBegin));
	}
	template<class IteratorT>
	void Assign(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		Assign(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class IteratorT>
	void Assign(IteratorT itBegin, std::size_t uLength){
		Clear();
		Append(itBegin, uLength);
	}
	void Assign(const Observer &obs){
		Assign(obs.GetBegin(), obs.GetEnd());
	}
	void Assign(std::initializer_list<CharT> vInitList){
		Assign(Observer(vInitList));
	}
	void Assign(String &&rhs) noexcept {
		Swap(rhs);
	}
	template<typename SrcCharT, StringEncoding SRC_ENCODING_T>
	void Assign(const String<SrcCharT, SRC_ENCODING_T> &rhs){
		Clear();
		Append(rhs);
	}
	template<StringEncoding SRC_ENCODING_T, typename SrcCharT>
	void Assign(const StringObserver<SrcCharT> &rhs){
		Clear();
		Append<SRC_ENCODING_T, SrcCharT>(rhs);
	}

	void Append(CharT ch, std::size_t uCount = 1){
		const std::size_t uOldLength = GetLength();
		FillN(xChopAndSplice(uOldLength, uOldLength, 0, uOldLength + uCount), uCount, ch);
		xSetSize(uOldLength + uCount);
	}
	void Append(const CharT *pszBegin){
		Append(Observer(pszBegin));
	}
	template<class IteratorT>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		Append(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class IteratorT>
	void Append(IteratorT itBegin, std::size_t uLength){
		const std::size_t uOldLength = GetLength();
		CopyN(xChopAndSplice(uOldLength, uOldLength, 0, uOldLength + uLength), itBegin, uLength);
		xSetSize(uOldLength + uLength);
	}
	void Append(const Observer &obs){
		Append(obs.GetBegin(), obs.GetEnd());
	}
	void Append(std::initializer_list<CharT> vInitList){
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
	template<typename SrcCharT, StringEncoding SRC_ENCODING_T>
	void Append(const String<SrcCharT, SRC_ENCODING_T> &rhs){
		Impl::Transcoder<CharT, ENCODING_T, SrcCharT, SRC_ENCODING_T>()(*this, rhs);
	}
	template<StringEncoding SRC_ENCODING_T, typename SrcCharT>
	void Append(const StringObserver<SrcCharT> &rhs){
		Impl::Transcoder<CharT, ENCODING_T, SrcCharT, SRC_ENCODING_T>()(*this, rhs);
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		ASSERT_MSG(uCount <= GetLength(), L"删除的字符数太多。");

		xSetSize(GetLength() - uCount);
	}

	void Push(CharT ch){
		Append(ch);
	}
	void Pop() noexcept {
		Truncate(1);
	}

	void PushNoCheck(CharT ch) noexcept {
		ASSERT_MSG(GetLength() < GetCapacity(), L"容器已满。");

		if(xm_vStorage.vSmall.chNull == CharT()){
			xm_vStorage.vSmall.achData[xm_vStorage.vSmall.uchLength] = ch;
			xm_vStorage.vSmall.achData[++xm_vStorage.vSmall.uchLength] = CharT();
		} else {
			xm_vStorage.vLarge.pchBegin[xm_vStorage.vLarge.uLength] = ch;
			xm_vStorage.vLarge.pchBegin[++xm_vStorage.vLarge.uLength] = CharT();
		}
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(GetLength() != 0, L"容器已空。");

		if(xm_vStorage.vSmall.chNull == CharT()){
			xm_vStorage.vSmall.achData[--xm_vStorage.vSmall.uchLength] = CharT();
		} else {
			xm_vStorage.vLarge.pchBegin[--xm_vStorage.vLarge.uLength] = CharT();
		}
	}

	void Unshift(CharT ch, std::size_t uCount = 1){
		const std::size_t uOldLength = GetLength();
		FillN(xChopAndSplice(0, 0, 0, uCount), uCount, ch);
		xSetSize(uOldLength + uCount);
	}
	void Unshift(const CharT *pszBegin){
		Unshift(Observer(pszBegin));
	}
	template<class IteratorT>
	void Unshift(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		Unshift(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class IteratorT>
	void Unshift(IteratorT itBegin, std::size_t uLength){
		const std::size_t uOldLength = GetLength();
		CopyN(xChopAndSplice(0, 0, 0, uLength), itBegin, uLength);
		xSetSize(uOldLength + uLength);
	}
	void Unshift(const Observer &obs){
		Unshift(obs.GetBegin(), obs.GetEnd());
	}
	void Unshift(std::initializer_list<CharT> vInitList){
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

	std::size_t Find(const Observer &obsToFind,
		std::ptrdiff_t nOffsetBegin = 0) const noexcept
	{
		return GetObserver().Find(obsToFind, nOffsetBegin);
	}
	std::size_t FindBackward(const Observer &obsToFind,
		std::ptrdiff_t nOffsetEnd = -1) const noexcept
	{
		return GetObserver().FindBackward(obsToFind, nOffsetEnd);
	}
	std::size_t FindRep(CharT chToFind, std::size_t uRepCount,
		std::ptrdiff_t nOffsetBegin = 0) const noexcept
	{
		return GetObserver().FindRep(chToFind, uRepCount, nOffsetBegin);
	}
	std::size_t FindRepBackward(CharT chToFind, std::size_t uRepCount,
		std::ptrdiff_t nOffsetEnd = -1) const noexcept
	{
		return GetObserver().FindRepBackward(chToFind, uRepCount, nOffsetEnd);
	}
	std::size_t Find(CharT chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return GetObserver().Find(chToFind, nOffsetBegin);
	}
	std::size_t FindBackward(CharT chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return GetObserver().FindBackward(chToFind, nOffsetEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd,
		CharT chReplacement, std::size_t uCount = 1)
	{
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
	template<class IteratorT>
	void Replace(
		std::ptrdiff_t nBegin, std::ptrdiff_t nEnd,
		IteratorT itReplacementBegin, std::common_type_t<IteratorT> itReplacementEnd
	){
		Replace(nBegin, nEnd, itReplacementBegin,
			(std::size_t)std::distance(itReplacementBegin, itReplacementEnd));
	}
	template<class IteratorT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd,
		IteratorT itReplacementBegin, std::size_t uReplacementLen)
	{
		const auto obsCurrent(GetObserver());
		const std::size_t uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		// 注意：不指向同一个数组的两个指针相互比较是未定义行为。
		if(
			(uReplacementLen != 0) &&
			((std::uintptr_t)&*itReplacementBegin - (std::uintptr_t)obsCurrent.GetBegin()
				<= uOldLength * sizeof(CharT))
		){
			// 待替换字符串和当前字符串重叠。
			String strTemp(*this);
			const auto pchWrite = strTemp.xChopAndSplice(
				uRemovedBegin, uRemovedEnd,
				0, uRemovedBegin + uReplacementLen
			);
			CopyN(pchWrite, itReplacementBegin, uReplacementLen);
			Swap(strTemp);
		} else {
			const auto pchWrite = xChopAndSplice(
				uRemovedBegin, uRemovedEnd,
				0, uRemovedBegin + uReplacementLen
			);
			CopyN(pchWrite, itReplacementBegin, uReplacementLen);
		}
		xSetSize(uRemovedBegin + uReplacementLen + (uOldLength - uRemovedEnd));
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
	typedef CharT value_type;

	// std::back_insert_iterator
	template<typename ParamT>
	void push_back(ParamT &&vParam){
		Push(std::forward<ParamT>(vParam));
	}
	// std::front_insert_iterator
	template<typename ParamT>
	void push_front(ParamT &&vParam){
		Unshift(std::forward<ParamT>(vParam));
	}

public:
	operator Observer() const noexcept {
		return GetObserver();
	}

	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const CharT *() const noexcept {
		return GetStr();
	}
	explicit operator CharT *() noexcept {
		return GetStr();
	}
	const CharT &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetStr()[uIndex];
	}
	CharT &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetStr()[uIndex];
	}
};

template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &operator+=(
	String<CharT, ENCODING_T> &lhs,
	const StringObserver<CharT> &rhs
){
	lhs.Append(rhs);
	return lhs;
}
template<
	typename CharT, StringEncoding ENCODING_T,
	typename SrcCharT, StringEncoding SRC_ENCODING_T
>
String<CharT, ENCODING_T> &operator+=(
	String<CharT, ENCODING_T> &lhs,
	const String<SrcCharT, SRC_ENCODING_T> &rhs
){
	lhs.Append(rhs);
	return lhs;
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &operator+=(
	String<CharT, ENCODING_T> &lhs,
	CharT rhs
){
	lhs.Append(rhs);
	return lhs;
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+=(
	String<CharT, ENCODING_T> &&lhs,
	const StringObserver<CharT> &rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<
	typename CharT, StringEncoding ENCODING_T,
	typename SrcCharT, StringEncoding SRC_ENCODING_T
>
String<CharT, ENCODING_T> &&operator+=(
	String<CharT, ENCODING_T> &&lhs,
	const String<SrcCharT, SRC_ENCODING_T> &rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+=(
	String<CharT, ENCODING_T> &&lhs,
	CharT rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}

template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> operator+(
	const String<CharT, ENCODING_T> &lhs,
	const StringObserver<CharT> &rhs
){
	String<CharT, ENCODING_T> ret;
	ret.Reserve(lhs.GetSize() + rhs.GetSize());
	ret.Append(lhs);
	ret.Append(rhs);
	return std::move(ret);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> operator+(
	const StringObserver<CharT> &lhs,
	const String<CharT, ENCODING_T> &rhs
){
	String<CharT, ENCODING_T> ret;
	ret.Reserve(lhs.GetSize() + rhs.GetSize());
	ret.Append(lhs);
	ret.Append(rhs);
	return std::move(ret);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> operator+(
	const String<CharT, ENCODING_T> &lhs,
	CharT rhs
){
	String<CharT, ENCODING_T> ret;
	ret.Reserve(lhs.GetSize() + 1);
	ret.Append(lhs);
	ret.Append(rhs);
	return std::move(ret);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> operator+(
	CharT lhs,
	const String<CharT, ENCODING_T> &rhs
){
	String<CharT, ENCODING_T> ret;
	ret.Reserve(1 + rhs.GetSize());
	ret.Append(lhs);
	ret.Append(rhs);
	return std::move(ret);
}

template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+(
	String<CharT, ENCODING_T> &&lhs,
	const StringObserver<CharT> &rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+(
	const StringObserver<CharT> &lhs,
	String<CharT, ENCODING_T> &&rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+(
	String<CharT, ENCODING_T> &&lhs,
	CharT rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+(
	CharT lhs,
	String<CharT, ENCODING_T> &&rhs
){
	rhs.Unshift(lhs);
	return std::move(rhs);
}

template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> operator+(
	const String<CharT, ENCODING_T> &lhs,
	const String<CharT, ENCODING_T> &rhs
){
	String<CharT, ENCODING_T> ret;
	ret.Reserve(lhs.GetSize() + rhs.GetSize());
	ret.Append(lhs);
	ret.Append(rhs);
	return std::move(ret);
}
template<
	typename CharT, StringEncoding ENCODING_T,
	typename SrcCharT, StringEncoding SRC_ENCODING_T
>
String<CharT, ENCODING_T> operator+(
	const String<CharT, ENCODING_T> &lhs,
	const String<SrcCharT, SRC_ENCODING_T> &rhs
){
	String<CharT, ENCODING_T> ret(lhs);
	ret.Append(rhs);
	return std::move(ret);
}

template<typename CharT, StringEncoding ENCODING_T>
String<CharT, ENCODING_T> &&operator+(
	String<CharT, ENCODING_T> &&lhs,
	String<CharT, ENCODING_T> &&rhs
){
	if(lhs.GetCapacity() >= rhs.GetCapacity()){
		lhs.Append(rhs);
		return std::move(lhs);
	} else {
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
}
template<
	typename CharT, StringEncoding ENCODING_T,
	typename SrcCharT, StringEncoding SRC_ENCODING_T
>
String<CharT, ENCODING_T> operator+(
	String<CharT, ENCODING_T> &&lhs,
	const String<SrcCharT, SRC_ENCODING_T> &rhs
){
	lhs.Append(rhs);
	return std::move(lhs);
}

#define STRING_RATIONAL_OPERATOR_(op_type)	\
	template<typename CharT, StringEncoding ENCODING_T>	\
	bool operator op_type (	\
		const String<CharT, ENCODING_T> &lhs,	\
		const String<CharT, ENCODING_T> &rhs	\
	) noexcept {	\
		return lhs.GetObserver() op_type rhs.GetObserver();	\
	}	\
	template<typename CharT, StringEncoding ENCODING_T>	\
	bool operator op_type (	\
		const StringObserver<CharT> &lhs,	\
		const String<CharT, ENCODING_T> &rhs	\
	) noexcept {	\
		return lhs op_type rhs.GetObserver();	\
	}	\
	template<typename CharT, StringEncoding ENCODING_T>	\
	bool operator op_type (	\
		const String<CharT, ENCODING_T> &lhs,	\
		const StringObserver<CharT> &rhs	\
	) noexcept {	\
		return lhs.GetObserver() op_type rhs;	\
	}

STRING_RATIONAL_OPERATOR_(==)
STRING_RATIONAL_OPERATOR_(!=)
STRING_RATIONAL_OPERATOR_(<)
STRING_RATIONAL_OPERATOR_(>)
STRING_RATIONAL_OPERATOR_(<=)
STRING_RATIONAL_OPERATOR_(>=)

#undef STRING_RATIONAL_OPERATOR_

template<typename CharT, StringEncoding ENCODING_T>
const CharT *begin(const String<CharT, ENCODING_T> &rhs) noexcept {
	return rhs.GetBegin();
}
template<typename CharT, StringEncoding ENCODING_T>
CharT *begin(String<CharT, ENCODING_T> &rhs) noexcept {
	return rhs.GetBegin();
}
template<typename CharT, StringEncoding ENCODING_T>
const CharT *cbegin(const String<CharT, ENCODING_T> &rhs) noexcept {
	return rhs.GetCBegin();
}

template<typename CharT, StringEncoding ENCODING_T>
const CharT *end(const String<CharT, ENCODING_T> &rhs) noexcept {
	return rhs.GetEnd();
}
template<typename CharT, StringEncoding ENCODING_T>
CharT *end(String<CharT, ENCODING_T> &rhs) noexcept {
	return rhs.GetEnd();
}
template<typename CharT, StringEncoding ENCODING_T>
const CharT *cend(const String<CharT, ENCODING_T> &rhs) noexcept {
	return rhs.GetCEnd();
}

template<typename CharT, StringEncoding ENCODING_T>
void swap(String<CharT, ENCODING_T> &lhs, String<CharT, ENCODING_T> &rhs) noexcept {
	lhs.Swap(rhs);
}

namespace Impl {
	template<typename DstCharT, StringEncoding DST_ENCODING_T,
		typename SrcCharT, StringEncoding SRC_ENCODING_T>
	void Transcoder<DstCharT, DST_ENCODING_T, SrcCharT, SRC_ENCODING_T>::operator()(
		String<DstCharT, DST_ENCODING_T> &strDst,
		const StringObserver<SrcCharT> &soSrc
	) const {
		VVector<wchar_t> vecUnified;
		UnicodeConv<SrcCharT, SRC_ENCODING_T>()(vecUnified, soSrc);
		UnicodeConv<DstCharT, DST_ENCODING_T>()(strDst, vecUnified);
	}
	template<typename DstCharT, StringEncoding ENCODING_T, typename SrcCharT>
	void Transcoder<DstCharT, ENCODING_T, SrcCharT, ENCODING_T>::operator()(
		String<DstCharT, ENCODING_T> &strDst,
		const StringObserver<SrcCharT> &soSrc
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

template<typename CharT, StringEncoding ENCODING_T>
void operator>>=(const String<CharT, ENCODING_T> &strSource, StreamBuffer &sbufSink){
	Utf8String(strSource) >>= sbufSink;
}
template<typename CharT, StringEncoding ENCODING_T>
void operator<<=(String<CharT, ENCODING_T> &strSink, StreamBuffer &sbufSource){
	Utf8String u8sTemp;
	u8sTemp <<= sbufSource;
	strSink.Assign(u8sTemp);
}

// 字面量运算符。
template<typename CharT, CharT ...STRING_T>
extern inline
std::enable_if_t<std::is_same<CharT, char>::value, const AnsiString &>
	operator""_as()
{
	static AnsiString s_strRet({STRING_T...});
	return s_strRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline
std::enable_if_t<std::is_same<CharT, wchar_t>::value, const WideString &>
	operator""_ws()
{
	static WideString s_strRet({STRING_T...});
	return s_strRet;
}

template<typename CharT, CharT ...STRING_T>
extern inline
std::enable_if_t<std::is_same<CharT, char>::value, const Utf8String &>
	operator""_u8s()
{
	static Utf8String s_strRet({STRING_T...});
	return s_strRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline
std::enable_if_t<std::is_same<CharT, char16_t>::value, const Utf16String &>
	operator""_u16s()
{
	static Utf16String s_strRet({STRING_T...});
	return s_strRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline
std::enable_if_t<std::is_same<CharT, char32_t>::value, const Utf32String &>
	operator""_u32s()
{
	static Utf32String s_strRet({STRING_T...});
	return s_strRet;
}

}

using ::MCF::operator""_as;
using ::MCF::operator""_ws;
using ::MCF::operator""_u8s;
using ::MCF::operator""_u16s;
using ::MCF::operator""_u32s;

#endif
