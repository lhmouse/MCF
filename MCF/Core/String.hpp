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
	static UnifiedStringObserver Unify(UnifiedString &usTempStorage, const Observer &obsSrc);
	static void Deunify(String &strDst, std::size_t uPos, const UnifiedStringObserver &usoSrc);

private:
	union xStorage {
		struct {
			Char achData[31];
			std::make_signed_t<Char> schComplLength;
		} vSmall;

		struct {
			Char *pchBegin;
			std::size_t uLength;
			std::size_t uSizeAllocated;
		} vLarge;
	} x_vStorage;

private:
	std::size_t xGetSmallLength() const noexcept {
		return COUNT_OF(x_vStorage.vSmall.achData) - static_cast<std::make_unsigned_t<Char>>(x_vStorage.vSmall.schComplLength);
	}
	void xSetSmallLength(std::size_t uLength) noexcept {
		x_vStorage.vSmall.schComplLength = static_cast<std::make_signed_t<Char>>(COUNT_OF(x_vStorage.vSmall.achData) - uLength);
	}

public:
	String() noexcept {
#ifndef NDEBUG
		std::memset(x_vStorage.vSmall.achData, 0xCC, sizeof(x_vStorage.vSmall.achData));
#endif
		xSetSmallLength(0);
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
		if(x_vStorage.vSmall.schComplLength < 0){
			delete[] x_vStorage.vLarge.pchBegin;
		}
#ifndef NDEBUG
		std::memset(&x_vStorage, 0xDD, sizeof(x_vStorage));
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
		auto uSizeToAlloc = uNewLength + 1;

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
			pchNewBuffer = new Char[uSizeToAlloc];
		}

		if((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldLength != uRemovedEnd)){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(x_vStorage.vSmall.schComplLength >= 0){
				x_vStorage.vSmall.schComplLength = -1;
			} else {
				delete[] pchOldBuffer;
			}

			x_vStorage.vLarge.pchBegin = pchNewBuffer;
			x_vStorage.vLarge.uLength = uOldLength;
			x_vStorage.vLarge.uSizeAllocated = uSizeToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void xSetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(x_vStorage.vSmall.schComplLength >= 0){
			xSetSmallLength(uNewSize);
		} else {
			x_vStorage.vLarge.uLength = uNewSize;
		}
	}

