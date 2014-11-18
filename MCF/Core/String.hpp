// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_HPP_
#define MCF_CORE_STRING_HPP_

#include "StringObserver.hpp"
#include "Exception.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/BitsOf.hpp"
#include "../Utilities/CountLeadingTrailingZeroes.hpp"
#include "../Utilities/Algorithms.hpp"
#include <type_traits>
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<StringTypes TYPE_T>
class String;

using UnifiedString = String<StringTypes::UTF32>;
using UnifiedStringObserver = StringObserver<StringTypes::UTF32>;

template<StringTypes TYPE_T>
class String {
public:
	using Observer = StringObserver<TYPE_T>;

	static constexpr StringTypes Type = TYPE_T;
	using Char = typename Observer::Char;

	static constexpr std::size_t NPOS = Observer::NPOS;

public:
	static UnifiedStringObserver Unify(UnifiedString &&usTempStorage, const Observer &obsSrc);
	static void Deunify(String &strDst, std::size_t uPos, const UnifiedStringObserver &usoSrc);

private:
	union xStorage {
		struct {
			Char achData[(4 * sizeof(void *)) / sizeof(Char) - 2];
			Char chNull;
			std::make_unsigned_t<Char> uchLength;
		} vSmall;

		struct {
			Char *pchBegin;
			std::size_t uLength;
			std::size_t uCapacity;
		} vLarge;
	} xm_vStorage;

public:
	String() noexcept {
		xm_vStorage.vSmall.chNull = Char();
		xm_vStorage.vSmall.uchLength = 0;
	}
	explicit String(Char ch, std::size_t uCount = 1)
		: String()
	{
		Append(ch, uCount);
	}
	explicit String(const Char *pszBegin)
		: String()
	{
		Append(pszBegin);
	}
	template<class IteratorT>
	String(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: String()
	{
		Append(std::move(itBegin), itEnd);
	}
	template<class IteratorT>
	String(IteratorT itBegin, std::size_t uLen)
		: String()
	{
		Append(std::move(itBegin), uLen);
	}
	explicit String(const Observer &rhs)
		: String()
	{
		Append(rhs);
	}
	explicit String(std::initializer_list<Char> rhs)
		: String()
	{
		Append(rhs);
	}
	String(const String &rhs)
		: String()
	{
		Append(rhs);
	}
	String(String &&rhs) noexcept
		: String()
	{
		Swap(rhs);
	}
	template<StringTypes OtherTypeT>
	explicit String(const StringObserver<OtherTypeT> &rhs)
		: String()
	{
		Append(rhs);
	}
	template<StringTypes OtherTypeT>
	explicit String(const String<OtherTypeT> &rhs)
		: String()
	{
		Append(rhs);
	}
	String &operator=(Char ch) noexcept {
		Assign(ch);
		return *this;
	}
	String &operator=(const Char *pszBegin){
		Assign(pszBegin);
		return *this;
	}
	String &operator=(const Observer &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(std::initializer_list<Char> rhs){
		Assign(rhs);
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
	template<StringTypes OtherTypeT>
	String &operator=(const StringObserver<OtherTypeT> &rhs){
		Assign(rhs);
		return *this;
	}
	template<StringTypes OtherTypeT>
	String &operator=(const String<OtherTypeT> &rhs){
		Assign(rhs);
		return *this;
	}
	~String() noexcept {
		if(xm_vStorage.vSmall.chNull != Char()){
			delete[] xm_vStorage.vLarge.pchBegin;
		}
#ifndef NDEBUG
		std::memset(&xm_vStorage, 0xDD, sizeof(xm_vStorage));
#endif
	}

private:
	Char *xChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd,
		std::size_t uFirstOffset, std::size_t uThirdOffset)
	{
		const auto pchOldBuffer = GetBegin();
		const auto uOldLength = GetLength();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewLength = uThirdOffset + (uOldLength - uRemovedEnd);
		std::size_t uSizeToAlloc = uNewLength + 1;

		ASSERT(uRemovedBegin <= uOldLength);
		ASSERT(uRemovedEnd <= uOldLength);
		ASSERT(uRemovedBegin <= uRemovedEnd);
		ASSERT(uFirstOffset + uRemovedBegin <= uThirdOffset);

		if(GetCapacity() < uNewLength){
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & (std::size_t)-0x10;
			if(uSizeToAlloc < uNewLength + 1){
				throw std::bad_alloc();
			}
			pchNewBuffer = new Char[uSizeToAlloc];
		}

		if(uRemovedBegin != 0){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if(uOldLength != uRemovedEnd){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(xm_vStorage.vSmall.chNull == Char()){
				++xm_vStorage.vSmall.chNull;
			} else {
				delete[] pchOldBuffer;
			}

			xm_vStorage.vLarge.pchBegin = pchNewBuffer;
			xm_vStorage.vLarge.uLength = uOldLength;
			xm_vStorage.vLarge.uCapacity = uSizeToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void xSetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(xm_vStorage.vSmall.chNull == Char()){
			xm_vStorage.vSmall.uchLength = uNewSize;
		} else {
			xm_vStorage.vLarge.uLength = uNewSize;
		}
	}

public:
	const Char *GetBegin() const noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
			return xm_vStorage.vSmall.achData;
		} else {
			return xm_vStorage.vLarge.pchBegin;
		}
	}
	Char *GetBegin() noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
			return xm_vStorage.vSmall.achData;
		} else {
			return xm_vStorage.vLarge.pchBegin;
		}
	}
	const Char *GetCBegin() const noexcept {
		return GetBegin();
	}

