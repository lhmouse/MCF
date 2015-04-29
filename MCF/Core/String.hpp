// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_HPP_
#define MCF_CORE_STRING_HPP_

#include "StringObserver.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/BitsOf.hpp"
#include "../Utilities/CountLeadingTrailingZeroes.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include <type_traits>
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<StringType TYPE_T>
	class String;

using UnifiedString = String<StringType::UTF32>;
using UnifiedStringObserver = StringObserver<StringType::UTF32>;

template<StringType TYPE_T>
class String {
public:
	using ObserverType = StringObserver<TYPE_T>;

	static constexpr StringType Type = TYPE_T;
	using CharType = typename ObserverType::CharType;

	static constexpr std::size_t NPOS = ObserverType::NPOS;

public:
	static UnifiedStringObserver Unify(UnifiedString &usTempStorage, const ObserverType &obsSrc);
	static void Deunify(String &strDst, std::size_t uPos, const UnifiedStringObserver &usoSrc);

private:
	union xStorage {
		struct {
			CharType achData[(4 * sizeof(void *)) / sizeof(CharType) - 2];
			CharType chNull;
			std::make_unsigned_t<CharType> uchLength;
		} vSmall;

		struct {
			CharType *pchBegin;
			std::size_t uLength;
			std::size_t uCapacity;
		} vLarge;
	} x_vStorage;

public:
	String() noexcept {
		x_vStorage.vSmall.chNull = CharType();
		x_vStorage.vSmall.uchLength = 0;
	}
	explicit String(CharType ch, std::size_t uCount = 1)
		: String()
	{
		Append(ch, uCount);
	}
	explicit String(const CharType *pszBegin)
		: String()
	{
		Append(pszBegin);
	}
	String(const CharType *pchBegin, const CharType *pchEnd)
		: String()
	{
		Append(pchBegin, pchEnd);
	}
	String(const CharType *pchBegin, std::size_t uLen)
		: String()
	{
		Append(pchBegin, uLen);
	}
	explicit String(const ObserverType &rhs)
		: String()
	{
		Append(rhs);
	}
	explicit String(std::initializer_list<CharType> rhs)
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
	template<StringType OTHER_TYPE_T>
	explicit String(const StringObserver<OTHER_TYPE_T> &rhs)
		: String()
	{
		Append(rhs);
	}
	template<StringType OTHER_TYPE_T>
	explicit String(const String<OTHER_TYPE_T> &rhs)
		: String()
	{
		Append(rhs);
	}
	String &operator=(CharType ch) noexcept {
		Assign(ch);
		return *this;
	}
	String &operator=(const CharType *pszBegin){
		Assign(pszBegin);
		return *this;
	}
	String &operator=(const ObserverType &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(std::initializer_list<CharType> rhs){
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
	template<StringType OTHER_TYPE_T>
	String &operator=(const StringObserver<OTHER_TYPE_T> &rhs){
		Assign(rhs);
		return *this;
	}
	template<StringType OTHER_TYPE_T>
	String &operator=(const String<OTHER_TYPE_T> &rhs){
		Assign(rhs);
		return *this;
	}
	~String() noexcept {
		if(x_vStorage.vSmall.chNull != CharType()){
			delete[] x_vStorage.vLarge.pchBegin;
		}
#ifndef NDEBUG
		std::memset(&x_vStorage, 0xDD, sizeof(x_vStorage));
#endif
	}

private:
	CharType *xChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd,
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
			uSizeToAlloc = (uSizeToAlloc + 0x0F) & (std::size_t)-0x10;
			if(uSizeToAlloc < uNewLength + 1){
				uSizeToAlloc = uNewLength + 1;
			}
			pchNewBuffer = new CharType[uSizeToAlloc];
		}

		if((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(CharType));
		}
		if((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldLength != uRemovedEnd)){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(CharType));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(x_vStorage.vSmall.chNull == CharType()){
				++x_vStorage.vSmall.chNull;
			} else {
				delete[] pchOldBuffer;
			}

			x_vStorage.vLarge.pchBegin = pchNewBuffer;
			x_vStorage.vLarge.uLength = uOldLength;
			x_vStorage.vLarge.uCapacity = uSizeToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void xSetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(x_vStorage.vSmall.chNull == CharType()){
			x_vStorage.vSmall.uchLength = uNewSize;
		} else {
			x_vStorage.vLarge.uLength = uNewSize;
		}
	}

public:
	const CharType *GetBegin() const noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return x_vStorage.vSmall.achData;
		} else {
			return x_vStorage.vLarge.pchBegin;
		}
	}
	CharType *GetBegin() noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return x_vStorage.vSmall.achData;
		} else {
			return x_vStorage.vLarge.pchBegin;
		}
	}

	const CharType *GetEnd() const noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return x_vStorage.vSmall.achData + x_vStorage.vSmall.uchLength;
		} else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}
	CharType *GetEnd() noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return x_vStorage.vSmall.achData + x_vStorage.vSmall.uchLength;
		} else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}

	const CharType *GetData() const noexcept {
		return GetBegin();
	}
	CharType *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return x_vStorage.vSmall.uchLength;
		} else {
			return x_vStorage.vLarge.uLength;
		}
	}

	const CharType *GetStr() const noexcept {
		const_cast<CharType &>(GetEnd()[0]) = CharType();
		return GetBegin();
	}
	CharType *GetStr() noexcept {
		GetEnd()[0] = CharType();
		return GetBegin();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	ObserverType GetObserver() const noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return ObserverType(x_vStorage.vSmall.achData, x_vStorage.vSmall.uchLength);
		} else {
			return ObserverType(x_vStorage.vLarge.pchBegin, x_vStorage.vLarge.uLength);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(x_vStorage.vSmall.chNull == CharType()){
			return COUNT_OF(x_vStorage.vSmall.achData);
		} else {
			return x_vStorage.vLarge.uCapacity - 1;
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

	void Resize(std::size_t uNewSize){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			xSetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Truncate(uOldSize - uNewSize);
		}
	}
	CharType *ResizeFront(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		xChopAndSplice(uOldSize, uOldSize, uDeltaSize, uOldSize + uDeltaSize);
		xSetSize(uOldSize + uDeltaSize);
		return GetData();
	}
	CharType *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		xChopAndSplice(uOldSize, uOldSize, 0, uOldSize + uDeltaSize);
		xSetSize(uOldSize + uDeltaSize);
		return GetData() + uOldSize;
	}
	void Shrink() noexcept {
		const auto uSzLen = ObserverType(GetStr()).GetLength();
		ASSERT(uSzLen <= GetSize());
		xSetSize(uSzLen);
	}

	bool IsEmpty() const noexcept {
		return GetBegin() == GetEnd();
	}
	void Clear() noexcept {
		xSetSize(0);
	}

	void Swap(String &rhs) noexcept {
		xStorage vStorage;
		std::memcpy(&vStorage, &x_vStorage, sizeof(vStorage));
		std::memcpy(&x_vStorage, &rhs.x_vStorage, sizeof(vStorage));
		std::memcpy(&rhs.x_vStorage, &vStorage, sizeof(vStorage));
	}

	int Compare(const ObserverType &rhs) const noexcept {
		return GetObserver().Compare(rhs);
	}
	int Compare(const String &rhs) const noexcept {
		return GetObserver().Compare(rhs.GetObserver());
	}

	void Assign(CharType ch, std::size_t uCount = 1){
		Resize(uCount);
		FillN(GetStr(), uCount, ch);
	}
	void Assign(const CharType *pszBegin){
		Assign(ObserverType(pszBegin));
	}
	void Assign(const CharType *pchBegin, const CharType *pchEnd){
		Assign(ObserverType(pchBegin, pchEnd));
	}
	void Assign(const CharType *pchBegin, std::size_t uCount){
		Assign(ObserverType(pchBegin, uCount));
	}
	void Assign(const ObserverType &rhs){
		Resize(rhs.GetSize());
		Copy(GetStr(), rhs.GetBegin(), rhs.GetEnd());
	}
	void Assign(std::initializer_list<CharType> rhs){
		Assign(ObserverType(rhs));
	}
	template<StringType OTHER_TYPE_T>
	void Assign(const StringObserver<OTHER_TYPE_T> &rhs){
		Clear();
		Append(rhs);
	}
	template<StringType OTHER_TYPE_T>
	void Assign(const String<OTHER_TYPE_T> &rhs){
		Assign(StringObserver<OTHER_TYPE_T>(rhs));
	}
	void Assign(const String &rhs){
		if(&rhs != this){
			Assign(ObserverType(rhs));
		}
	}
	void Assign(String &&rhs) noexcept {
		ASSERT(this != &rhs);
		Swap(rhs);
	}

	void Append(CharType ch, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, ch);
	}
	void Append(const CharType *pszBegin){
		Append(ObserverType(pszBegin));
	}
	void Append(const CharType *pchBegin, const CharType *pchEnd){
		Append(ObserverType(pchBegin, pchEnd));
	}
	void Append(const CharType *pchBegin, std::size_t uCount){
		Append(ObserverType(pchBegin, uCount));
	}
	void Append(const ObserverType &rhs){
		Replace(-1, -1, rhs);
	}
	void Append(std::initializer_list<CharType> rhs){
		Append(ObserverType(rhs));
	}
	void Append(const String &rhs){
		Append(ObserverType(rhs));
	}
	void Append(String &&rhs){
		const ObserverType obsToAppend(rhs);
		const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
		if((GetCapacity() >= uSizeTotal) || (rhs.GetCapacity() < uSizeTotal)){
			Append(obsToAppend);
		} else {
			rhs.Unshift(obsToAppend);
			Swap(rhs);
		}
	}
	template<StringType OTHER_TYPE_T>
	void Append(const StringObserver<OTHER_TYPE_T> &rhs){
		UnifiedString ucsTempStorage;
		Deunify(*this, GetSize(), String<OTHER_TYPE_T>::Unify(ucsTempStorage, rhs));
	}
	template<StringType OTHER_TYPE_T>
	void Append(const String<OTHER_TYPE_T> &rhs){
		Append(rhs.GetObserver());
	}
	void Truncate(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT_MSG(uOldSize >= uCount, L"删除的字符数太多。");
		xSetSize(uOldSize - uCount);
	}

	void Push(CharType ch){
		Append(ch, 1);
	}
	void Pop() noexcept {
		Truncate(1);
	}

	void UncheckedPush(CharType ch) noexcept {
		ASSERT_MSG(GetLength() < GetCapacity(), L"容器已满。");

		if(x_vStorage.vSmall.chNull == CharType()){
			x_vStorage.vSmall.achData[x_vStorage.vSmall.uchLength] = ch;
			++x_vStorage.vSmall.uchLength;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = ch;
			++x_vStorage.vLarge.uLength;
		}
	}
	void UncheckedPop() noexcept {
		ASSERT_MSG(GetLength() != 0, L"容器已空。");

		if(x_vStorage.vSmall.chNull == CharType()){
			--x_vStorage.vSmall.uchLength;
		} else {
			--x_vStorage.vLarge.uLength;
		}
	}

	void Unshift(CharType ch, std::size_t uCount = 1){
		FillN(ResizeFront(uCount), uCount, ch);
	}
	void Unshift(const CharType *pszBegin){
		Unshift(ObserverType(pszBegin));
	}
	void Unshift(const CharType *pchBegin, const CharType *pchEnd){
		Unshift(ObserverType(pchBegin, pchEnd));
	}
	void Unshift(const CharType *pchBegin, std::size_t uCount){
		Unshift(ObserverType(pchBegin, uCount));
	}
	void Unshift(const ObserverType &obs){
		Replace(0, 0, obs);
	}
	void Unshift(std::initializer_list<CharType> rhs){
		Unshift(ObserverType(rhs));
	}
	void Unshift(const String &rhs){
		Unshift(ObserverType(rhs));
	}
	void Unshift(String &&rhs){
		const ObserverType obsToAppend(rhs);
		const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
		if((GetCapacity() >= uSizeTotal) || (rhs.GetCapacity() < uSizeTotal)){
			Unshift(obsToAppend);
		} else {
			rhs.Append(obsToAppend);
			Swap(rhs);
		}
	}
	template<StringType OTHER_TYPE_T>
	void Unshift(const StringObserver<OTHER_TYPE_T> &rhs){
		UnifiedString ucsTempStorage;
		Deunify(*this, 0, String<OTHER_TYPE_T>::Unify(ucsTempStorage, rhs));
	}
	template<StringType OTHER_TYPE_T>
	void Unshift(const String<OTHER_TYPE_T> &rhs){
		Unshift(rhs.GetObserver());
	}
	void Shift(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT_MSG(uOldSize >= uCount, L"删除的字符数太多。");
		const auto pchWrite = GetBegin();
		CopyN(pchWrite, pchWrite + uCount, uOldSize - uCount);
		xSetSize(uOldSize - uCount);
	}

	ObserverType Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
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

	std::size_t Find(const ObserverType &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().Find(obsToFind, nBegin);
	}
	std::size_t FindBackward(const ObserverType &obsToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().FindBackward(obsToFind, nEnd);
	}
	std::size_t FindRep(CharType chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().FindRep(chToFind, uRepCount, nBegin);
	}
	std::size_t FindRepBackward(CharType chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().FindRepBackward(chToFind, uRepCount, nEnd);
	}
	std::size_t Find(CharType chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetObserver().Find(chToFind, nBegin);
	}
	std::size_t FindBackward(CharType chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetObserver().FindBackward(chToFind, nEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, CharType chRep, std::size_t uCount = 1){
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uCount);
		FillN(pchWrite, uCount, chRep);
		xSetSize(uRemovedBegin + uCount + (uOldLength - uRemovedEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const CharType *pchRepBegin){
		Replace(nBegin, nEnd, ObserverType(pchRepBegin));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const CharType *pchRepBegin, const CharType *pchRepEnd){
		Replace(nBegin, nEnd, ObserverType(pchRepBegin, pchRepEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const CharType *pchRepBegin, std::size_t uLen){
		Replace(nBegin, nEnd, ObserverType(pchRepBegin, uLen));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const ObserverType &obsRep){
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		if(obsCurrent.DoesOverlapWith(obsRep)){
			String strTemp;
			strTemp.Resize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
			auto pchWrite = strTemp.GetStr();
			pchWrite = Copy(pchWrite, obsCurrent.GetBegin(), obsCurrent.GetBegin() + uRemovedBegin);
			pchWrite = Copy(pchWrite, obsRep.GetBegin(), obsRep.GetEnd());
			pchWrite = Copy(pchWrite, obsCurrent.GetBegin() + uRemovedEnd, obsCurrent.GetEnd());
			Swap(strTemp);
		} else {
			const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + obsRep.GetSize());
			CopyN(pchWrite, obsRep.GetBegin(), obsRep.GetSize());
			xSetSize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
		}
	}
	template<StringType OTHER_TYPE_T>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const StringObserver<OTHER_TYPE_T> &obsRep){
		// 基本异常安全保证。
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin);
		xSetSize(uRemovedBegin + (uOldLength - uRemovedEnd));
		UnifiedString ucsTempStorage;
		Deunify(*this, uRemovedBegin, String<OTHER_TYPE_T>::Unify(ucsTempStorage, obsRep));
	}
	template<StringType OTHER_TYPE_T>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const String<OTHER_TYPE_T> &strRep){
		Replace(nBegin, nEnd, strRep.GetObserver());
	}

