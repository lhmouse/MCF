// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VAR_CHAR_HPP_
#define MCF_CORE_VAR_CHAR_HPP_

#include "../Containers/_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include "Exception.hpp"
#include "Array.hpp"
#include "ArrayView.hpp"
#include "StringView.hpp"
#include <type_traits>
#include <initializer_list>
#include <cstring>
#include <cstddef>

namespace MCF {

template<unsigned kCapacityT>
class VarChar {
	static_assert(kCapacityT <= static_cast<unsigned char>(-1), "kCapacityT is too large for VarChar.");

public:
	using View = NarrowStringView;
	using Char = char;

	// 容器需求。
	using Element         = char;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <VarChar>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <VarChar>;

public:
	static const VarChar kEmpty;

private:
	Array<Char, kCapacityT + 1> x_achData;

public:
	VarChar(std::nullptr_t = nullptr) noexcept {
		reinterpret_cast<std::make_unsigned_t<Char> &>(x_achData.GetEnd()[-1]) = kCapacityT;
	}
	VarChar(const Char *pszStr, std::size_t uLen)
		: VarChar()
	{
		Assign(pszStr, uLen);
	}
	explicit VarChar(const Char *rhs)
		: VarChar()
	{
		Assign(rhs);
	}
	explicit VarChar(const View &rhs)
		: VarChar()
	{
		Assign(rhs);
	}
	VarChar(std::initializer_list<Char> rhs)
		: VarChar()
	{
		Assign(rhs);
	}
	VarChar &operator=(const Char *rhs){
		Assign(rhs);
		return *this;
	}
	VarChar &operator=(const View &rhs){
		Assign(rhs);
		return *this;
	}
	VarChar &operator=(std::initializer_list<Char> rhs){
		Assign(rhs);
		return *this;
	}

private:
	Char *X_ChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd, std::size_t uFirstOffset, std::size_t uThirdOffset){
		const auto pchBuffer = GetBegin();
		const auto uOldSize = GetSize();
		const auto uNewSize = uThirdOffset + (uOldSize - uRemovedEnd);

		ASSERT(uRemovedBegin <= uOldSize);
		ASSERT(uRemovedEnd <= uOldSize);
		ASSERT(uRemovedBegin <= uRemovedEnd);
		ASSERT(uFirstOffset + uRemovedBegin <= uThirdOffset);

		if(uNewSize > kCapacityT){
			DEBUG_THROW(Exception, ERROR_OUTOFMEMORY, "VarChar: Max capacity exceeded"_rcs);
		}

		if((pchBuffer + uFirstOffset != pchBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchBuffer + uFirstOffset, pchBuffer, uRemovedBegin * sizeof(Char));
		}
		if((pchBuffer + uThirdOffset != pchBuffer + uRemovedEnd) && (uOldSize != uRemovedEnd)){
			std::memmove(pchBuffer + uThirdOffset, pchBuffer + uRemovedEnd, (uOldSize - uRemovedEnd) * sizeof(Char));
		}

		return pchBuffer + uFirstOffset + uRemovedBegin;
	}
	void X_SetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		reinterpret_cast<std::make_unsigned_t<Char> &>(x_achData.GetEnd()[-1]) = GetCapacity() - uNewSize;
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		X_SetSize(0);
	}

	const Element *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}
	ConstEnumerator EnumerateConstFirst() const noexcept {
		return EnumerateFirst();
	}
	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}
	ConstEnumerator EnumerateConstLast() const noexcept {
		return EnumerateLast();
	}
	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}
	constexpr ConstEnumerator EnumerateConstSingular() const noexcept {
		return EnumerateSingular();
	}

	void Swap(VarChar &rhs) noexcept {
		using std::swap;
		swap(x_achData, rhs.x_achData);
	}

	// VarChar 需求。
	const Char *GetBegin() const noexcept {
		return x_achData.GetBegin();
	}
	Char *GetBegin() noexcept {
		return x_achData.GetBegin();
	}

	const Char *GetEnd() const noexcept {
		return x_achData.GetBegin() + GetSize();
	}
	Char *GetEnd() noexcept {
		return x_achData.GetBegin() + GetSize();
	}
	std::size_t GetSize() const noexcept {
		return kCapacityT - reinterpret_cast<const std::make_unsigned_t<Char> &>(x_achData.GetEnd()[-1]);
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	const Char *GetConstData() const noexcept {
		return GetData();
	}
	const Char *GetStr() const noexcept {
		const_cast<Char &>(GetEnd()[0]) = Char();
		return GetBegin();
	}
	Char *GetStr() noexcept {
		GetEnd()[0] = Char();
		return GetBegin();
	}
	const Char *GetConstStr() const noexcept {
		return GetStr();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const Char &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "VarChar: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Char &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "VarChar: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	const Char &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		return GetBegin()[uIndex];
	}
	Char &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		return GetBegin()[uIndex];
	}

	View GetView() const noexcept {
		return View(GetData(), GetSize());
	}

	static constexpr std::size_t GetCapacity() noexcept {
		return kCapacityT;
	}

	void Resize(std::size_t uNewSize){
		X_SetSize(uNewSize);
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		const auto uNewSize = uOldSize + uDeltaSize;
		if(uNewSize < uOldSize){
			throw std::bad_array_new_length();
		}
		X_SetSize(uNewSize);
		return GetData() + uOldSize;
	}
	std::pair<Char *, std::size_t> ResizeToCapacity() noexcept {
		const auto uOldSize = GetSize();
		const auto uNewSize = GetCapacity();
		const auto uDeltaSize = uNewSize - uOldSize;
		X_SetSize(uNewSize);
		return std::make_pair(GetData() + uOldSize, uDeltaSize);
	}
	void ShrinkAsZeroTerminated() noexcept {
		const auto uSzLen = View(GetStr()).GetSize();
		ASSERT(uSzLen <= GetSize());
		X_SetSize(uSzLen);
	}

	int Compare(const View &rhs) const noexcept {
		return GetView().Compare(rhs);
	}
	int Compare(const VarChar &rhs) const noexcept {
		return GetView().Compare(rhs.GetView());
	}

	void Assign(Char ch, std::size_t uCount = 1){
		Resize(uCount);
		FillN(GetStr(), uCount, ch);
	}
	void Assign(const Char *pszBegin){
		Assign(View(pszBegin));
	}
	void Assign(const Char *pchBegin, const Char *pchEnd){
		Assign(View(pchBegin, pchEnd));
	}
	void Assign(const Char *pchBegin, std::size_t uCount){
		Assign(View(pchBegin, uCount));
	}
	void Assign(const View &rhs){
		Resize(rhs.GetSize());
		Copy(GetStr(), rhs.GetBegin(), rhs.GetEnd());
	}
	void Assign(std::initializer_list<Char> rhs){
		Assign(View(rhs));
	}
	void Assign(const VarChar &rhs) noexcept {
		if(&rhs != this){
			Assign(View(rhs));
		}
	}
	void Assign(VarChar &&rhs) noexcept {
		ASSERT(&rhs != this);

		Assign(View(rhs));
	}

	void Push(Char ch){
		Append(ch, 1);
	}
	void UncheckedPush(Char ch) noexcept {
		ASSERT(GetSize() < GetCapacity());

		auto &byComplLen = reinterpret_cast<std::make_unsigned_t<Char> &>(x_achData.GetEnd()[-1]);
		x_achData.UncheckedGet(byComplLen) = ch;
		--byComplLen;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT(uOldSize >= uCount);
		X_SetSize(uOldSize - uCount);
	}

	void Append(Char ch, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, ch);
	}
	void Append(const Char *pszBegin){
		Append(View(pszBegin));
	}
	void Append(const Char *pchBegin, const Char *pchEnd){
		Append(View(pchBegin, pchEnd));
	}
	void Append(const Char *pchBegin, std::size_t uCount){
		Append(View(pchBegin, uCount));
	}
	void Append(const View &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd());
	}
	void Append(std::initializer_list<Char> rhs){
		Append(View(rhs));
	}

	View Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().Slice(nBegin, nEnd);
	}
	VarChar SliceStr(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		return VarChar(Slice(nBegin, nEnd));
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

	std::size_t Find(const View &vToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().Find(vToFind, nBegin);
	}
	std::size_t FindBackward(const View &vToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindBackward(vToFind, nEnd);
	}
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().FindRep(chToFind, uRepCount, nBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindRepBackward(chToFind, uRepCount, nEnd);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().Find(chToFind, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindBackward(chToFind, nEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chRep, std::size_t uRepSize = 1){
		const auto vCurrent = GetView();
		const auto uOldSize = vCurrent.GetSize();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = static_cast<std::size_t>(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = static_cast<std::size_t>(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldSize - (uRemovedEnd - uRemovedBegin);
		const auto uNewSize = uLengthAfterRemoved + uRepSize;
		if(uNewSize < uLengthAfterRemoved){
			throw std::bad_array_new_length();
		}

		const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uRepSize);
		FillN(pchWrite, uRepSize, chRep);
		X_SetSize(uNewSize);
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin){
		Replace(nBegin, nEnd, View(pchRepBegin));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, const Char *pchRepEnd){
		Replace(nBegin, nEnd, View(pchRepBegin, pchRepEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, std::size_t uLen){
		Replace(nBegin, nEnd, View(pchRepBegin, uLen));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const View &vRep){
		const auto uRepSize = vRep.GetSize();

		const auto vCurrent = GetView();
		const auto uOldSize = vCurrent.GetSize();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = static_cast<std::size_t>(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = static_cast<std::size_t>(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldSize - (uRemovedEnd - uRemovedBegin);
		const auto uNewSize = uLengthAfterRemoved + uRepSize;
		if(uNewSize < uLengthAfterRemoved){
			throw std::bad_array_new_length();
		}

		if(vCurrent.DoesOverlapWith(vRep)){
			VarChar vchTemp;
			vchTemp.Resize(uNewSize);
			auto pchWrite = vchTemp.GetData();
			pchWrite = Copy(pchWrite, vCurrent.GetBegin(), vCurrent.GetBegin() + uRemovedBegin);
			pchWrite = Copy(pchWrite, vRep.GetBegin(), vRep.GetEnd());
			pchWrite = Copy(pchWrite, vCurrent.GetBegin() + uRemovedEnd, vCurrent.GetEnd());
			Assign(std::move(vchTemp));
		} else {
			const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + vRep.GetSize());
			CopyN(pchWrite, vRep.GetBegin(), vRep.GetSize());
			X_SetSize(uNewSize);
		}
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
	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

	operator ArrayView<const Element>() const noexcept {
		return ArrayView<const Element>(GetData(), GetSize());
	}
	operator ArrayView<Element>() noexcept {
		return ArrayView<Element>(GetData(), GetSize());
	}

	operator NarrowStringView() const noexcept {
		return NarrowStringView(GetData(), GetSize());
	}
};

template<unsigned kCapacityT>
const VarChar<kCapacityT> VarChar<kCapacityT>::kEmpty;

template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> &operator+=(VarChar<kCapacityT> &lhs, const VarChar<kOtherCapacityT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> &operator+=(VarChar<kCapacityT> &lhs, const StringView<kOtherCapacityT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &operator+=(VarChar<kCapacityT> &lhs, typename VarChar<kCapacityT>::Char rhs){
	lhs.Append(rhs);
	return lhs;
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &operator+=(VarChar<kCapacityT> &lhs, const typename VarChar<kCapacityT>::Char *rhs){
	lhs.Append(rhs);
	return lhs;
}
template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> &&operator+=(VarChar<kCapacityT> &&lhs, const VarChar<kOtherCapacityT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> &&operator+=(VarChar<kCapacityT> &&lhs, const StringView<kOtherCapacityT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &&operator+=(VarChar<kCapacityT> &&lhs, typename VarChar<kCapacityT>::Char rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &&operator+=(VarChar<kCapacityT> &&lhs, const typename VarChar<kCapacityT>::Char *rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &&operator+=(VarChar<kCapacityT> &&lhs, VarChar<kCapacityT> &&rhs){
	lhs.Append(std::move(rhs));
	return std::move(lhs);
}

template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> operator+(const VarChar<kCapacityT> &lhs, const VarChar<kOtherCapacityT> &rhs){
	auto ret = lhs;
	ret += rhs;
	return ret;
}
template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> operator+(const VarChar<kCapacityT> &lhs, const StringView<kOtherCapacityT> &rhs){
	auto ret = lhs;
	ret += rhs;
	return ret;
}
template<unsigned kCapacityT>
VarChar<kCapacityT> operator+(const VarChar<kCapacityT> &lhs, typename VarChar<kCapacityT>::Char rhs){
	auto ret = lhs;
	ret += rhs;
	return ret;
}
template<unsigned kCapacityT>
VarChar<kCapacityT> operator+(const VarChar<kCapacityT> &lhs, const typename VarChar<kCapacityT>::Char *rhs){
	auto ret = lhs;
	ret += rhs;
	return ret;
}
template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> &&operator+(VarChar<kCapacityT> &&lhs, const VarChar<kOtherCapacityT> &rhs){
	return std::move(lhs += rhs);
}
template<unsigned kCapacityT, unsigned kOtherCapacityT>
VarChar<kCapacityT> &&operator+(VarChar<kCapacityT> &&lhs, const StringView<kOtherCapacityT> &rhs){
	return std::move(lhs += rhs);
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &&operator+(VarChar<kCapacityT> &&lhs, typename VarChar<kCapacityT>::Char rhs){
	return std::move(lhs += rhs);
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &&operator+(VarChar<kCapacityT> &&lhs, const typename VarChar<kCapacityT>::Char *rhs){
	return std::move(lhs += rhs);
}
template<unsigned kCapacityT>
VarChar<kCapacityT> &&operator+(VarChar<kCapacityT> &&lhs, VarChar<kCapacityT> &&rhs){
	return std::move(lhs += std::move(rhs));
}

template<unsigned kCapacityT>
bool operator==(const VarChar<kCapacityT> &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs.GetView() == rhs.GetView();
}
template<unsigned kCapacityT>
bool operator==(const VarChar<kCapacityT> &lhs, const NarrowStringView &rhs) noexcept {
	return lhs.GetView() == rhs;
}
template<unsigned kCapacityT>
bool operator==(const NarrowStringView &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs == rhs.GetView();
}

template<unsigned kCapacityT>
bool operator!=(const VarChar<kCapacityT> &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs.GetView() != rhs.GetView();
}
template<unsigned kCapacityT>
bool operator!=(const VarChar<kCapacityT> &lhs, const NarrowStringView &rhs) noexcept {
	return lhs.GetView() != rhs;
}
template<unsigned kCapacityT>
bool operator!=(const NarrowStringView &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs != rhs.GetView();
}

template<unsigned kCapacityT>
bool operator<(const VarChar<kCapacityT> &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs.GetView() < rhs.GetView();
}
template<unsigned kCapacityT>
bool operator<(const VarChar<kCapacityT> &lhs, const NarrowStringView &rhs) noexcept {
	return lhs.GetView() < rhs;
}
template<unsigned kCapacityT>
bool operator<(const NarrowStringView &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs < rhs.GetView();
}

template<unsigned kCapacityT>
bool operator>(const VarChar<kCapacityT> &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs.GetView() > rhs.GetView();
}
template<unsigned kCapacityT>
bool operator>(const VarChar<kCapacityT> &lhs, const NarrowStringView &rhs) noexcept {
	return lhs.GetView() > rhs;
}
template<unsigned kCapacityT>
bool operator>(const NarrowStringView &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs > rhs.GetView();
}

template<unsigned kCapacityT>
bool operator<=(const VarChar<kCapacityT> &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs.GetView() <= rhs.GetView();
}
template<unsigned kCapacityT>
bool operator<=(const VarChar<kCapacityT> &lhs, const NarrowStringView &rhs) noexcept {
	return lhs.GetView() <= rhs;
}
template<unsigned kCapacityT>
bool operator<=(const NarrowStringView &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs <= rhs.GetView();
}

template<unsigned kCapacityT>
bool operator>=(const VarChar<kCapacityT> &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs.GetView() >= rhs.GetView();
}
template<unsigned kCapacityT>
bool operator>=(const VarChar<kCapacityT> &lhs, const NarrowStringView &rhs) noexcept {
	return lhs.GetView() >= rhs;
}
template<unsigned kCapacityT>
bool operator>=(const NarrowStringView &lhs, const VarChar<kCapacityT> &rhs) noexcept {
	return lhs >= rhs.GetView();
}

template<unsigned kCapacityT>
void swap(VarChar<kCapacityT> &lhs, VarChar<kCapacityT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<unsigned kCapacityT>
decltype(auto) begin(const VarChar<kCapacityT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<unsigned kCapacityT>
decltype(auto) begin(VarChar<kCapacityT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<unsigned kCapacityT>
decltype(auto) cbegin(const VarChar<kCapacityT> &rhs) noexcept {
	return begin(rhs);
}
template<unsigned kCapacityT>
decltype(auto) end(const VarChar<kCapacityT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<unsigned kCapacityT>
decltype(auto) end(VarChar<kCapacityT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<unsigned kCapacityT>
decltype(auto) cend(const VarChar<kCapacityT> &rhs) noexcept {
	return end(rhs);
}

}

#endif
