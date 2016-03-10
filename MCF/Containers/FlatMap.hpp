// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_FLAT_MAP_HPP_
#define MCF_CONTAINERS_FLAT_MAP_HPP_

#include "_Enumerator.hpp"
#include "../Function/Comparators.hpp"
#include "../Utilities/ReconstructOrAssign.hpp"
#include "../Utilities/AddressOf.hpp"
#include "../Utilities/DeclVal.hpp"
#include "_FlatContainer.hpp"
#include <utility>
#include <tuple>

namespace MCF {

template<typename KeyT, typename ValueT, typename ComparatorT = Less>
class FlatMap {
public:
	// 容器需求。
	using Element         = std::pair<const KeyT, ValueT>;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <FlatMap>;
	using Enumerator      = Impl_Enumerator::Enumerator      <FlatMap>;

private:
	template<typename CvElementT, typename ComparandT>
	static CvElementT *X_GetLowerBound(CvElementT *pBegin, CvElementT *pEnd, const ComparandT &vComparand){
		auto pLower = pBegin, pUpper = pEnd;
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(pMiddle->first, vComparand)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		return pLower;
	}
	template<typename CvElementT, typename ComparandT>
	static CvElementT *X_GetUpperBound(CvElementT *pBegin, CvElementT *pEnd, const ComparandT &vComparand){
		auto pLower = pBegin, pUpper = pEnd;
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(!ComparatorT()(vComparand, pMiddle->first)){
				pLower = pMiddle + 1;
			} else {
				pUpper = pMiddle;
			}
		}
		return pLower;
	}
	template<typename CvElementT, typename ComparandT>
	static CvElementT *X_GetMatch(CvElementT *pBegin, CvElementT *pEnd, const ComparandT &vComparand){
		auto pLower = pBegin, pUpper = pEnd;
		while(pLower != pUpper){
			const auto pMiddle = pLower + (pUpper - pLower) / 2;
			if(ComparatorT()(pMiddle->first, vComparand)){
				pLower = pMiddle + 1;
			} else if(ComparatorT()(vComparand, pMiddle->first)){
				pUpper = pMiddle;
			} else {
				return pMiddle;
			}
		}
		return pEnd;
	}
	template<typename CvElementT, typename ComparandT>
	static std::pair<CvElementT *, CvElementT *> X_GetEqualRange(CvElementT *pBegin, CvElementT *pEnd, const ComparandT &vComparand){
		const auto pMiddle = X_GetMatch(pBegin, pEnd, vComparand);
		return std::make_pair(pMiddle, pMiddle);
	}

private:
	struct X_MoveCaster {
		std::pair<KeyT &&, ValueT &&> operator()(Element &rhs) const noexcept {
			return std::pair<KeyT &&, ValueT &&>(static_cast<KeyT &&>(const_cast<KeyT &>(rhs.first)), static_cast<ValueT &&>(rhs.second));
		}
		static constexpr bool kEnabled = std::is_nothrow_move_constructible<KeyT>::value && std::is_nothrow_move_constructible<ValueT>::value;
	};
	Impl_FlatContainer::FlatContainer<Element, X_MoveCaster> x_vStorage;

public:
	constexpr FlatMap() noexcept
		: x_vStorage()
	{
	}
	// 如果键有序，则效率最大化；并且是稳定的。
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	FlatMap(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: FlatMap()
	{
		if(std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			Reserve(uDeltaSize);
		}
		for(auto it = itBegin; it != itEnd; ++it){
			AddWithHint(nullptr, *it);
		}
	}
	// 如果键有序，则效率最大化；并且是稳定的。
	FlatMap(std::initializer_list<Element> rhs)
		: FlatMap(rhs.begin(), rhs.end())
	{
	}
	FlatMap(const FlatMap &rhs)
		: x_vStorage(rhs.x_vStorage)
	{
	}
	FlatMap(FlatMap &&rhs) noexcept
		: x_vStorage(std::move(rhs.x_vStorage))
	{
	}
	FlatMap &operator=(const FlatMap &rhs){
		FlatMap(rhs).Swap(*this);
		return *this;
	}
	FlatMap &operator=(FlatMap &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return x_vStorage.IsEmpty();
	}
	void Clear() noexcept {
		x_vStorage.Clear();
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		return x_vStorage.Extract(itOutput);
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

	void Swap(FlatMap &rhs) noexcept {
		using std::swap;
		swap(x_vStorage, rhs.x_vStorage);
	}

	// FlatMap 需求。
	const Element *GetData() const noexcept {
		return x_vStorage.GetData();
	}
	Element *GetData() noexcept {
		return x_vStorage.GetData();
	}
	const Element *GetConstData() const noexcept {
		return GetData();
	}
	std::size_t GetSize() const noexcept {
		return x_vStorage.GetSize();
	}
	std::size_t GetCapacity() const noexcept {
		return x_vStorage.GetCapacity();
	}

	const Element *GetBegin() const noexcept {
		return x_vStorage.GetBegin();
	}
	Element *GetBegin() noexcept {
		return x_vStorage.GetBegin();
	}
	const Element *GetConstBegin() const noexcept {
		return GetBegin();
	}
	const Element *GetEnd() const noexcept {
		return x_vStorage.GetEnd();
	}
	Element *GetEnd() noexcept {
		return x_vStorage.GetEnd();
	}
	const Element *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"FlatMap: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"FlatMap: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		return GetData()[uIndex];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		return GetData()[uIndex];
	}

	void Reserve(std::size_t uNewCapacity){
		x_vStorage.Reserve(uNewCapacity);
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		x_vStorage.ReserveMore(uDeltaCapacity);
	}

	template<typename ComparandT, typename ...ValueParamsT>
	std::pair<Element *, bool> Add(ComparandT &&vComparand, ValueParamsT &&...vValueParams){
		return AddWithHint(nullptr, std::forward<ComparandT>(vComparand), std::forward<ValueParamsT>(vValueParams)...);
	}
	template<typename ComparandT, typename ...ValueParamsT>
	std::pair<Element *, bool> AddWithHint(const Element *pHint, ComparandT &&vComparand, ValueParamsT &&...vValueParams){
		if(!pHint){
			pHint = GetEnd();
			if((pHint == GetBegin()) || !ComparatorT()(vComparand, pHint[-1].first)){
				goto jUseHint;
			}
		} else {
			if(((pHint == GetBegin()) || !ComparatorT()(vComparand, pHint[-1].first)) && ((pHint == GetEnd()) || ComparatorT()(vComparand, pHint[0].first))){
				goto jUseHint;
			}
		}
		pHint = X_GetUpperBound(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	jUseHint:
		if((pHint != GetBegin()) && !ComparatorT()(pHint[-1].first, vComparand)){
			return std::make_pair(const_cast<Element *>(pHint), false);
		}
		return std::make_pair(x_vStorage.Emplace(pHint, std::piecewise_construct,
			std::forward_as_tuple(std::forward<ComparandT>(vComparand)), std::forward_as_tuple(std::forward<ValueParamsT>(vValueParams)...)), true);
	}
	template<typename FirstT, typename SecondT>
	std::pair<Element *, bool> AddWithHint(const Element *pHint, const std::pair<FirstT, SecondT> &vPair){
		return AddWithHint(pHint, vPair.first, vPair.second);
	}
	template<typename FirstT, typename SecondT>
	std::pair<Element *, bool> AddWithHint(const Element *pHint, std::pair<FirstT, SecondT> &&vPair){
		return AddWithHint(pHint, std::move(vPair.first), std::move(vPair.second));
	}
	template<typename ComparandT, typename ...ValueParamsT>
	std::pair<Element *, bool> Replace(ComparandT &&vComparand, ValueParamsT &&...vValueParams){
		return ReplaceWithHint(nullptr, std::forward<ComparandT>(vComparand), std::forward<ValueParamsT>(vValueParams)...);
	}
	template<typename ComparandT, typename ...ValueParamsT>
	std::pair<Element *, bool> ReplaceWithHint(const Element *pHint, ComparandT &&vComparand, ValueParamsT &&...vValueParams){
		const auto vResult = AddWithHint(pHint, std::forward<ComparandT>(vComparand), std::forward<ValueParamsT>(vValueParams)...);
		if(!vResult.second){
			ReconstructOrAssign(AddressOf(vResult.first->second), std::forward<ValueParamsT>(vValueParams)...);
		}
		return vResult;
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

	template<typename ComparandT, typename ...ValueParamsT>
	Element *Emplace(const Element *pPos, ComparandT &&vComparand, ValueParamsT &&...vValueParams){
		return AddWithHint(pPos, std::forward<ComparandT>(vComparand), std::forward<ValueParamsT>(vValueParams)...).first;
	}
	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(noexcept(DeclVal<decltype((x_vStorage))>().Erase(pBegin, pEnd))) {
		return x_vStorage.Erase(pBegin, pEnd);
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(DeclVal<decltype((x_vStorage))>().Erase(pPos))) {
		return x_vStorage.Erase(pPos);
	}

	template<typename ComparandT>
	const Element *GetLowerBound(const ComparandT &vComparand) const {
		return X_GetLowerBound(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	Element *GetLowerBound(const ComparandT &vComparand){
		return X_GetLowerBound(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	const Element *GetConstLowerBound(const ComparandT &vComparand) const {
		return GetLowerBound(vComparand);
	}

	template<typename ComparandT>
	const Element *GetUpperBound(const ComparandT &vComparand) const {
		return X_GetUpperBound(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	Element *GetUpperBound(const ComparandT &vComparand){
		return X_GetUpperBound(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	const Element *GetConstUpperBound(const ComparandT &vComparand) const {
		return GetUpperBound(vComparand);
	}

	template<typename ComparandT>
	const Element *GetMatch(const ComparandT &vComparand) const {
		return X_GetMatch(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	Element *GetMatch(const ComparandT &vComparand){
		return X_GetMatch(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	const Element *GetConstMatch(const ComparandT &vComparand) const {
		return GetMatch(vComparand);
	}

	template<typename ComparandT>
	std::pair<const Element *, const Element *> GetEqualRange(const ComparandT &vComparand) const {
		return X_GetEqualRange(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
	}
	template<typename ComparandT>
	std::pair<Element *, Element *> GetEqualRange(const ComparandT &vComparand){
		return X_GetEqualRange(x_vStorage.GetBegin(), x_vStorage.GetEnd(), vComparand);
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
		const auto pPos = GetMatch(vComparand);
		if(pPos == GetEnd()){
			return ConstEnumerator(*this, nullptr);
		}
		return ConstEnumerator(*this, pPos);
	}
	template<typename ComparandT>
	Enumerator EnumerateMatch(const ComparandT &vComparand){
		const auto pPos = GetMatch(vComparand);
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

	friend void swap(FlatMap &lhs, FlatMap &rhs) noexcept {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const FlatMap &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) begin(FlatMap &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const FlatMap &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const FlatMap &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) end(FlatMap &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) cend(const FlatMap &rhs) noexcept {
		return end(rhs);
	}
};

}

#endif
