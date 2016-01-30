// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_STATIC_VECTOR_HPP_
#define MCF_CONTAINERS_STATIC_VECTOR_HPP_

#include "_Enumerator.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/AlignedUnion.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <new>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT, std::size_t kCapacity>
class StaticVector {
	static_assert(kCapacity > 0, "A StaticVector shall have a non-zero capacity.");

public:
	// 容器需求。
	using Element         = ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <StaticVector>;
	using Enumerator      = Impl_Enumerator::Enumerator      <StaticVector>;

private:
	AlignedUnion<0, Element> x_aStorage[kCapacity];
	std::size_t x_uSize;

public:
	StaticVector() noexcept
		: x_uSize(0)
	{
	}
	template<typename ...ParamsT>
	explicit StaticVector(std::size_t uSize, const ParamsT &...vParams)
		: StaticVector()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	StaticVector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: StaticVector()
	{
		Append(itBegin, itEnd);
	}
	StaticVector(std::initializer_list<Element> rhs)
		: StaticVector(rhs.begin(), rhs.end())
	{
	}
	StaticVector(const StaticVector &rhs) noexcept(std::is_nothrow_copy_constructible<Element>::value)
		: StaticVector()
	{
		Append(rhs.GetBegin(), rhs.GetEnd());
	}
	StaticVector(StaticVector &&rhs) noexcept(std::is_nothrow_move_constructible<Element>::value)
		: StaticVector()
	{
		Append(std::make_move_iterator(rhs.GetBegin()), std::make_move_iterator(rhs.GetEnd()));
	}
	StaticVector &operator=(const StaticVector &rhs) noexcept(std::is_nothrow_copy_constructible<Element>::value) {
		Clear();
		Append(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	StaticVector &operator=(StaticVector &&rhs) noexcept(std::is_nothrow_move_constructible<Element>::value) {
		Clear();
		Append(std::make_move_iterator(rhs.GetBegin()), std::make_move_iterator(rhs.GetEnd()));
		return *this;
	}
	~StaticVector(){
		Clear();
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear() noexcept {
		Pop(x_uSize);
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			const auto pBegin = GetBegin();
			const auto pEnd = GetEnd();
			for(auto p = pBegin; p != pEnd; ++p){
				*itOutput = std::move(*p);
				++itOutput;
			}
		} catch(...){
			Clear();
			throw;
		}
		Clear();
		return itOutput;
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
		if(uOffset == x_uSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
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

	void Swap(StaticVector &rhs) noexcept(std::is_nothrow_move_constructible<Element>::value) {
		auto vecTemp = std::move_if_noexcept(rhs);
		*this = std::move(rhs);
		rhs = std::move(vecTemp);
	}

	// StaticVector 需求。
	const Element *GetData() const noexcept {
		return reinterpret_cast<const Element *>(x_aStorage);
	}
	Element *GetData() noexcept {
		return reinterpret_cast<Element *>(x_aStorage);
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	static constexpr std::size_t GetCapacity() noexcept {
		return kCapacity;
	}

	const Element *GetBegin() const noexcept {
		return GetData();
	}
	Element *GetBegin() noexcept {
		return GetData();
	}
	const Element *GetConstBegin() const noexcept {
		return GetBegin();
	}
	const Element *GetEnd() const noexcept {
		return GetData() + GetSize();
	}
	Element *GetEnd() noexcept {
		return GetData() + GetSize();
	}
	const Element *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "StaticVector: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "StaticVector: Subscript out of range"_rcs);
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

	template<typename ...ParamsT>
	void Resize(std::size_t uSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		if(uSize > uOldSize){
			Append(uSize - uOldSize, vParams...);
		} else {
			Pop(uOldSize - uSize);
		}
	}
	template<typename ...ParamsT>
	Element *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		Append(uDeltaSize - uOldSize, vParams...);
		return GetData() + uOldSize;
	}

	void Reserve(std::size_t uNewCapacity){
		if(uNewCapacity > GetCapacity()){
			DEBUG_THROW(Exception, ERROR_OUTOFMEMORY, "StaticVector: Max capacity exceeded"_rcs);
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

	template<typename ...ParamsT>
	Element &Push(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto pData = GetData();
		const auto pElement = pData + x_uSize;
		DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		++x_uSize;

		return *pElement;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		const auto pData = GetData();
		for(std::size_t i = 0; i < uCount; ++i){
			Destruct(pData + x_uSize - 1 - i);
		}
		x_uSize -= uCount;
	}

	template<typename ...ParamsT>
	void Append(std::size_t uDeltaSize, const ParamsT &...vParams){
		ReserveMore(uDeltaSize);

		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				UncheckedPush(vParams...);
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		if(kHasDeltaSizeHint){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			ReserveMore(uDeltaSize);
		}

		std::size_t uElementsPushed = 0;
		try {
			for(auto it = itBegin; it != itEnd; ++it){
				if(kHasDeltaSizeHint){
					UncheckedPush(*it);
				} else {
					Push(*it);
				}
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	void Append(std::initializer_list<Element> ilElements){
		Append(ilElements.begin(), ilElements.end());
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

	friend void swap(StaticVector &lhs, StaticVector &rhs) noexcept(noexcept(lhs.Swap(rhs))) {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const StaticVector &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) begin(StaticVector &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const StaticVector &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const StaticVector &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) end(StaticVector &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) cend(const StaticVector &rhs) noexcept {
		return end(rhs);
	}
};

}

#endif
