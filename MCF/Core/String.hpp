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
#include <new>
#include <cstring>
#include <cstddef>
#include <cstdint>

// 借用了 https://github.com/elliotgoodrich/SSO-23 的一些想法。然而我们可以优化更多……
// 我们支持的最大 SSO 长度是 31 个“字符”，而不是 23 个“字节”。

namespace MCF {

template<StringType kTypeT>
class String;

using UnifiedString = String<StringType::UTF32>;
using UnifiedStringObserver = StringObserver<StringType::UTF32>;

template<StringType kTypeT>
class String {
public:
	using Observer = StringObserver<kTypeT>;

	using Char = typename Observer::Char;

	enum : std::size_t {
		kNpos = Observer::kNpos
	};

public:
	static const String kEmpty;

public:
	static UnifiedStringObserver Unify(UnifiedString &usTempStorage, const Observer &obsSrc);
	static void Deunify(String &strDst, std::size_t uPos, const UnifiedStringObserver &usoSrc);

private:
	union $Storage {
		struct {
			Char achData[31];
			std::make_signed_t<Char> schComplLength;
		} vSmall;

		struct {
			Char *pchBegin;
			std::size_t uLength;
			std::size_t uSizeAllocated;
		} vLarge;
	} $vStorage;

private:
	std::size_t $GetSmallLength() const noexcept {
		return COUNT_OF($vStorage.vSmall.achData) - static_cast<std::make_unsigned_t<Char>>($vStorage.vSmall.schComplLength);
	}
	void $SetSmallLength(std::size_t uLength) noexcept {
		$vStorage.vSmall.schComplLength = static_cast<std::make_signed_t<Char>>(COUNT_OF($vStorage.vSmall.achData) - uLength);
	}

public:
	String() noexcept {
#ifndef NDEBUG
		std::memset($vStorage.vSmall.achData, 0xCC, sizeof($vStorage.vSmall.achData));
#endif
		$SetSmallLength(0);
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
	String(const Char *pchBegin, const Char *pchEnd)
		: String()
	{
		Assign(pchBegin, pchEnd);
	}
	String(const Char *pchBegin, std::size_t uLen)
		: String()
	{
		Assign(pchBegin, uLen);
	}
	explicit String(const Observer &rhs)
		: String()
	{
		Assign(rhs);
	}
	explicit String(std::initializer_list<Char> rhs)
		: String()
	{
		Assign(rhs);
	}
	template<StringType kOtherTypeT>
	explicit String(const StringObserver<kOtherTypeT> &rhs)
		: String()
	{
		Assign(rhs);
	}
	template<StringType kOtherTypeT>
	explicit String(const String<kOtherTypeT> &rhs)
		: String()
	{
		Assign(rhs);
	}
	String(const String &rhs)
		: String()
	{
		Assign(rhs);
	}
	String(String &&rhs) noexcept
		: String()
	{
		Assign(std::move(rhs));
	}
	String &operator=(Char ch){
		Assign(ch, 1);
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
	~String() noexcept {
		if($vStorage.vSmall.schComplLength < 0){
			::operator delete[]($vStorage.vLarge.pchBegin);
		}
#ifndef NDEBUG
		std::memset(&$vStorage, 0xDD, sizeof($vStorage));
#endif
	}

private:
	Char *$ChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd, std::size_t uFirstOffset, std::size_t uThirdOffset){
		const auto pchOldBuffer = GetBegin();
		const auto uOldLength = GetLength();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewLength = uThirdOffset + (uOldLength - uRemovedEnd);
		auto uCharsToAlloc = uNewLength + 1;

		ASSERT(uRemovedBegin <= uOldLength);
		ASSERT(uRemovedEnd <= uOldLength);
		ASSERT(uRemovedBegin <= uRemovedEnd);
		ASSERT(uFirstOffset + uRemovedBegin <= uThirdOffset);

		if(GetCapacity() < uNewLength){
			uCharsToAlloc += (uCharsToAlloc >> 1);
			uCharsToAlloc = (uCharsToAlloc + 0x0F) & (std::size_t)-0x10;
			if(uCharsToAlloc < uNewLength + 1){
				uCharsToAlloc = uNewLength + 1;
			}
			const std::size_t uBytesToAlloc = sizeof(Char) * uCharsToAlloc;
			if(uBytesToAlloc / sizeof(Char) != uCharsToAlloc){
				throw std::bad_array_new_length();
			}

			pchNewBuffer = (Char *)::operator new[](uBytesToAlloc);
		}

		if((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldLength != uRemovedEnd)){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if($vStorage.vSmall.schComplLength >= 0){
				$vStorage.vSmall.schComplLength = -1;
			} else {
				::operator delete[](pchOldBuffer);
			}

			$vStorage.vLarge.pchBegin = pchNewBuffer;
			$vStorage.vLarge.uLength = uOldLength;
			$vStorage.vLarge.uSizeAllocated = uCharsToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void $SetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if($vStorage.vSmall.schComplLength >= 0){
			$SetSmallLength(uNewSize);
		} else {
			$vStorage.vLarge.uLength = uNewSize;
		}
	}

public:
	const Char *GetBegin() const noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return $vStorage.vSmall.achData;
		} else {
			return $vStorage.vLarge.pchBegin;
		}
	}
	Char *GetBegin() noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return $vStorage.vSmall.achData;
		} else {
			return $vStorage.vLarge.pchBegin;
		}
	}

	const Char *GetEnd() const noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return $vStorage.vSmall.achData + $GetSmallLength();
		} else {
			return $vStorage.vLarge.pchBegin + $vStorage.vLarge.uLength;
		}
	}
	Char *GetEnd() noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return $vStorage.vSmall.achData + $GetSmallLength();
		} else {
			return $vStorage.vLarge.pchBegin + $vStorage.vLarge.uLength;
		}
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return $GetSmallLength();
		} else {
			return $vStorage.vLarge.uLength;
		}
	}

	const Char *GetStr() const noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			const_cast<Char &>($vStorage.vSmall.achData[$GetSmallLength()]) = Char();
			return $vStorage.vSmall.achData;
		} else {
			$vStorage.vLarge.pchBegin[$vStorage.vLarge.uLength] = Char();
			return $vStorage.vLarge.pchBegin;
		}
	}
	Char *GetStr() noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			$vStorage.vSmall.achData[$GetSmallLength()] = Char();
			return $vStorage.vSmall.achData;
		} else {
			$vStorage.vLarge.pchBegin[$vStorage.vLarge.uLength] = Char();
			return $vStorage.vLarge.pchBegin;
		}
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	Observer GetObserver() const noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return Observer($vStorage.vSmall.achData, $GetSmallLength());
		} else {
			return Observer($vStorage.vLarge.pchBegin, $vStorage.vLarge.uLength);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if($vStorage.vSmall.schComplLength >= 0){
			return COUNT_OF($vStorage.vSmall.achData);
		} else {
			return $vStorage.vLarge.uSizeAllocated - 1;
		}
	}
	void Reserve(std::size_t uNewCapacity){
		if(uNewCapacity > GetCapacity()){
			const auto uOldSize = GetSize();
			$ChopAndSplice(uOldSize, uOldSize, 0, uNewCapacity);
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = GetSize();
		const auto uNewCapacity = uOldSize + uDeltaCapacity;
		if(uNewCapacity < uOldSize){
			throw std::bad_array_new_length();
		}
		Reserve(uNewCapacity);
	}

	void Resize(std::size_t uNewSize){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			$SetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Pop(uOldSize - uNewSize);
		}
	}
	Char *ResizeMoreFront(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		const auto uNewSize = uOldSize + uDeltaSize;
		if(uNewSize < uOldSize){
			throw std::bad_array_new_length();
		}
		$ChopAndSplice(uOldSize, uOldSize, uDeltaSize, uNewSize);
		$SetSize(uNewSize);
		return GetData() + uOldSize;
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		const auto uNewSize = uOldSize + uDeltaSize;
		if(uNewSize < uOldSize){
			throw std::bad_array_new_length();
		}
		$ChopAndSplice(uOldSize, uOldSize, 0, uNewSize);
		$SetSize(uNewSize);
		return GetData() + uOldSize;
	}
	void Shrink() noexcept {
		const auto uSzLen = Observer(GetStr()).GetLength();
		ASSERT(uSzLen <= GetSize());
		$SetSize(uSzLen);
	}

	bool IsEmpty() const noexcept {
		return GetBegin() == GetEnd();
	}
	void Clear() noexcept {
		$SetSize(0);
	}

	void Swap(String &rhs) noexcept {
		std::swap($vStorage, rhs.$vStorage);
	}

	int Compare(const Observer &rhs) const noexcept {
		return GetObserver().Compare(rhs);
	}
	int Compare(const String &rhs) const noexcept {
		return GetObserver().Compare(rhs.GetObserver());
	}

	void Assign(Char ch, std::size_t uCount = 1){
		Resize(uCount);
		FillN(GetStr(), uCount, ch);
	}
	void Assign(const Char *pszBegin){
		Assign(Observer(pszBegin));
	}
	void Assign(const Char *pchBegin, const Char *pchEnd){
		Assign(Observer(pchBegin, pchEnd));
	}
	void Assign(const Char *pchBegin, std::size_t uCount){
		Assign(Observer(pchBegin, uCount));
	}
	void Assign(const Observer &rhs){
		Resize(rhs.GetSize());
		Copy(GetStr(), rhs.GetBegin(), rhs.GetEnd());
	}
	void Assign(std::initializer_list<Char> rhs){
		Assign(Observer(rhs));
	}
	template<StringType kOtherTypeT>
	void Assign(const StringObserver<kOtherTypeT> &rhs){
		String strTemp;
		strTemp.Append(rhs);
		Assign(std::move(strTemp));
	}
	template<StringType kOtherTypeT>
	void Assign(const String<kOtherTypeT> &rhs){
		Assign(StringObserver<kOtherTypeT>(rhs));
	}
	void Assign(const String &rhs){
		if(&rhs != this){
			Assign(Observer(rhs));
		}
	}
	void Assign(String &&rhs) noexcept {
		ASSERT(this != &rhs);

		if($vStorage.vSmall.schComplLength < 0){
			::operator delete[]($vStorage.vLarge.pchBegin);
		}
		$vStorage = rhs.$vStorage;
#ifndef NDEBUG
		std::memset(rhs.$vStorage.vSmall.achData, 0xCD, sizeof(rhs.$vStorage.vSmall.achData));
#endif
		rhs.$vStorage.vSmall.schComplLength = 0;
	}

	void Append(Char ch, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, ch);
	}
	void Append(const Char *pszBegin){
		Append(Observer(pszBegin));
	}
	void Append(const Char *pchBegin, const Char *pchEnd){
		Append(Observer(pchBegin, pchEnd));
	}
	void Append(const Char *pchBegin, std::size_t uCount){
		Append(Observer(pchBegin, uCount));
	}
	void Append(const Observer &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd());
	}
	void Append(std::initializer_list<Char> rhs){
		Append(Observer(rhs));
	}
	void Append(const String &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd()); // 这是正确的即使对于 &rhs == this 的情况。
	}
	template<StringType kOtherTypeT>
	void Append(const StringObserver<kOtherTypeT> &rhs){
		UnifiedString ucsTempStorage;
		Deunify(*this, GetSize(), String<kOtherTypeT>::Unify(ucsTempStorage, rhs));
	}
	template<StringType kOtherTypeT>
	void Append(const String<kOtherTypeT> &rhs){
		Append(rhs.GetObserver());
	}
	void Push(Char ch){
		Append(ch, 1);
	}
	void UncheckedPush(Char ch) noexcept {
		ASSERT(GetLength() < GetCapacity());

		if($vStorage.vSmall.schComplLength >= 0){
			$vStorage.vSmall.achData[$GetSmallLength()] = ch;
			--$vStorage.vSmall.schComplLength;
		} else {
			$vStorage.vLarge.pchBegin[$vStorage.vLarge.uLength] = ch;
			++$vStorage.vLarge.uLength;
		}
	}
	void Pop(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT(uOldSize >= uCount);
		$SetSize(uOldSize - uCount);
	}

	void Unshift(Char ch, std::size_t uCount = 1){
		FillN(ResizeMoreFront(uCount), uCount, ch);
	}
	void Unshift(const Char *pszBegin){
		Unshift(Observer(pszBegin));
	}
	void Unshift(const Char *pchBegin, const Char *pchEnd){
		Unshift(Observer(pchBegin, pchEnd));
	}
	void Unshift(const Char *pchBegin, std::size_t uCount){
		Unshift(Observer(pchBegin, uCount));
	}
	void Unshift(const Observer &rhs){
		const auto pWrite = ResizeMoreFront(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd());
	}
	void Unshift(std::initializer_list<Char> rhs){
		Unshift(Observer(rhs));
	}
	void Unshift(const String &rhs){
		const auto pWrite = ResizeMoreFront(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd()); // 这是正确的即使对于 &rhs == this 的情况。
	}
	template<StringType kOtherTypeT>
	void Unshift(const StringObserver<kOtherTypeT> &rhs){
		UnifiedString ucsTempStorage;
		Deunify(*this, 0, String<kOtherTypeT>::Unify(ucsTempStorage, rhs));
	}
	template<StringType kOtherTypeT>
	void Unshift(const String<kOtherTypeT> &rhs){
		Unshift(rhs.GetObserver());
	}
	void Shift(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT(uOldSize >= uCount);
		const auto pchWrite = GetBegin();
		CopyN(pchWrite, pchWrite + uCount, uOldSize - uCount);
		$SetSize(uOldSize - uCount);
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

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chRep, std::size_t uRepSize = 1){
		const auto obsCurrent = GetObserver();
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved = obsCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldLength - (uRemovedEnd - uRemovedBegin);
		const auto uNewLength = uLengthAfterRemoved + uRepSize;
		if(uNewLength < uLengthAfterRemoved){
			throw std::bad_array_new_length();
		}

		const auto pchWrite = $ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uRepSize);
		FillN(pchWrite, uRepSize, chRep);
		$SetSize(uNewLength);
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin){
		Replace(nBegin, nEnd, Observer(pchRepBegin));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, const Char *pchRepEnd){
		Replace(nBegin, nEnd, Observer(pchRepBegin, pchRepEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, std::size_t uLen){
		Replace(nBegin, nEnd, Observer(pchRepBegin, uLen));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Observer &obsRep){
		const auto uRepSize = obsRep.GetLength();

		const auto obsCurrent = GetObserver();
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved = obsCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldLength - (uRemovedEnd - uRemovedBegin);
		const auto uNewLength = uLengthAfterRemoved + uRepSize;
		if(uNewLength < uLengthAfterRemoved){
			throw std::bad_array_new_length();
		}

		if(obsCurrent.DoesOverlapWith(obsRep)){
			String strTemp;
			strTemp.Resize(uNewLength);
			auto pchWrite = strTemp.GetStr();
			pchWrite = Copy(pchWrite, obsCurrent.GetBegin(), obsCurrent.GetBegin() + uRemovedBegin);
			pchWrite = Copy(pchWrite, obsRep.GetBegin(), obsRep.GetEnd());
			pchWrite = Copy(pchWrite, obsCurrent.GetBegin() + uRemovedEnd, obsCurrent.GetEnd());
			Assign(std::move(strTemp));
		} else {
			const auto pchWrite = $ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + obsRep.GetSize());
			CopyN(pchWrite, obsRep.GetBegin(), obsRep.GetSize());
			$SetSize(uNewLength);
		}
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
		ASSERT(uIndex <= GetLength());

		return GetBegin()[uIndex];
	}
	Char &operator[](std::size_t uIndex) noexcept {
		ASSERT(uIndex <= GetLength());

		return GetBegin()[uIndex];
	}
};

