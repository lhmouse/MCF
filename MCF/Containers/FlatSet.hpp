// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_FLAT_SET_HPP_
#define MCF_CONTAINERS_FLAT_SET_HPP_

#include "../Utilities/RationalFunctors.hpp"
#include "Vector.hpp"
#include <utility>

namespace MCF {

template<typename ElementT, typename ComparatorT = Less>
class FlatSet {
private:
	Vector<const ElementT> x_vecStorage;

public:
	constexpr FlatSet() noexcept
		: x_vecStorage()
	{
	}
	template<typename ...ParamsT>
	explicit FlatSet(std::size_t uSize, const ParamsT &...vParams)
		: x_vecStorage(uSize, vParams...)
	{
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	FlatSet(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: FlatSet()
	{
		if(std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			Reserve(uDeltaSize);
		}
		for(auto it = itBegin; it != itEnd; ++it){
			AddUsingHint(nullptr, *it);
		}
	}
	FlatSet(std::initializer_list<ElementT> rhs)
		: FlatSet(rhs.begin(), rhs.end())
	{
	}
	FlatSet(const FlatSet &rhs)
		: x_vecStorage(rhs.x_vecStorage)
	{
	}
	FlatSet(FlatSet &&rhs) noexcept
		: x_vecStorage(std::move(rhs.x_vecStorage))
	{
	}
	FlatSet &operator=(const FlatSet &rhs){
		FlatSet(rhs).Swap(*this);
		return *this;
	}
	FlatSet &operator=(FlatSet &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}

public:
	// 容器需求。
	using Element         = const ElementT;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <FlatSet>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <FlatSet>;

	bool IsEmpty() const noexcept {
		return x_vecStorage.IsEmpty();
	}
	void Clear() noexcept {
		x_vecStorage.Clear();
	}

	const Element *GetFirst() const noexcept {
		return x_vecStorage.GetFirst();
	}
	Element *GetFirst() noexcept {
		return x_vecStorage.GetFirst();
	}
	const Element *GetConstFirst() const noexcept {
		return x_vecStorage.GetConstFirst();
	}
	const Element *GetLast() const noexcept {
		return x_vecStorage.GetLast();
	}
	Element *GetLast() noexcept {
		return x_vecStorage.GetLast();
	}
	const Element *GetConstLast() const noexcept {
		return x_vecStorage.GetConstLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		return x_vecStorage.GetPrev(pPos);
	}
	Element *GetPrev(Element *pPos) noexcept {
		return x_vecStorage.GetPrev(pPos);
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		return x_vecStorage.GetNext(pPos);
	}
	Element *GetNext(Element *pPos) noexcept {
		return x_vecStorage.GetNext(pPos);
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

	void Swap(FlatSet &rhs) noexcept {
		x_vecStorage.Swap(rhs.x_vecStorage);
	}

	// FlatSet 需求。
	const Element *GetData() const noexcept {
		return x_vecStorage.GetData();
	}
	Element *GetData() noexcept {
		return x_vecStorage.GetData();
	}
	const Element *GetConstData() const noexcept {
		return x_vecStorage.GetConstData();
	}
	std::size_t GetSize() const noexcept {
		return x_vecStorage.GetSize();
	}
	std::size_t GetCapacity() noexcept {
		return x_vecStorage.GetCapacity();
	}

	const Element *GetBegin() const noexcept {
		return x_vecStorage.GetBegin();
	}
	Element *GetBegin() noexcept {
		return x_vecStorage.GetBegin();
	}
	const Element *GetConstBegin() const noexcept {
		return x_vecStorage.GetConstBegin();
	}
	const Element *GetEnd() const noexcept {
		return x_vecStorage.GetEnd();
	}
	Element *GetEnd() noexcept {
		return x_vecStorage.GetEnd();
	}
	const Element *GetConstEnd() const noexcept {
		return x_vecStorage.GetConstEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		return x_vecStorage.Get(uIndex);
	}
	Element &Get(std::size_t uIndex){
		return x_vecStorage.Get(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		return x_vecStorage.UncheckedGet(uIndex);
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		return x_vecStorage.UncheckedGet(uIndex);
	}

	void Reserve(std::size_t uNewCapacity){
		x_vecStorage.Reserve(uNewCapacity);
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		x_vecStorage.ReserveMore(uDeltaCapacity);
	}

	template<typename ComparandT>
	std::pair<Element *, bool> Add(ComparandT &&vComparand){
		return AddUsingHint(nullptr, std::forward<ComparandT>(vComparand));
	}
	template<typename ComparandT>
	std::pair<Element *, bool> AddUsingHint(const Element *pHint, ComparandT &&vComparand){
		if(!pHint){
			pHint = GetEnd();
			if((pHint == GetBegin()) || !ComparatorT()(vComparand, pHint[-1])){
				goto jUseHint;
			}
		} else {
			if(((pHint == GetBegin()) || !ComparatorT()(vComparand, pHint[-1])) && ((pHint == GetEnd()) || ComparatorT()(vComparand, pHint[0]))){
				goto jUseHint;
			}
		}
		pHint = GetUpperBound(vComparand);
	jUseHint:
		if((pHint != GetBegin()) && !ComparatorT()(pHint[-1], vComparand)){
			return std::make_pair(pHint, false);
		}
		return std::make_pair(x_vecStorage.Emplace(pHint, std::forward<ComparandT>(vComparand)), true);
	}
	template<typename ComparandT>
	bool Remove(const ComparandT &vComparand){
		const auto vRange = GetEqualRange(vComparand);
		if(vRange.first == vRange.second){
			return false;
		}
		Erase(vRange.first, vRange.second);
		return true;
	}

	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(noexcept(std::declval<FlatSet &>().x_vecStorage.Erase(pBegin, pEnd))) {
		return x_vecStorage.Erase(pBegin, pEnd);
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(std::declval<FlatSet &>().x_vecStorage.Erase(pPos))) {
		return x_vecStorage.Erase(pPos);
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return x_vecStorage[uIndex];
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return x_vecStorage[uIndex];
	}

	template<typename ComparandT>
	const Element *GetLowerBound(const ComparandT &vComparand) const {
		auto pLower = x_vecStorage.GetBegin(), pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(*pMiddle, vComparand)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		return pLower;
	}
	template<typename ComparandT>
	Element *GetLowerBound(const ComparandT &vComparand){
		auto pLower = x_vecStorage.GetBegin(), pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(*pMiddle, vComparand)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		return pLower;
	}
	template<typename ComparandT>
	const Element *GetConstLowerBound(const ComparandT &vComparand) const {
		return GetLowerBound(vComparand);
	}

	template<typename ComparandT>
	const Element *GetUpperBound(const ComparandT &vComparand) const {
		auto pLower = x_vecStorage.GetBegin(), pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(!ComparatorT()(vComparand, *pMiddle)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		return pLower;
	}
	template<typename ComparandT>
	Element *GetUpperBound(const ComparandT &vComparand){
		auto pLower = x_vecStorage.GetBegin(), pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(!ComparatorT()(vComparand, *pMiddle)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		return pLower;
	}
	template<typename ComparandT>
	const Element *GetConstUpperBound(const ComparandT &vComparand) const {
		return GetUpperBound(vComparand);
	}

	template<typename ComparandT>
	const Element *Find(const ComparandT &vComparand) const {
		auto pLower = x_vecStorage.GetBegin(), pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(*pMiddle, vComparand)){
				pLower = pMiddle + 1;
			} else if(ComparatorT()(vComparand, *pMiddle)){
				pUpper = pMiddle;
			} else {
				return pMiddle;
			}
		}
		return x_vecStorage.GetEnd();
	}
	template<typename ComparandT>
	Element *Find(const ComparandT &vComparand){
		auto pLower = x_vecStorage.GetBegin(), pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(*pMiddle, vComparand)){
				pLower = pMiddle + 1;
			} else if(ComparatorT()(vComparand, *pMiddle)){
				pUpper = pMiddle;
			} else {
				return pMiddle;
			}
		}
		return x_vecStorage.GetEnd();
	}
	template<typename ComparandT>
	const Element *FindConst(const ComparandT &vComparand) const {
		return Find(vComparand);
	}

	template<typename ComparandT>
	std::pair<const Element *, const Element *> GetEqualRange(const ComparandT &vComparand) const {
		auto vRange = std::make_pair(x_vecStorage.GetEnd(), x_vecStorage.GetEnd());

		const auto pTop = Find(vComparand);
		if(pTop == x_vecStorage.GetEnd()){
			return vRange;
		}

		auto pLower = x_vecStorage.GetBegin(), pUpper = pTop;
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(*pMiddle, vComparand)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		vRange.first = pLower;

		pLower = pTop, pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(!ComparatorT()(vComparand, *pMiddle)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		vRange.second = pUpper;

		return vRange;
	}
	template<typename ComparandT>
	std::pair<Element *, Element *> GetEqualRange(const ComparandT &vComparand){
		auto vRange = std::make_pair(x_vecStorage.GetEnd(), x_vecStorage.GetEnd());

		const auto pTop = Find(vComparand);
		if(pTop == x_vecStorage.GetEnd()){
			return vRange;
		}

		auto pLower = x_vecStorage.GetBegin(), pUpper = pTop;
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(*pMiddle, vComparand)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		vRange.first = pLower;

		pLower = pTop, pUpper = x_vecStorage.GetEnd();
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(!ComparatorT()(vComparand, *pMiddle)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		vRange.second = pUpper;

		return vRange;
	}
	template<typename ComparandT>
	std::pair<const Element *, const Element *> GetConstEqualRange(const ComparandT &vComparand) const {
		return GetEqualRange(vComparand);
	}

	template<typename ComparandT>
	ConstEnumerator EnumerateLowerBound(const ComparandT &vComparand) const {
		const auto pPos = GetLowerBound(vComparand);
		if(pPos == GetEnd()){
			return ConstEnumerator(*this, nullptr);
		}
		return ConstEnumerator(*this, pPos);
	}
	template<typename ComparandT>
	Enumerator EnumerateLowerBound(const ComparandT &vComparand){
		const auto pPos = GetLowerBound(vComparand);
		if(pPos == GetEnd()){
			return Enumerator(*this, nullptr);
		}
		return Enumerator(*this, pPos);
	}
	template<typename ComparandT>
	ConstEnumerator EnumerateConstLowerBound(const ComparandT &vComparand) const {
		return EnumerateLowerBound(vComparand);
	}

	template<typename ComparandT>
	ConstEnumerator EnumerateUpperBound(const ComparandT &vComparand) const {
		const auto pPos = GetUpperBound(vComparand);
		if(pPos == GetEnd()){
			return ConstEnumerator(*this, nullptr);
		}
		return ConstEnumerator(*this, pPos);
	}
	template<typename ComparandT>
	Enumerator EnumerateUpperBound(const ComparandT &vComparand){
		const auto pPos = GetUpperBound(vComparand);
		if(pPos == GetEnd()){
			return Enumerator(*this, nullptr);
		}
		return Enumerator(*this, pPos);
	}
	template<typename ComparandT>
	ConstEnumerator EnumerateConstUpperBound(const ComparandT &vComparand) const {
		return EnumerateUpperBound(vComparand);
	}

	template<typename ComparandT>
	ConstEnumerator EnumerateMatch(const ComparandT &vComparand) const {
		const auto pPos = Find(vComparand);
		if(pPos == GetEnd()){
			return ConstEnumerator(*this, nullptr);
		}
		return ConstEnumerator(*this, pPos);
	}
	template<typename ComparandT>
	Enumerator EnumerateMatch(const ComparandT &vComparand){
		const auto pPos = Find(vComparand);
		if(pPos == GetEnd()){
			return Enumerator(*this, nullptr);
		}
		return Enumerator(*this, pPos);
	}
	template<typename ComparandT>
	ConstEnumerator EnumerateConstMatch(const ComparandT &vComparand) const {
		return EnumerateMatch(vComparand);
	}

	template<typename ComparandT>
	std::pair<ConstEnumerator, ConstEnumerator> EnumerateEqualRange(const ComparandT &vComparand) const {
		auto vRange = GetEqualRange(vComparand);
		if(vRange.first == GetEnd()){
			vRange.first = nullptr;
		}
		if(vRange.second == GetEnd()){
			vRange.second = nullptr;
		}
		return std::make_pair(ConstEnumerator(*this, vRange.first), ConstEnumerator(*this, vRange.second));
	}
	template<typename ComparandT>
	std::pair<Enumerator, Enumerator> EnumerateEqualRange(const ComparandT &vComparand){
		auto vRange = GetEqualRange(vComparand);
		if(vRange.first == GetEnd()){
			vRange.first = nullptr;
		}
		if(vRange.second == GetEnd()){
			vRange.second = nullptr;
		}
		return std::make_pair(Enumerator(*this, vRange.first), Enumerator(*this, vRange.second));
	}
	template<typename ComparandT>
	std::pair<ConstEnumerator, ConstEnumerator> EnumerateConstEqualRange(const ComparandT &vComparand) const {
		return EnumerateEqualRange(vComparand);
	}
};

template<typename ElementT, typename ComparatorT>
void swap(FlatSet<ElementT, ComparatorT> &lhs, FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ElementT, typename ComparatorT>
decltype(auto) begin(const FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT, typename ComparatorT>
decltype(auto) begin(FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT, typename ComparatorT>
decltype(auto) cbegin(const FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	return begin(rhs);
}
template<typename ElementT, typename ComparatorT>
decltype(auto) end(const FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT, typename ComparatorT>
decltype(auto) end(FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT, typename ComparatorT>
decltype(auto) cend(const FlatSet<ElementT, ComparatorT> &rhs) noexcept {
	return end(rhs);
}

}

#endif