public:
	operator ObserverType() const noexcept {
		return GetObserver();
	}

	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const CharType *() const noexcept {
		return GetStr();
	}
	explicit operator CharType *() noexcept {
		return GetStr();
	}
	const CharType &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");
		return GetBegin()[uIndex];
	}
	CharType &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");
		return GetBegin()[uIndex];
	}

public:
	using value_type = CharType;

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

template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> &operator+=(String<TYPE_T> &lhs, const String<OTHER_TYPE_T> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> &operator+=(String<TYPE_T> &lhs, const StringObserver<OTHER_TYPE_T> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType TYPE_T>
String<TYPE_T> &operator+=(String<TYPE_T> &lhs, typename String<TYPE_T>::CharType rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, const String<OTHER_TYPE_T> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, const StringObserver<OTHER_TYPE_T> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType TYPE_T>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, typename String<TYPE_T>::CharType rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType TYPE_T>
String<TYPE_T> &&operator+=(String<TYPE_T> &&lhs, String<TYPE_T> &&rhs){
	lhs.Append(std::move(rhs));
	return std::move(lhs);
}

template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> operator+(const String<TYPE_T> &lhs, const String<OTHER_TYPE_T> &rhs){
	String<TYPE_T> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> operator+(const String<TYPE_T> &lhs, const StringObserver<OTHER_TYPE_T> &rhs){
	String<TYPE_T> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType TYPE_T>
String<TYPE_T> operator+(const String<TYPE_T> &lhs, typename String<TYPE_T>::CharType rhs){
	String<TYPE_T> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, const String<OTHER_TYPE_T> &rhs){
	return std::move(lhs += rhs);
}
template<StringType TYPE_T, StringType OTHER_TYPE_T>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, const StringObserver<OTHER_TYPE_T> &rhs){
	return std::move(lhs += rhs);
}
template<StringType TYPE_T>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, typename String<TYPE_T>::CharType rhs){
	return std::move(lhs += rhs);
}
template<StringType TYPE_T>
String<TYPE_T> &&operator+(String<TYPE_T> &&lhs, String<TYPE_T> &&rhs){
	return std::move(lhs += std::move(rhs));
}

template<StringType TYPE_T>
bool operator==(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() == rhs.GetObserver();
}
template<StringType TYPE_T>
bool operator==(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() == rhs;
}
template<StringType TYPE_T>
bool operator==(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs == rhs.GetObserver();
}

template<StringType TYPE_T>
bool operator!=(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() != rhs.GetObserver();
}
template<StringType TYPE_T>
bool operator!=(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() != rhs;
}
template<StringType TYPE_T>
bool operator!=(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs != rhs.GetObserver();
}

template<StringType TYPE_T>
bool operator<(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() < rhs.GetObserver();
}
template<StringType TYPE_T>
bool operator<(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() < rhs;
}
template<StringType TYPE_T>
bool operator<(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs < rhs.GetObserver();
}

template<StringType TYPE_T>
bool operator>(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() > rhs.GetObserver();
}
template<StringType   TYPE_T>
bool operator>(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() > rhs;
}
template<StringType TYPE_T>
bool operator>(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs > rhs.GetObserver();
}

template<StringType TYPE_T>
bool operator<=(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() <= rhs.GetObserver();
}
template<StringType TYPE_T>
bool operator<=(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() <= rhs;
}
template<StringType TYPE_T>
bool operator<=(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs <= rhs.GetObserver();
}

template<StringType TYPE_T>
bool operator>=(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() >= rhs.GetObserver();
}
template<StringType TYPE_T>
bool operator>=(const String<TYPE_T> &lhs, const StringObserver<TYPE_T> &rhs) noexcept {
	return lhs.GetObserver() >= rhs;
}
template<StringType TYPE_T>
bool operator>=(const StringObserver<TYPE_T> &lhs, const String<TYPE_T> &rhs) noexcept {
	return lhs >= rhs.GetObserver();
}

template<StringType TYPE_T>
void swap(String<TYPE_T> &lhs, String<TYPE_T> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType TYPE_T>
auto begin(const String<TYPE_T> &lhs) noexcept {
	return lhs.GetBegin();
}
template<StringType TYPE_T>
auto begin(String<TYPE_T> &lhs) noexcept {
	return lhs.GetBegin();
}
template<StringType TYPE_T>
auto cbegin(const String<TYPE_T> &lhs) noexcept {
	return lhs.GetBegin();
}
template<StringType TYPE_T>
auto end(const String<TYPE_T> &lhs) noexcept {
	return lhs.GetEnd();
}
template<StringType TYPE_T>
auto end(String<TYPE_T> &lhs) noexcept {
	return lhs.GetEnd();
}
template<StringType TYPE_T>
auto cend(const String<TYPE_T> &lhs) noexcept {
	return lhs.GetEnd();
}

extern template class String<StringType::NARROW>;
extern template class String<StringType::WIDE>;
extern template class String<StringType::UTF8>;
extern template class String<StringType::UTF16>;
extern template class String<StringType::UTF32>;
extern template class String<StringType::CESU8>;
extern template class String<StringType::ANSI>;

using NarrowString		= String<StringType::NARROW>;
using WideString		= String<StringType::WIDE>;
using Utf8String		= String<StringType::UTF8>;
using Utf16String		= String<StringType::UTF16>;
using Utf32String		= String<StringType::UTF32>;
using Cesu8String		= String<StringType::CESU8>;
using AnsiString		= String<StringType::ANSI>;

// 字面量运算符。
template<typename CharT, CharT ...STRING_T>
[[deprecated("Be warned that encodings of narrow string literals vary from compilers to compilers "
	"and might even depend on encodings of source files on g++.")]]
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

// MultiIndexMap
template<StringType TYPE_T>
struct StringTripleComparator {
	int operator()(const String<TYPE_T> &lhs, const String<TYPE_T> &rhs) const noexcept {
		return lhs.Compare(rhs);
	}
	template<typename ComparandT>
	int operator()(const String<TYPE_T> &lhs, const ComparandT &rhs) const noexcept {
		return lhs.Compare(rhs);
	}
	template<typename ComparandT>
	int operator()(const ComparandT &lhs, const String<TYPE_T> &rhs) const noexcept {
		return -rhs.Compare(lhs);
	}
};

template<StringType TYPE_T>
	StringTripleComparator<TYPE_T> GetDefaultComparator(const String<TYPE_T> &) noexcept;

}

using ::MCF::operator""_ns;
using ::MCF::operator""_ws;
using ::MCF::operator""_u8s;
using ::MCF::operator""_u16s;
using ::MCF::operator""_u32s;

#endif