	const Char *GetEnd() const noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
			return xm_vStorage.vSmall.achData + xm_vStorage.vSmall.uchLength;
		} else {
			return xm_vStorage.vLarge.pchBegin + xm_vStorage.vLarge.uLength;
		}
	}
	Char *GetEnd() noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
			return xm_vStorage.vSmall.achData + xm_vStorage.vSmall.uchLength;
		} else {
			return xm_vStorage.vLarge.pchBegin + xm_vStorage.vLarge.uLength;
		}
	}
	const Char *GetCEnd() const noexcept {
		return GetEnd();
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
			return xm_vStorage.vSmall.uchLength;
		} else {
			return xm_vStorage.vLarge.uLength;
		}
	}

	const Char *GetStr() const noexcept {
		const_cast<Char &>(GetEnd()[0]) = Char();
		return GetBegin();
	}
	Char *GetStr() noexcept {
		GetEnd()[0] = Char();
		return GetBegin();
	}
	const Char *GetCStr() const noexcept {
		return GetStr();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	Observer GetObserver() const noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
			return Observer(xm_vStorage.vSmall.achData, xm_vStorage.vSmall.uchLength);
		} else {
			return Observer(xm_vStorage.vLarge.pchBegin, xm_vStorage.vLarge.uLength);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(xm_vStorage.vSmall.chNull == Char()){
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
	Char *ResizeFront(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		xChopAndSplice(uOldSize, uOldSize, uDeltaSize, uOldSize + uDeltaSize);
		xSetSize(uOldSize + uDeltaSize);
		return GetData();
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		xChopAndSplice(uOldSize, uOldSize, 0, uOldSize + uDeltaSize);
		xSetSize(uOldSize + uDeltaSize);
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

	void Swap(String &rhs) noexcept {
		std::swap(xm_vStorage, rhs.xm_vStorage);
	}

	int Compare(const Observer &rhs) const noexcept {
		return GetObserver().Compare(rhs);
	}
	int Compare(const String &rhs) const noexcept {
		return GetObserver().Compare(rhs.GetObserver());
	}

	void Assign(Char ch, std::size_t uCount = 1){
		Clear();
		Append(ch, uCount);
	}
	void Assign(const Char *pszBegin){
		Clear();
		Append(Observer(pszBegin));
	}
	template<class IteratorT>
	void Assign(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		Clear();
		Append(std::move(itBegin), std::move(itEnd));
	}
	template<class IteratorT>
	void Assign(IteratorT itBegin, std::size_t uLen){
		Clear();
		Append(std::move(itBegin), uLen);
	}
	void Assign(const Observer &rhs){
		ASSERT(!GetObserver().DoesOverlapWith(rhs));
		Clear();
		Append(rhs);
	}
	void Assign(std::initializer_list<Char> rhs){
		Clear();
		Append(rhs);
	}
	template<StringTypes OtherTypeT>
	void Assign(const StringObserver<OtherTypeT> &rhs){
		Clear();
		Append(rhs);
	}
	template<StringTypes OtherTypeT>
	void Assign(const String<OtherTypeT> &rhs){
		Clear();
		Append(rhs);
	}
	void Assign(const String &rhs){
		if(&rhs != this){
			Clear();
			Append(rhs);
		}
	}
	void Assign(String &&rhs) noexcept {
		Swap(rhs);
	}

	void Append(Char ch, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, ch);
	}
	void Append(const Char *pszBegin){
		Append(Observer(pszBegin));
	}
	template<class IteratorT>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		Append(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class IteratorT>
	void Append(IteratorT itBegin, std::size_t uCount){
		CopyN(ResizeMore(uCount), itBegin, uCount);
	}
	void Append(const Observer &rhs){
		ASSERT(!GetObserver().DoesOverlapWith(rhs));
		Copy(ResizeMore(rhs.GetSize()), rhs.GetBegin(), rhs.GetEnd());
	}
	void Append(std::initializer_list<Char> rhs){
		Append(Observer(rhs));
	}
	void Append(const String &rhs){
		if(&rhs == this){
			const auto uSize = GetSize();
			const auto pchWrite = ResizeMore(uSize);
			Copy(pchWrite, pchWrite - uSize, pchWrite);
		} else {
			Append(Observer(rhs));
		}
	}
	void Append(String &&rhs){
		const Observer obsToAppend(rhs);
		const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
		if(GetCapacity() >= uSizeTotal){
			Append(obsToAppend);
		} else if(rhs.GetCapacity() >= uSizeTotal){
			rhs.Unshift(obsToAppend);
			Swap(rhs);
		} else {
			Append(obsToAppend);
		}
	}
	template<StringTypes OtherTypeT>
	void Append(const StringObserver<OtherTypeT> &rhs){
		Deunify(*this, GetSize(), String<OtherTypeT>::Unify(UnifiedString(), rhs));
	}
	template<StringTypes OtherTypeT>
	void Append(const String<OtherTypeT> &rhs){
		Append(rhs.GetObserver());
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		const auto uOldLength = GetLength();
		ASSERT_MSG(uCount <= uOldLength, L"删除的字符数太多。");
		xSetSize(uOldLength - uCount);
	}

	void Push(Char ch){
		Append(ch, 1);
	}
	void Pop() noexcept {
		Truncate(1);
	}

	void PushNoCheck(Char ch) noexcept {
		ASSERT_MSG(GetLength() < GetCapacity(), L"容器已满。");

		if(xm_vStorage.vSmall.chNull == Char()){
			xm_vStorage.vSmall.achData[xm_vStorage.vSmall.uchLength] = ch;
		} else {
			xm_vStorage.vLarge.pchBegin[xm_vStorage.vLarge.uLength] = ch;
		}
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(GetLength() != 0, L"容器已空。");

		if(xm_vStorage.vSmall.chNull == Char()){
			--xm_vStorage.vSmall.uchLength;
		} else {
			--xm_vStorage.vLarge.uLength;
		}
	}

	void Unshift(Char ch, std::size_t uCount = 1){
		FillN(ResizeFront(uCount), uCount, ch);
	}
	void Unshift(const Char *pszBegin){
		Unshift(Observer(pszBegin));
	}
	template<class IteratorT>
	void Unshift(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		Unshift(itBegin, (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class IteratorT>
	void Unshift(IteratorT itBegin, std::size_t uCount){
		CopyN(ResizeFront(uCount), itBegin, uCount);
	}
	void Unshift(const Observer &obs){
		Copy(ResizeFront(obs.GetSize()), obs.GetBegin(), obs.GetEnd());
	}
	void Unshift(std::initializer_list<Char> rhs){
		Unshift(Observer(rhs));
	}
	void Unshift(const String &rhs){
		if(&rhs == this){
			Append(*this);
		} else {
			Unshift(Observer(rhs));
		}
	}
	void Unshift(String &&rhs){
		const Observer obsToAppend(rhs);
		const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
		if(GetCapacity() >= uSizeTotal){
			Unshift(obsToAppend);
		} else if(rhs.GetCapacity() >= uSizeTotal){
			rhs.Append(obsToAppend);
			Swap(rhs);
		} else {
			Unshift(obsToAppend);
		}
	}
	template<StringTypes OtherTypeT>
	void Unshift(const StringObserver<OtherTypeT> &rhs){
		Deunify(*this, 0, String<OtherTypeT>::Unify(UnifiedString(), rhs));
	}
	template<StringTypes OtherTypeT>
	void Unshift(const String<OtherTypeT> &rhs){
		Unshift(rhs.GetObserver());
	}
	void Shift(std::size_t uCount = 1) noexcept {
		const auto uOldLength = GetLength();
		ASSERT_MSG(uCount <= uOldLength, L"删除的字符数太多。");
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

	void Reverse() noexcept {
		auto pchBegin = GetBegin();
		auto pchEnd = GetEnd();
		if(pchBegin != pchEnd){
			--pchEnd;
			while(pchBegin < pchEnd){
				std::iter_swap(pchBegin++, pchEnd--);
			}
		}
	}

	std::size_t Find(const Observer &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().Find(obsToFind, nBegin);
	}
	std::size_t FindBackward(const Observer &obsToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().FindBackward(obsToFind, nEnd);
	}
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().FindRep(chToFind, uRepCount, nBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().FindRepBackward(chToFind, uRepCount, nEnd);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().Find(chToFind, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().FindBackward(chToFind, nEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chRep, std::size_t uCount = 1){
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uCount);
		FillN(pchWrite, uCount, chRep);
		xSetSize(uRemovedBegin + uCount + (uOldLength - uRemovedEnd));
	}
	template<class IteratorT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, IteratorT itRepBegin, std::common_type_t<IteratorT> itRepEnd){
		Replace(nBegin, nEnd, std::move(itRepBegin), (std::size_t)std::distance(itRepBegin, itRepEnd));
	}
	template<class IteratorT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, IteratorT itRepBegin, std::size_t uRepLen){
		// 基本异常安全保证。
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uRepLen);
		CopyN(pchWrite, itRepBegin, uRepLen);
		xSetSize(uRemovedBegin + uRepLen + (uOldLength - uRemovedEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Observer &obsRep){
		Replace(nBegin, nEnd, obsRep.GetBegin(), obsRep.GetSize());
	}
	template<StringTypes OtherTypeT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const StringObserver<OtherTypeT> &obsRep){
		// 基本异常安全保证。
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin);
		xSetSize(uRemovedBegin + (uOldLength - uRemovedEnd));
		Deunify(*this, uRemovedBegin, String<OtherTypeT>::Unify(UnifiedString(), obsRep));
	}
	template<StringTypes OtherTypeT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const String<OtherTypeT> &strRep){
		Replace(nBegin, nEnd, strRep.GetObserver());
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
		return GetBegin()[uIndex];
	}
	Char &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");
		return GetBegin()[uIndex];
	}

public:
	using value_type = Char;

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
};

template<StringTypes TYPE_T, StringTypes OtherTypeT>
String<TYPE_T> &operator+=(String<TYPE_T> &lhs, const StringObserver<OtherTypeT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringTypes TYPE_T>
String<TYPE_T> &operator+=(String<TYPE_T> &lhs, typename String<TYPE_T>::Char rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringTypes TYPE_T, StringTypes OtherTypeT>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, const StringObserver<OtherTypeT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringTypes TYPE_T>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, typename String<TYPE_T>::Char rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringTypes TYPE_T>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, String<TYPE_T> &&rhs){
	lhs.Append(std::move(rhs));
	return std::move(lhs);
}

template<StringTypes TYPE_T, StringTypes OtherTypeT>
String<TYPE_T> operator+(const String<TYPE_T> &lhs, const StringObserver<OtherTypeT> &rhs){
	return std::move(String<TYPE_T>(lhs) += rhs);
}
template<StringTypes TYPE_T>
String<TYPE_T> operator+(const String<TYPE_T> &lhs, typename String<TYPE_T>::Char rhs){
	return std::move(String<TYPE_T>(lhs) += rhs);
}
template<StringTypes TYPE_T, StringTypes OtherTypeT>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, const StringObserver<OtherTypeT> &rhs){
	return std::move(lhs += rhs);
}
template<StringTypes TYPE_T>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, typename String<TYPE_T>::Char rhs){
	return std::move(lhs += rhs);
}
template<StringTypes TYPE_T>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, String<TYPE_T> &&rhs){
	return std::move(lhs += std::move(rhs));
}