public:
	const Char *GetBegin() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData;
		} else {
			return x_vStorage.vLarge.pchBegin;
		}
	}
	Char *GetBegin() noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData;
		} else {
			return x_vStorage.vLarge.pchBegin;
		}
	}

	const Char *GetEnd() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData + xGetSmallLength();
		} else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}
	Char *GetEnd() noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData + xGetSmallLength();
		} else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	std::size_t GetSize() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return xGetSmallLength();
		} else {
			return x_vStorage.vLarge.uLength;
		}
	}

	const Char *GetStr() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			const_cast<Char &>(x_vStorage.vSmall.achData[xGetSmallLength()]) = Char();
			return x_vStorage.vSmall.achData;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = Char();
			return x_vStorage.vLarge.pchBegin;
		}
	}
	Char *GetStr() noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			x_vStorage.vSmall.achData[xGetSmallLength()] = Char();
			return x_vStorage.vSmall.achData;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = Char();
			return x_vStorage.vLarge.pchBegin;
		}
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	Observer GetObserver() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return Observer(x_vStorage.vSmall.achData, xGetSmallLength());
		} else {
			return Observer(x_vStorage.vLarge.pchBegin, x_vStorage.vLarge.uLength);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return COUNT_OF(x_vStorage.vSmall.achData);
		} else {
			return x_vStorage.vLarge.uSizeAllocated - 1;
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
	Char *ResizeMoreFront(std::size_t uDeltaSize){
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
		const auto uSzLen = Observer(GetStr()).GetLength();
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
		std::swap(x_vStorage, rhs.x_vStorage);
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

		if(x_vStorage.vSmall.schComplLength < 0){
			delete[] x_vStorage.vLarge.pchBegin;
		}
		x_vStorage = rhs.x_vStorage;
#ifndef NDEBUG
		std::memset(rhs.x_vStorage.vSmall.achData, 0xCD, sizeof(rhs.x_vStorage.vSmall.achData));
#endif
		rhs.x_vStorage.vSmall.schComplLength = 0;
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
		Replace(-1, -1, rhs);
	}
	void Append(std::initializer_list<Char> rhs){
		Append(Observer(rhs));
	}
	void Append(const String &rhs){
		Append(Observer(rhs));
	}
	void Append(String &&rhs){
		const Observer obsToAppend(rhs);
		const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
		if((GetCapacity() >= uSizeTotal) || (rhs.GetCapacity() < uSizeTotal)){
			Append(obsToAppend);
		} else {
			rhs.Unshift(obsToAppend);
			Assign(std::move(rhs));
		}
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
	void Truncate(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT_MSG(uOldSize >= uCount, L"删除的字符数太多。");
		xSetSize(uOldSize - uCount);
	}

	void Push(Char ch){
		Append(ch, 1);
	}
	void Pop() noexcept {
		Truncate(1);
	}

	void UncheckedPush(Char ch) noexcept {
		ASSERT_MSG(GetLength() < GetCapacity(), L"容器已满。");

		if(x_vStorage.vSmall.schComplLength >= 0){
			x_vStorage.vSmall.achData[xGetSmallLength()] = ch;
			--x_vStorage.vSmall.schComplLength;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = ch;
			++x_vStorage.vLarge.uLength;
		}
	}
	void UncheckedPop() noexcept {
		ASSERT_MSG(GetLength() != 0, L"容器已空。");

		if(x_vStorage.vSmall.schComplLength >= 0){
			++x_vStorage.vSmall.schComplLength;
		} else {
			--x_vStorage.vLarge.uLength;
		}
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
	void Unshift(const Observer &obs){
		Replace(0, 0, obs);
	}
	void Unshift(std::initializer_list<Char> rhs){
		Unshift(Observer(rhs));
	}
	void Unshift(const String &rhs){
		Unshift(Observer(rhs));
	}
	void Unshift(String &&rhs){
		const Observer obsToAppend(rhs);
		const auto uSizeTotal = GetSize() + obsToAppend.GetSize();
		if((GetCapacity() >= uSizeTotal) || (rhs.GetCapacity() < uSizeTotal)){
			Unshift(obsToAppend);
		} else {
			rhs.Append(obsToAppend);
			Assign(std::move(rhs));
		}
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
		ASSERT_MSG(uOldSize >= uCount, L"删除的字符数太多。");
		const auto pchWrite = GetBegin();
		CopyN(pchWrite, pchWrite + uCount, uOldSize - uCount);
		xSetSize(uOldSize - uCount);
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
			Assign(std::move(strTemp));
		} else {
			const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + obsRep.GetSize());
			CopyN(pchWrite, obsRep.GetBegin(), obsRep.GetSize());
			xSetSize(uRemovedBegin + obsRep.GetSize() + (uOldLength - uRemovedEnd));
		}
	}
	template<StringType kOtherTypeT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const StringObserver<kOtherTypeT> &obsRep){
		// 基本异常安全保证。
		const auto obsCurrent(GetObserver());
		const auto uOldLength = obsCurrent.GetLength();

		const auto obsRemoved(obsCurrent.Slice(nBegin, nEnd));
		const auto uRemovedBegin = (std::size_t)(obsRemoved.GetBegin() - obsCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(obsRemoved.GetEnd() - obsCurrent.GetBegin());

		const auto pchWrite = xChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin);
		xSetSize(uRemovedBegin + (uOldLength - uRemovedEnd));
		UnifiedString ucsTempStorage;
		Deunify(*this, uRemovedBegin, String<kOtherTypeT>::Unify(ucsTempStorage, obsRep));
	}
	template<StringType kOtherTypeT>
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const String<kOtherTypeT> &strRep){
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
auto begin(const String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
auto begin(String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
auto cbegin(const String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
auto end(const String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
auto end(String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
auto cend(const String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
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

// MultiIndexMap
template<StringType kTypeT>
struct StringTripleComparator {
	int operator()(const String<kTypeT> &lhs, const String<kTypeT> &rhs) const noexcept {
		return lhs.Compare(rhs);
	}
	template<typename ComparandT>
	int operator()(const String<kTypeT> &lhs, const ComparandT &rhs) const noexcept {
		return lhs.Compare(rhs);
	}
	template<typename ComparandT>
	int operator()(const ComparandT &lhs, const String<kTypeT> &rhs) const noexcept {
		return -rhs.Compare(lhs);
	}
};

template<StringType kTypeT>
StringTripleComparator<kTypeT> GetDefaultComparator(const String<kTypeT> &) noexcept;

}

using ::MCF::operator""_ns;
using ::MCF::operator""_ws;
using ::MCF::operator""_u8s;
using ::MCF::operator""_u16s;
using ::MCF::operator""_u32s;

#endif