template<StringType kTypeT>
const String<kTypeT> String<kTypeT>::kEmpty;

template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, const String<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, const StringObserver<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, typename String<kTypeT>::Char rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, const typename String<kTypeT>::Char *rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const String<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const StringObserver<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, typename String<kTypeT>::Char rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const typename String<kTypeT>::Char *rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, String<kTypeT> &&rhs){
	lhs.Append(std::move(rhs));
	return std::move(lhs);
}

template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, const String<kOtherTypeT> &rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, const StringObserver<kOtherTypeT> &rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, typename String<kTypeT>::Char rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, const typename String<kTypeT>::Char *rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, const String<kOtherTypeT> &rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, const StringObserver<kOtherTypeT> &rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, typename String<kTypeT>::Char rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, const typename String<kTypeT>::Char *rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, String<kTypeT> &&rhs){
	return std::move(lhs += std::move(rhs));
}

template<StringType kTypeT>
bool operator==(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() == rhs.GetObserver();
}
template<StringType kTypeT>
bool operator==(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() == rhs;
}
template<StringType kTypeT>
bool operator==(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs == rhs.GetObserver();
}

template<StringType kTypeT>
bool operator!=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() != rhs.GetObserver();
}
template<StringType kTypeT>
bool operator!=(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() != rhs;
}
template<StringType kTypeT>
bool operator!=(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs != rhs.GetObserver();
}