template<StringTypes TYPE_T>
bool operator==(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() == rhs.GetObserver();
}
template<StringTypes TYPE_T>
bool operator==(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() == rhs;
}
template<StringTypes TYPE_T>
bool operator==(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs == rhs.GetObserver();
}

template<StringTypes TYPE_T>
bool operator!=(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() != rhs.GetObserver();
}
template<StringTypes TYPE_T>
bool operator!=(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() != rhs;
}
template<StringTypes TYPE_T>
bool operator!=(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs != rhs.GetObserver();
}

template<StringTypes TYPE_T>
bool operator<(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() < rhs.GetObserver();
}
template<StringTypes TYPE_T>
bool operator<(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() < rhs;
}
template<StringTypes TYPE_T>
bool operator<(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs < rhs.GetObserver();
}

template<StringTypes TYPE_T>
bool operator>(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() > rhs.GetObserver();
}
template<StringTypes   TYPE_T>
bool operator>(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() > rhs;
}
template<StringTypes TYPE_T>
bool operator>(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs > rhs.GetObserver();
}

template<StringTypes TYPE_T>
bool operator<=(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() <= rhs.GetObserver();
}
template<StringTypes TYPE_T>
bool operator<=(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() <= rhs;
}
template<StringTypes TYPE_T>
bool operator<=(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs <= rhs.GetObserver();
}

