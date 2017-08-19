// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_CIRCULAR_QUEUE_HPP_
#define MCF_CONTAINERS_CIRCULAR_QUEUE_HPP_

#include "../Core/_Enumerator.hpp"
#include "../Core/DefaultAllocator.hpp"
#include "../Core/Assert.hpp"
#include "../Core/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include "../Core/_CheckedSizeArithmetic.hpp"
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT, class AllocatorT = DefaultAllocator>
class CircularQueue {
public:
	// 容器需求。
	using Element         = ElementT;
	using Allocator       = AllocatorT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <CircularQueue>;
	using Enumerator      = Impl_Enumerator::Enumerator      <CircularQueue>;

private:
	Element *x_pStorage;
	std::size_t x_uBegin;
	std::size_t x_uEnd;
	std::size_t x_uCircularCap;

public:
	constexpr CircularQueue() noexcept
		: x_pStorage(nullptr), x_uBegin(0), x_uEnd(0), x_uCircularCap(1)
	{ }
	template<typename ...ParamsT>
	explicit CircularQueue(std::size_t uSize, const ParamsT &...vParams)
		: CircularQueue()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	CircularQueue(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: CircularQueue()
	{
		Append(itBegin, itEnd);
	}
	CircularQueue(std::initializer_list<Element> ilInitList)
		: CircularQueue(ilInitList.begin(), ilInitList.end())
	{ }
	CircularQueue(const CircularQueue &vOther)
		: CircularQueue()
	{
		Reserve(vOther.GetSize());
		vOther.X_IterateForward(vOther.x_uBegin, vOther.x_uEnd,
			[&, this](auto uIndex){
				this->UncheckedPush(vOther.x_pStorage[uIndex]);
			});
	}
	CircularQueue(CircularQueue &&vOther) noexcept
		: CircularQueue()
	{
		vOther.Swap(*this);
	}
	CircularQueue &operator=(const CircularQueue &vOther){
		if(std::is_nothrow_copy_constructible<Element>::value || IsEmpty()){
			Reserve(vOther.GetSize());
			try {
				vOther.X_IterateForward(vOther.x_uBegin, vOther.x_uEnd,
					[&, this](auto uIndex){
						this->UncheckedPush(vOther.x_pStorage[uIndex]);
					});
			} catch(...){
				Clear();
				throw;
			}
		} else {
			CircularQueue(vOther).Swap(*this);
		}
		return *this;
	}
	CircularQueue &operator=(CircularQueue &&vOther) noexcept {
		vOther.Swap(*this);
		return *this;
	}
	~CircularQueue(){
		Clear();
		Allocator()(static_cast<void *>(x_pStorage));
#ifndef NDEBUG
		__builtin_memset(&x_pStorage, 0xEF, sizeof(x_pStorage));
#endif
	}

private:
	template<typename CallbackT>
	void X_IterateForward(std::size_t uBegin, std::size_t uEnd, CallbackT &&fnCallback) const {
		if(uBegin <= uEnd){
			for(std::size_t uIndex = uBegin; uIndex != uEnd; ++uIndex){
				fnCallback(uIndex);
			}
		} else {
			for(std::size_t uIndex = uBegin; uIndex != x_uCircularCap; ++uIndex){
				fnCallback(uIndex);
			}
			for(std::size_t uIndex = 0; uIndex != uEnd; ++uIndex){
				fnCallback(uIndex);
			}
		}
	}
	template<typename CallbackT>
	void X_IterateBackward(std::size_t uBegin, std::size_t uEnd, CallbackT &&fnCallback) const {
		if(uBegin <= uEnd){
			for(std::size_t uIndex = uEnd; uIndex != uBegin; --uIndex){
				fnCallback(uIndex - 1);
			}
		} else {
			for(std::size_t uIndex = uEnd; uIndex != 0; --uIndex){
				fnCallback(uIndex - 1);
			}
			for(std::size_t uIndex = x_uCircularCap; uIndex != uBegin; --uIndex){
				fnCallback(uIndex - 1);
			}
		}
	}

	std::size_t X_Retreat(std::size_t uIndex, std::size_t uDelta) const noexcept {
		MCF_DEBUG_CHECK(uIndex < x_uCircularCap);
		MCF_DEBUG_CHECK(uDelta < x_uCircularCap);

		auto uNewIndex = uIndex - uDelta;
		if(uIndex < uDelta){
			uNewIndex += x_uCircularCap;
		}
		return uNewIndex;
	}
	std::size_t X_Advance(std::size_t uIndex, std::size_t uDelta) const noexcept {
		MCF_DEBUG_CHECK(uIndex < x_uCircularCap);
		MCF_DEBUG_CHECK(uDelta < x_uCircularCap);

		auto uNewIndex = uIndex + uDelta;
		if(x_uCircularCap - uIndex <= uDelta){
			uNewIndex -= x_uCircularCap;
		}
		return uNewIndex;
	}
	std::size_t X_Measure(std::size_t uBegin, std::size_t uEnd) const noexcept {
		return X_Retreat(uEnd, uBegin);
	}

	std::pair<std::size_t, bool> X_PrepareForInsertion(std::size_t uPos, std::size_t uDeltaSize){
		MCF_DEBUG_CHECK(std::is_nothrow_move_constructible<Element>::value);
		MCF_DEBUG_CHECK(!IsEmpty());
		MCF_DEBUG_CHECK(X_Advance(x_uBegin, uPos) <= GetSize());

		const auto uCountBefore = X_Measure(x_uBegin, uPos);
		const auto uCountAfter = X_Measure(uPos, x_uEnd);

		ReserveMore(uDeltaSize);

		if(uCountBefore >= uCountAfter){
			X_IterateBackward(uPos, x_uEnd,
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					const auto uDestinationIndex = X_Advance(uIndex, uDeltaSize);
					Construct(pStorage + uDestinationIndex, std::move(*(pStorage + uIndex)));
					Destruct(pStorage + uIndex);
				});
			return std::make_pair(uPos, true);
		} else {
			X_IterateForward(x_uBegin, uPos,
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					const auto uDestinationIndex = X_Retreat(uIndex, uDeltaSize);
					Construct(pStorage + uDestinationIndex, std::move(*(pStorage + uIndex)));
					Destruct(pStorage + uIndex);
				});
			return std::make_pair(X_Retreat(uPos, uDeltaSize), false);
		}
	}
	void X_UndoPreparation(const std::pair<std::size_t, bool> &vPrepared, std::size_t uDeltaSize) noexcept {
		MCF_DEBUG_CHECK(std::is_nothrow_move_constructible<Element>::value);
		MCF_DEBUG_CHECK(!IsEmpty());
		MCF_DEBUG_CHECK(uDeltaSize <= GetCapacity() - GetSize());
		MCF_DEBUG_CHECK(X_Advance(x_uBegin, vPrepared.second ? vPrepared.first : X_Advance(vPrepared.first, uDeltaSize)) <= GetSize());

		if(vPrepared.second){
			X_IterateForward(vPrepared.first, x_uEnd,
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					const auto uSourceIndex = X_Advance(uIndex, uDeltaSize);
					Construct(pStorage + uIndex, std::move(*(pStorage + uSourceIndex)));
					Destruct(pStorage + uSourceIndex);
				});
		} else {
			X_IterateBackward(x_uBegin, X_Advance(vPrepared.first, uDeltaSize),
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					const auto uSourceIndex = X_Retreat(uIndex, uDeltaSize);
					Construct(pStorage + uIndex, std::move(pStorage + uSourceIndex)));
					Destruct(pStorage + uSourceIndex);
				});
		}
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return x_uBegin == x_uEnd;
	}
	void Clear() noexcept {
		Shift(GetSize());
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			X_IterateForward(x_uBegin, x_uEnd,
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					*itOutput = std::move(pStorage[uIndex]);
					++itOutput;
				});
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
		return x_pStorage + x_uBegin;
	}
	Element *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return x_pStorage + x_uBegin;
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return x_pStorage + X_Retreat(x_uEnd, 1);
	}
	Element *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return x_pStorage + X_Retreat(x_uEnd, 1);
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		MCF_DEBUG_CHECK(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		if(uOffset == x_uBegin){
			return nullptr;
		}
		uOffset = X_Retreat(uOffset, 1);
		return x_pStorage + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		if(uOffset == x_uBegin){
			return nullptr;
		}
		uOffset = X_Retreat(uOffset, 1);
		return x_pStorage + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		MCF_DEBUG_CHECK(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		uOffset = X_Advance(uOffset, 1);
		if(uOffset == x_uEnd){
			return nullptr;
		}
		return x_pStorage + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		uOffset = X_Advance(uOffset, 1);
		if(uOffset == x_uEnd){
			return nullptr;
		}
		return x_pStorage + uOffset;
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

	void Swap(CircularQueue &vOther) noexcept {
		using std::swap;
		swap(x_pStorage,     vOther.x_pStorage);
		swap(x_uBegin,       vOther.x_uBegin);
		swap(x_uEnd,         vOther.x_uEnd);
		swap(x_uCircularCap, vOther.x_uCircularCap);
	}

	// CircularQueue 需求。
	std::size_t GetSize() const noexcept {
		return X_Measure(x_uBegin, x_uEnd);
	}
	std::size_t GetCapacity() const noexcept {
		return x_uCircularCap - 1;
	}
	std::size_t GetCapacityRemaining() const noexcept {
		return GetCapacity() - GetSize();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"CircularQueue: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"CircularQueue: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex < GetSize());

		return x_pStorage[X_Advance(x_uBegin, uIndex)];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		MCF_DEBUG_CHECK(uIndex < GetSize());

		return x_pStorage[X_Advance(x_uBegin, uIndex)];
	}

	std::pair<const Element *, std::size_t> GetLongestLeadingSequence() const noexcept {
		if(x_uBegin <= x_uEnd){
			return std::make_pair(x_pStorage + x_uBegin, x_uEnd - x_uBegin);
		} else {
			return std::make_pair(x_pStorage + x_uBegin, x_uCircularCap - x_uBegin);
		}
	}
	std::pair<Element *, std::size_t> GetLongestLeadingSequence() noexcept {
		if(x_uBegin <= x_uEnd){
			return std::make_pair(x_pStorage + x_uBegin, x_uEnd - x_uBegin);
		} else {
			return std::make_pair(x_pStorage + x_uBegin, x_uCircularCap - x_uBegin);
		}
	}

	std::pair<std::size_t, const Element *> GetLongestTrailingSequence() const noexcept {
		if(x_uBegin <= x_uEnd){
			return std::make_pair(x_uEnd - x_uBegin, x_pStorage + x_uEnd);
		} else {
			return std::make_pair(x_uEnd, x_pStorage + x_uEnd);
		}
	}
	std::pair<std::size_t, Element *> GetLongestTrailingSequence() noexcept {
		if(x_uBegin <= x_uEnd){
			return std::make_pair(x_uEnd - x_uBegin, x_pStorage + x_uEnd);
		} else {
			return std::make_pair(x_uEnd, x_pStorage + x_uEnd);
		}
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
	void ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		Append(uDeltaSize, vParams...);
	}

	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity <= uOldCapacity){
			return;
		}

		auto uElementsToAlloc = uOldCapacity + 1;
		uElementsToAlloc += uElementsToAlloc >> 1;
		uElementsToAlloc = (uElementsToAlloc + 0x0F) & (std::size_t)-0x10;
		if(uElementsToAlloc < uNewCapacity + 1){
			uElementsToAlloc = uNewCapacity + 1;
		}
		const auto uBytesToAlloc = Impl_CheckedSizeArithmetic::Mul(sizeof(Element), uElementsToAlloc);
		const auto pNewStorage = static_cast<Element *>(Allocator()(uBytesToAlloc));
		const auto pOldStorage = x_pStorage;
		auto pWrite = pNewStorage;
		try {
			X_IterateForward(x_uBegin, x_uEnd,
				[&, this](auto uIndex){
					Construct(pWrite, std::move_if_noexcept(pOldStorage[uIndex]));
					++pWrite;
				});
		} catch(...){
			while(pWrite != pNewStorage){
				--pWrite;
				Destruct(pWrite);
			}
			Allocator()(static_cast<void *>(pNewStorage));
			throw;
		}
		X_IterateForward(x_uBegin, x_uEnd,
			[&, this](auto uIndex){
				Destruct(pOldStorage + uIndex);
			});
		Allocator()(static_cast<void *>(pOldStorage));

		x_pStorage  = pNewStorage;
		x_uBegin    = 0;
		x_uEnd      = static_cast<std::size_t>(pWrite - pNewStorage);
		x_uCircularCap  = uElementsToAlloc;
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = GetSize();
		const auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaCapacity, uOldSize);
		Reserve(uNewCapacity);
	}

	template<typename ...ParamsT>
	Element &Unshift(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedUnshift(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedUnshift(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		MCF_DEBUG_CHECK(GetCapacity() - GetSize() > 0);

		const auto uNewBegin = X_Retreat(x_uBegin, 1);
		const auto pElem = x_pStorage + uNewBegin;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		x_uBegin = uNewBegin;

		return *pElem;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= GetSize());

		const auto uNewBegin = X_Advance(x_uBegin, uCount);
		X_IterateForward(x_uBegin, uNewBegin,
			[&, this](auto uIndex){
				const auto pStorage = this->x_pStorage;
				Destruct(pStorage + uIndex);
			});
		x_uBegin = uNewBegin;
	}

	template<typename ...ParamsT>
	Element &Push(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		MCF_DEBUG_CHECK(GetCapacity() - GetSize() > 0);

		const auto uNewEnd = X_Advance(x_uEnd, 1);
		const auto pElem = x_pStorage + x_uEnd;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		x_uEnd = uNewEnd;

		return *pElem;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= GetSize());

		const auto uNewEnd = X_Retreat(x_uEnd, uCount);
		X_IterateBackward(uNewEnd, x_uEnd,
			[&, this](auto uIndex){
				const auto pStorage = this->x_pStorage;
				Destruct(pStorage + uIndex);
			});
		x_uEnd = uNewEnd;
	}

	template<typename ...ParamsT>
	void Prepend(std::size_t uDeltaSize, const ParamsT &...vParams){
		ReserveMore(uDeltaSize);

		std::size_t uElementsUnshifted = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				UncheckedUnshift(vParams...);
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Prepend(std::size_t uDeltaSize, IteratorT itEnd){
		ReserveMore(uDeltaSize);

		std::size_t uElementsUnshifted = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				--itEnd;
				UncheckedUnshift(*itEnd);
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Prepend(std::common_type_t<IteratorT> itBegin, IteratorT itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		if(kHasDeltaSizeHint){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			ReserveMore(uDeltaSize);
		}

		std::size_t uElementsUnshifted = 0;
		try {
			while(itEnd != itBegin){
				--itEnd;
				if(kHasDeltaSizeHint){
					UncheckedUnshift(*itEnd);
				} else {
					Unshift(*itEnd);
				}
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	void Prepend(std::initializer_list<Element> ilElements){
		Prepend(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	void UncheckedPrepend(std::size_t uDeltaSize, const ParamsT &...vParams){
		std::size_t uElementsUnshifted = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				UncheckedUnshift(vParams...);
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void UncheckedPrepend(std::size_t uDeltaSize, IteratorT itEnd){
		std::size_t uElementsUnshifted = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				--itEnd;
				UncheckedUnshift(*itEnd);
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void UncheckedPrepend(std::common_type_t<IteratorT> itBegin, IteratorT itEnd){
		std::size_t uElementsUnshifted = 0;
		try {
			while(itEnd != itBegin){
				--itEnd;
				UncheckedUnshift(*itEnd);
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	void UncheckedPrepend(std::initializer_list<Element> ilElements){
		UncheckedPrepend(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	void Append(std::size_t uDeltaSize, const ParamsT &...vParams){
		ReserveMore(uDeltaSize);

		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				UncheckedPush(vParams...);
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Append(IteratorT itBegin, std::size_t uDeltaSize){
		ReserveMore(uDeltaSize);

		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				UncheckedPush(*itBegin);
				++itBegin;
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		if(kHasDeltaSizeHint){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			ReserveMore(uDeltaSize);
		}

		std::size_t uElementsPushed = 0;
		try {
			if(kHasDeltaSizeHint){
				while(itBegin != itEnd){
					UncheckedPush(*itBegin);
					++itBegin;
					++uElementsPushed;
				}
			} else {
				while(itBegin != itEnd){
					Push(*itBegin);
					++itBegin;
					++uElementsPushed;
				}
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	void Append(std::initializer_list<Element> ilElements){
		Append(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	void UncheckedAppend(std::size_t uDeltaSize, const ParamsT &...vParams){
		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				UncheckedPush(vParams...);
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void UncheckedAppend(IteratorT itBegin, std::size_t uDeltaSize){
		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
				UncheckedPush(*itBegin);
				++itBegin;
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void UncheckedAppend(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		std::size_t uElementsPushed = 0;
		try {
			while(itBegin != itEnd){
				UncheckedPush(*itBegin);
				++itBegin;
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	void UncheckedAppend(std::initializer_list<Element> ilElements){
		UncheckedAppend(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	Element *Emplace(const Element *pPos, ParamsT &&...vParams){
		std::size_t uOffset;
		if(pPos){
			uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		} else {
			uOffset = x_uEnd;
		}

		const auto uCountBefore = X_Measure(x_uBegin, uOffset);

		if(std::is_nothrow_move_constructible<Element>::value){
			const auto vPrepared = X_PrepareForInsertion(uOffset, 1);
			auto uWrite = vPrepared.first;
			try {
				DefaultConstruct(x_pStorage + uWrite, std::forward<ParamsT>(vParams)...);
			} catch(...){
				X_UndoPreparation(vPrepared, 1);
				throw;
			}
			if(vPrepared.second){
				x_uEnd = X_Advance(x_uEnd, 1);
			} else {
				x_uBegin = X_Retreat(x_uBegin, 1);
			}
		} else {
			const auto uSize = GetSize();
			auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(1, uSize);
			const auto uCapacity = GetCapacity();
			if(uNewCapacity < uCapacity){
				uNewCapacity = uCapacity;
			}
			CircularQueue queTemp;
			queTemp.Reserve(uNewCapacity);
			X_IterateForward(x_uBegin, uOffset,
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					queTemp.UncheckedPush(pStorage[uIndex]);
				});
			queTemp.UncheckedPush(vParams...);
			X_IterateForward(uOffset, x_uEnd,
				[&, this](auto uIndex){
					const auto pStorage = this->x_pStorage;
					queTemp.UncheckedPush(pStorage[uIndex]);
				});
			*this = std::move(queTemp);
		}

		return x_pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		std::size_t uOffset;
		if(pPos){
			uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		} else {
			uOffset = x_uEnd;
		}

		const auto uCountBefore = X_Measure(x_uBegin, uOffset);

		if(uDeltaSize != 0){
			if(std::is_nothrow_move_constructible<Element>::value){
				const auto vPrepared = X_PrepareForInsertion(uOffset, uDeltaSize);
				auto uWrite = vPrepared.first;
				try {
					for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
						DefaultConstruct(x_pStorage + uWrite, vParams...);
						uWrite = X_Advance(uWrite, 1);
					}
				} catch(...){
					while(uWrite != vPrepared.first){
						uWrite = X_Retreat(uWrite, 1);
						Destruct(x_pStorage + uWrite);
					}
					X_UndoPreparation(vPrepared, uDeltaSize);
					throw;
				}
				if(vPrepared.second){
					x_uEnd = X_Advance(x_uEnd, uDeltaSize);
				} else {
					x_uBegin = X_Retreat(x_uBegin, uDeltaSize);
				}
			} else {
				const auto uSize = GetSize();
				auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaSize, uSize);
				const auto uCapacity = GetCapacity();
				if(uNewCapacity < uCapacity){
					uNewCapacity = uCapacity;
				}
				CircularQueue queTemp;
				queTemp.Reserve(uNewCapacity);
				X_IterateForward(x_uBegin, uOffset,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						queTemp.UncheckedPush(pStorage[uIndex]);
					});
				for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
					queTemp.UncheckedPush(vParams...);
				}
				X_IterateForward(uOffset, x_uEnd,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						queTemp.UncheckedPush(pStorage[uIndex]);
					});
				*this = std::move(queTemp);
			}
		}

		return x_pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	Element *Insert(const Element *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		std::size_t uOffset;
		if(pPos){
			uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		} else {
			uOffset = x_uEnd;
		}

		const auto uCountBefore = X_Measure(x_uBegin, uOffset);

		if(itBegin != itEnd){
			if(std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value){
				const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
				if(std::is_nothrow_move_constructible<Element>::value){
					const auto vPrepared = X_PrepareForInsertion(uOffset, uDeltaSize);
					auto uWrite = vPrepared.first;
					try {
						for(auto itCur = itBegin; itCur != itEnd; ++itCur){
							DefaultConstruct(x_pStorage + uWrite, *itCur);
							uWrite = X_Advance(uWrite, 1);
						}
					} catch(...){
						while(uWrite != vPrepared.first){
							uWrite = X_Retreat(uWrite, 1);
							Destruct(x_pStorage + uWrite);
						}
						X_UndoPreparation(vPrepared, uDeltaSize);
						throw;
					}
					if(vPrepared.second){
						x_uEnd = X_Advance(x_uEnd, uDeltaSize);
					} else {
						x_uBegin = X_Retreat(x_uBegin, uDeltaSize);
					}
				} else {
					const auto uSize = GetSize();
					auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaSize, uSize);
					const auto uCapacity = GetCapacity();
					if(uNewCapacity < uCapacity){
						uNewCapacity = uCapacity;
					}
					CircularQueue queTemp;
					queTemp.Reserve(uNewCapacity);
					X_IterateForward(x_uBegin, uOffset,
						[&, this](auto uIndex){
							const auto pStorage = this->x_pStorage;
							queTemp.UncheckedPush(pStorage[uIndex]);
						});
					for(auto itCur = itBegin; itCur != itEnd; ++itCur){
						queTemp.UncheckedPush(*itCur);
					}
					X_IterateForward(uOffset, x_uEnd,
						[&, this](auto uIndex){
							const auto pStorage = this->x_pStorage;
							queTemp.UncheckedPush(pStorage[uIndex]);
						});
					*this = std::move(queTemp);
				}
			} else {
				CircularQueue queTemp;
				const auto uCapacity = GetCapacity();
				queTemp.Reserve(uCapacity);
				X_IterateForward(x_uBegin, uOffset,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						queTemp.UncheckedPush(pStorage[uIndex]);
					});
				for(auto itCur = itBegin; itCur != itEnd; ++itCur){
					queTemp.Push(*itCur);
				}
				X_IterateForward(uOffset, x_uEnd,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						queTemp.Push(pStorage[uIndex]);
					});
				*this = std::move(queTemp);
			}
		}

		return x_pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	Element *Insert(const Element *pPos, std::initializer_list<Element> ilElements){
		return Insert(pPos, ilElements.begin(), ilElements.end());
	}

	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(std::is_nothrow_move_constructible<Element>::value) {
		std::size_t uOffsetBegin, uOffsetEnd;
		if(pBegin){
			uOffsetBegin = static_cast<std::size_t>(pBegin - x_pStorage);
		} else {
			uOffsetBegin = x_uEnd;
		}
		if(pEnd){
			uOffsetEnd = static_cast<std::size_t>(pEnd - x_pStorage);
		} else {
			uOffsetEnd = x_uEnd;
		}

		if(uOffsetBegin != uOffsetEnd){
			if(uOffsetEnd == x_uEnd){
				const auto uDeltaSize = X_Measure(uOffsetBegin, uOffsetEnd);
				Pop(uDeltaSize);
			} else if(x_uBegin == uOffsetBegin){
				const auto uDeltaSize = X_Measure(uOffsetBegin, uOffsetEnd);
				Shift(uDeltaSize);
			} else if(std::is_nothrow_move_constructible<Element>::value){
				const auto uDeltaSize = X_Measure(uOffsetBegin, uOffsetEnd);
				X_IterateForward(uOffsetBegin, uOffsetEnd,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						Destruct(pStorage + uIndex);
					});
				X_IterateForward(uOffsetEnd, x_uEnd,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						Construct(pStorage + X_Retreat(uIndex, uDeltaSize), std::move(*(pStorage + uIndex)));
						Destruct(pStorage + uIndex);
					});
				x_uEnd = X_Retreat(x_uEnd, uDeltaSize);
			} else {
				CircularQueue queTemp;
				const auto uCapacity = GetCapacity();
				queTemp.Reserve(uCapacity);
				X_IterateForward(x_uBegin, uOffsetBegin,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						queTemp.UncheckedPush(pStorage[uIndex]);
					});
				X_IterateForward(uOffsetEnd, x_uEnd,
					[&, this](auto uIndex){
						const auto pStorage = this->x_pStorage;
						queTemp.UncheckedPush(pStorage[uIndex]);
					});
				*this = std::move(queTemp);
			}
		}

		return x_pStorage + uOffsetBegin;
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(std::declval<CircularQueue &>().Erase(pPos, pPos))) {
		MCF_DEBUG_CHECK(pPos);

		return Erase(pPos, GetNext(pPos));
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

public:
	friend void swap(CircularQueue &vSelf, CircularQueue &vOther) noexcept {
		vSelf.Swap(vOther);
	}

	friend decltype(auto) begin(const CircularQueue &vOther) noexcept {
		return vOther.EnumerateFirst();
	}
	friend decltype(auto) begin(CircularQueue &vOther) noexcept {
		return vOther.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const CircularQueue &vOther) noexcept {
		return begin(vOther);
	}
	friend decltype(auto) end(const CircularQueue &vOther) noexcept {
		return vOther.EnumerateSingular();
	}
	friend decltype(auto) end(CircularQueue &vOther) noexcept {
		return vOther.EnumerateSingular();
	}
	friend decltype(auto) cend(const CircularQueue &vOther) noexcept {
		return end(vOther);
	}
};

}

#endif