template<StringType kTypeT>
bool operator<(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() < rhs.GetObserver();
}
template<StringType kTypeT>
bool operator<(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() < rhs;
}
template<StringType kTypeT>
bool operator<(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs < rhs.GetObserver();
}

template<StringType kTypeT>
bool operator>(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() > rhs.GetObserver();
}
template<StringType   kTypeT>
bool operator>(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() > rhs;
}
template<StringType kTypeT>
bool operator>(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs > rhs.GetObserver();
}

template<StringType kTypeT>
bool operator<=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() <= rhs.GetObserver();
}
template<StringType kTypeT>
bool operator<=(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() <= rhs;
}
template<StringType kTypeT>
bool operator<=(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs <= rhs.GetObserver();
}

template<StringType kTypeT>
bool operator>=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() >= rhs.GetObserver();
}
template<StringType kTypeT>
bool operator>=(const String<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.GetObserver() >= rhs;
}
template<StringType kTypeT>
bool operator>=(const StringObserver<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs >= rhs.GetObserver();
}

template<StringType kTypeT>
void swap(String<kTypeT> &lhs, String<kTypeT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType kTypeT>
decltype(auto) begin(const String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
decltype(auto) begin(String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
decltype(auto) end(const String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
decltype(auto) end(String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}

extern template class String<StringType::NARROW>;
extern template class String<StringType::WIDE>;
extern template class String<StringType::UTF8>;
extern template class String<StringType::UTF16>;
extern template class String<StringType::UTF32>;
extern template class String<StringType::CESU8>;
extern template class String<StringType::ANSI>;

using NarrowString = String<StringType::NARROW>;
using WideString   = String<StringType::WIDE>;
using Utf8String   = String<StringType::UTF8>;
using Utf16String  = String<StringType::UTF16>;
using Utf32String  = String<StringType::UTF32>;
using Cesu8String  = String<StringType::CESU8>;
using AnsiString   = String<StringType::ANSI>;

// 字面量运算符。
template<typename CharT, CharT ...kCharsT>
[[deprecated("Be warned that encodings of narrow string literals vary from compilers to compilers and might even depend on encodings of source files on g++.")]]
extern inline const auto &operator""_ns(){
	static const NarrowString s_nsRet{ kCharsT... };
	return s_nsRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_ws(){
	static const WideString s_wsRet{ kCharsT... };
	return s_wsRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u8s(){
	static const Utf8String s_u8sRet{ kCharsT... };
	return s_u8sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u16s(){
	static const Utf16String s_u16sRet{ kCharsT... };
	return s_u16sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u32s(){
	static const Utf32String s_u32sRet{ kCharsT... };
	return s_u32sRet;
}

}

using ::MCF::operator""_ns;
using ::MCF::operator""_ws;
using ::MCF::operator""_u8s;
using ::MCF::operator""_u16s;
using ::MCF::operator""_u32s;

#endif