template<StringTypes TYPE_T>
bool operator>=(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() >= rhs.GetObserver();
}
template<StringTypes TYPE_T>
bool operator>=(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() >= rhs;
}
template<StringTypes TYPE_T>
bool operator>=(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs >= rhs.GetObserver();
}

template<StringTypes TYPE_T>
auto begin(const String<TYPE_T> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringTypes TYPE_T>
auto begin(String<TYPE_T> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringTypes TYPE_T>
auto cbegin(const String<TYPE_T> &rhs) noexcept {
	return rhs.GetBegin();
}

template<StringTypes TYPE_T>
auto end(const String<TYPE_T> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringTypes TYPE_T>
auto end(String<TYPE_T> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringTypes TYPE_T>
auto cend(const String<TYPE_T> &rhs) noexcept {
	return rhs.GetEnd();
}

template<StringTypes TYPE_T>
void swap(String<TYPE_T> &lhs, String<TYPE_T> &rhs) noexcept {
	lhs.Swap(rhs);
}

extern template class String<StringTypes::NARROW>;
extern template class String<StringTypes::WIDE>;
extern template class String<StringTypes::UTF8>;
extern template class String<StringTypes::UTF16>;
extern template class String<StringTypes::UTF32>;
extern template class String<StringTypes::CESU8>;
extern template class String<StringTypes::ANSI>;

using NarrowString		= String<StringTypes::NARROW>;
using WideString		= String<StringTypes::WIDE>;
using Utf8String		= String<StringTypes::UTF8>;
using Utf16String		= String<StringTypes::UTF16>;
using Utf32String		= String<StringTypes::UTF32>;
using Cesu8String		= String<StringTypes::CESU8>;
using AnsiString		= String<StringTypes::ANSI>;

class StringEncodingError : public Exception {
public:
	StringEncodingError(const char *pszFile, unsigned long ulLine,
		const char *pszMessage, unsigned long ulErrorCode) noexcept
		: Exception(pszFile, ulLine, pszMessage, ulErrorCode)
	{
	}
	~StringEncodingError() override;
};

// 字面量运算符。
template<typename CharT, CharT ...STRING_T>
[[deprecated("Be warned that the encoding of narrow string literals varies from compilers to compilers "
	"and even depends on the encoding of source files on g++.")]]
extern inline const auto &operator""_ns(){
	static const NarrowString s_nsRet{ STRING_T... };
	return s_nsRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline const auto &operator""_ws(){
	static const WideString s_wsRet{ STRING_T... };
	return s_wsRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline const auto &operator""_u8s(){
	static const Utf8String s_u8sRet{ STRING_T... };
	return s_u8sRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline const auto &operator""_u16s(){
	static const Utf16String s_u16sRet{ STRING_T... };
	return s_u16sRet;
}
template<typename CharT, CharT ...STRING_T>
extern inline const auto &operator""_u32s(){
	static const Utf32String s_u32sRet{ STRING_T... };
	return s_u32sRet;
}

}

using ::MCF::operator""_ns;
using ::MCF::operator""_ws;
using ::MCF::operator""_u8s;
using ::MCF::operator""_u16s;
using ::MCF::operator""_u32s;

#endif
