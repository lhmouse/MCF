// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_RING_QUEUE_HPP_
#define MCF_CONTAINERS_RING_QUEUE_HPP_

#include "_Enumerator.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/DeclVal.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <new>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class RingQueue {
public:
	// 容器需求。
	using Element         = ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <RingQueue>;
	using Enumerator      = Impl_Enumerator::Enumerator      <RingQueue>;

private:
	Element *x_pStorage;
	std::size_t x_uBegin;
	std::size_t x_uEnd;
	std::size_t x_uRingCap;

public:
	constexpr RingQueue() noexcept
		: x_pStorage(nullptr), x_uBegin(0), x_uEnd(0), x_uRingCap(1)
	{
	}
	template<typename ...ParamsT>
	explicit RingQueue(std::size_t uSize, const ParamsT &...vParams)
		: RingQueue()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	RingQueue(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: RingQueue()
	{
		Append(itBegin, itEnd);
	}
	RingQueue(std::initializer_list<Element> rhs)
		: RingQueue(rhs.begin(), rhs.end())
	{
	}
	RingQueue(const RingQueue &rhs)
		: RingQueue()
	{
		Reserve(rhs.GetSize());
		rhs.X_IterateForward(rhs.x_uBegin, rhs.x_uEnd,
			[&, this](auto i){
				UncheckedPush(rhs.x_pStorage[i]);
			});
	}
	RingQueue(RingQueue &&rhs) noexcept
		: RingQueue()
	{
		rhs.Swap(*this);
	}
	RingQueue &operator=(const RingQueue &rhs){
		RingQueue(rhs).Swap(*this);
		return *this;
	}
	RingQueue &operator=(RingQueue &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~RingQueue(){
		Clear();
		::operator delete[](x_pStorage);
	}

private:
	template<typename CallbackT>
	void X_IterateForward(std::size_t uBegin, std::size_t uEnd, CallbackT &&vCallback) const {
		if(uBegin <= uEnd){
			for(std::size_t i = uBegin; i != uEnd; ++i){
				vCallback(i);
			}
		} else {
			for(std::size_t i = uBegin; i != x_uRingCap; ++i){
				vCallback(i);
			}
			for(std::size_t i = 0; i != uEnd; ++i){
				vCallback(i);
			}
		}
	}
	template<typename CallbackT>
	void X_IterateBackward(std::size_t uBegin, std::size_t uEnd, CallbackT &&vCallback) const {
		if(uBegin <= uEnd){
			for(std::size_t i = uEnd; i != uBegin; --i){
				vCallback(i - 1);
			}
		} else {
			for(std::size_t i = uEnd; i != 0; --i){
				vCallback(i - 1);
			}
			for(std::size_t i = x_uRingCap; i != uBegin; --i){
				vCallback(i - 1);
			}
		}
	}

	std::size_t X_Retreat(std::size_t uIndex, std::size_t uDelta) const noexcept {
		ASSERT(uIndex < x_uRingCap);
		ASSERT(uDelta < x_uRingCap);

		auto uNewIndex = uIndex - uDelta;
		if(uIndex < uDelta){
			uNewIndex += x_uRingCap;
		}
		return uNewIndex;
	}
	std::size_t X_Advance(std::size_t uIndex, std::size_t uDelta) const noexcept {
		ASSERT(uIndex < x_uRingCap);
		ASSERT(uDelta < x_uRingCap);

		auto uNewIndex = uIndex + uDelta;
		if(x_uRingCap - uIndex <= uDelta){
			uNewIndex -= x_uRingCap;
		}
		return uNewIndex;
	}

	std::pair<std::size_t, bool> X_PrepareForInsertion(std::size_t uPos, std::size_t uDeltaSize){
		ASSERT(std::is_nothrow_move_constructible<Element>::value);
		ASSERT(!IsEmpty());
		ASSERT(X_Advance(x_uBegin, uPos) <= GetSize());

		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		const auto uCountAfter = X_Retreat(x_uEnd, uPos);

		ReserveMore(uDeltaSize);
		uPos = X_Advance(x_uBegin, uCountBefore);

		if(uCountBefore >= uCountAfter){
			const auto uReadBegin = uPos;
			const auto uReadEnd = x_uEnd;
			auto uWriteEnd = X_Advance(uReadEnd, uDeltaSize);
			X_IterateBackward(uReadBegin, uReadEnd,
				[&, this](auto i){
					uWriteEnd = X_Retreat(uWriteEnd, 1);
					Construct(x_pStorage + uWriteEnd, std::move(x_pStorage[i]));
					Destruct(x_pStorage + i);
				});
			return std::make_pair(uPos, true);
		} else {
			const auto uReadBegin = x_uBegin;
			const auto uReadEnd = uPos;
			auto uWrite = X_Retreat(uReadBegin, uDeltaSize);
			X_IterateForward(uReadBegin, uReadEnd,
				[&, this](auto i){
					Construct(x_pStorage + uWrite, std::move(x_pStorage[i]));
					Destruct(x_pStorage + i);
					uWrite = X_Advance(uWrite, 1);
				});
			return std::make_pair(X_Retreat(uPos, uDeltaSize), false);
		}
	}
	void X_UndoPreparation(const std::pair<std::size_t, bool> &vPrepared, std::size_t uDeltaSize) noexcept {
		ASSERT(std::is_nothrow_move_constructible<Element>::value);
		ASSERT(!IsEmpty());
		ASSERT(uDeltaSize <= GetCapacity() - GetSize());
		ASSERT(X_Advance(x_uBegin, vPrepared.second ? vPrepared.first : X_Advance(vPrepared.first, uDeltaSize)) <= GetSize());

		if(vPrepared.second){
			const auto uWriteBegin = vPrepared.first;
			const auto uWriteEnd = x_uEnd;
			auto uRead = X_Advance(uWriteBegin, uDeltaSize);
			X_IterateForward(uWriteBegin, uWriteEnd,
				[&, this](auto i){
					Construct(x_pStorage + i, std::move(x_pStorage[uRead]));
					Destruct(x_pStorage + uRead);
					uRead = X_Advance(uRead, 1);
				});
		} else {
			const auto uWriteBegin = x_uBegin;
			const auto uWriteEnd = X_Advance(vPrepared.first, uDeltaSize);
			auto uReadEnd = vPrepared.first;
			X_IterateBackward(uWriteBegin, uWriteEnd,
				[&, this](auto i){
					uReadEnd = X_Retreat(uReadEnd, 1);
					Construct(x_pStorage + i, std::move(x_pStorage[uReadEnd]));
					Destruct(x_pStorage + uReadEnd);
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
				[&, this](auto i){
					*itOutput = std::move(x_pStorage[i]);
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
		ASSERT(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		if(uOffset == x_uBegin){
			return nullptr;
		}
		uOffset = X_Retreat(uOffset, 1);
		return x_pStorage + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		ASSERT(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		if(uOffset == x_uBegin){
			return nullptr;
		}
		uOffset = X_Retreat(uOffset, 1);
		return x_pStorage + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		ASSERT(pPos);

		auto uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		uOffset = X_Advance(uOffset, 1);
		if(uOffset == x_uEnd){
			return nullptr;
		}
		return x_pStorage + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		ASSERT(pPos);

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

	void Swap(RingQueue &rhs) noexcept {
		using std::swap;
		swap(x_pStorage, rhs.x_pStorage);
		swap(x_uBegin,   rhs.x_uBegin);
		swap(x_uEnd,     rhs.x_uEnd);
		swap(x_uRingCap, rhs.x_uRingCap);
	}

	// RingQueue 需求。
	std::size_t GetSize() const noexcept {
		if(x_uBegin <= x_uEnd){
			return x_uEnd - x_uBegin;
		} else {
			return x_uEnd + x_uRingCap - x_uBegin;
		}
	}
	std::size_t GetCapacity() const noexcept {
		return x_uRingCap - 1;
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "RingQueue: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "RingQueue: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		return x_pStorage[X_Advance(x_uBegin, uIndex)];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		return x_pStorage[X_Advance(x_uBegin, uIndex)];
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
		const auto uOldSize = GetSize();
		const auto uNewSize = uOldSize + uDeltaSize;
		if(uNewSize < uOldSize){
			throw std::bad_array_new_length();
		}
		Append(uDeltaSize - uOldSize, vParams...);
	}

	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity <= uOldCapacity){
			return;
		}

		if(uNewCapacity + 1 < uNewCapacity){
			throw std::bad_array_new_length();
		}

		auto uElementsToAlloc = uOldCapacity + 1;
		uElementsToAlloc += uElementsToAlloc >> 1;
		uElementsToAlloc = (uElementsToAlloc + 0x0F) & (std::size_t)-0x10;
		if(uElementsToAlloc < uNewCapacity + 1){
			uElementsToAlloc = uNewCapacity + 1;
		}
		const auto uBytesToAlloc = sizeof(Element) * uElementsToAlloc;
		if(uBytesToAlloc / sizeof(Element) != uElementsToAlloc){
			throw std::bad_array_new_length();
		}

		const auto pNewStorage = static_cast<Element *>(::operator new[](uBytesToAlloc));
		const auto pOldStorage = x_pStorage;
		auto pWrite = pNewStorage;
		try {
			X_IterateForward(x_uBegin, x_uEnd,
				[&, this](auto i){
					Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
					++pWrite;
				});
		} catch(...){
			while(pWrite != pNewStorage){
				--pWrite;
				Destruct(pWrite);
			}
			::operator delete[](pNewStorage);
			throw;
		}
		X_IterateForward(x_uBegin, x_uEnd,
			[&, this](auto i){
				Destruct(pOldStorage + i);
			});
		::operator delete[](pOldStorage);

		x_pStorage  = pNewStorage;
		x_uBegin    = 0;
		x_uEnd      = static_cast<std::size_t>(pWrite - pNewStorage);
		x_uRingCap  = uElementsToAlloc;
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
	Element &Unshift(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedUnshift(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedUnshift(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto uNewBegin = X_Retreat(x_uBegin, 1);
		const auto pElem = x_pStorage + uNewBegin;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		x_uBegin = uNewBegin;

		return *pElem;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		const auto uNewBegin = X_Advance(x_uBegin, uCount);
		X_IterateForward(x_uBegin, uNewBegin,
			[&, this](auto i){
				Destruct(x_pStorage + i);
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
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto uNewEnd = X_Advance(x_uEnd, 1);
		const auto pElem = x_pStorage + x_uEnd;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		x_uEnd = uNewEnd;

		return *pElem;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		const auto uNewEnd = X_Retreat(x_uEnd, uCount);
		X_IterateBackward(uNewEnd, x_uEnd,
			[&, this](auto i){
				Destruct(x_pStorage + i);
			});
		x_uEnd = uNewEnd;
	}

	template<typename ...ParamsT>
	void Prepend(std::size_t uDeltaSize, const ParamsT &...vParams){
		ReserveMore(uDeltaSize);

		std::size_t uElementsUnshifted = 0;
		try {
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				UncheckedUnshift(vParams...);
				++uElementsUnshifted;
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Prepend(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		if(kHasDeltaSizeHint){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			ReserveMore(uDeltaSize);
		}

		std::size_t uElementsUnshifted = 0;
		try {
			if(kHasDeltaSizeHint){
				auto it = itEnd;
				while(it != itBegin){
					--it;
					UncheckedUnshift(*it);
					++uElementsUnshifted;
				}
			} else {
				auto it = itEnd;
				while(it != itBegin){
					--it;
					Unshift(*it);
					++uElementsUnshifted;
				}
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
			if(kHasDeltaSizeHint){
				auto it = itBegin;
				while(it != itEnd){
					UncheckedPush(*it);
					++it;
					++uElementsPushed;
				}
			} else {
				auto it = itBegin;
				while(it != itEnd){
					Push(*it);
					++it;
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
	Element *Emplace(const Element *pPos, ParamsT &&...vParams){
		if(!pPos){
			Push(std::forward<ParamsT>(vParams)...);
			return nullptr;
		}

		const auto uPos = static_cast<std::size_t>(pPos - x_pStorage);
		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		if(std::is_nothrow_move_constructible<Element>::value){
			const auto vPrepared = X_PrepareForInsertion(uPos, 1);
			try {
				DefaultConstruct(x_pStorage + vPrepared.first, vParams...);
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
			auto uNewCapacity = GetSize() + 1;
			if(uNewCapacity < GetSize()){
				throw std::bad_array_new_length();
			}
			if(uNewCapacity < GetCapacity()){
				uNewCapacity = GetCapacity();
			}
			RingQueue queTemp;
			queTemp.Reserve(uNewCapacity);
			X_IterateForward(x_uBegin, uPos,
				[&, this](auto i){
					queTemp.UncheckedPush(x_pStorage[i]);
				});
			queTemp.UncheckedPush(vParams...);
			X_IterateForward(uPos, x_uEnd,
				[&, this](auto i){
					queTemp.UncheckedPush(x_pStorage[i]);
				});
			*this = std::move(queTemp);
		}

		return x_pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		if(!pPos){
			Append(uDeltaSize, vParams...);
			return nullptr;
		}

		const auto uPos = static_cast<std::size_t>(pPos - x_pStorage);
		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		if(std::is_nothrow_move_constructible<Element>::value){
			const auto vPrepared = X_PrepareForInsertion(uPos, uDeltaSize);
			auto uWrite = vPrepared.first;
			try {
				for(std::size_t i = 0; i < uDeltaSize; ++i){
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
			auto uNewCapacity = GetSize() + uDeltaSize;
			if(uNewCapacity < GetSize()){
				throw std::bad_array_new_length();
			}
			if(uNewCapacity < GetCapacity()){
				uNewCapacity = GetCapacity();
			}
			RingQueue queTemp;
			queTemp.Reserve(uNewCapacity);
			X_IterateForward(x_uBegin, uPos,
				[&, this](auto i){
					queTemp.UncheckedPush(x_pStorage[i]);
				});
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				queTemp.UncheckedPush(vParams...);
			}
			X_IterateForward(uPos, x_uEnd,
				[&, this](auto i){
					queTemp.UncheckedPush(x_pStorage[i]);
				});
			*this = std::move(queTemp);
		}

		return x_pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	Element *Insert(const Element *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		if(!pPos){
			Append(itBegin, itEnd);
			return nullptr;
		}

		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		const auto uPos = static_cast<std::size_t>(pPos - x_pStorage);

		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		if(kHasDeltaSizeHint && std::is_nothrow_move_constructible<Element>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));

			const auto vPrepared = X_PrepareForInsertion(uPos, uDeltaSize);
			auto uWrite = vPrepared.first;
			try {
				for(auto it = itBegin; it != itEnd; ++it){
					Construct(x_pStorage + uWrite, *it);
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
			if(kHasDeltaSizeHint){
				const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
				auto uNewCapacity = GetSize() + uDeltaSize;
				if(uNewCapacity < GetSize()){
					throw std::bad_array_new_length();
				}
				if(uNewCapacity < GetCapacity()){
					uNewCapacity = GetCapacity();
				}
				RingQueue queTemp;
				queTemp.Reserve(uNewCapacity);
				X_IterateForward(x_uBegin, uPos,
					[&, this](auto i){
						queTemp.UncheckedPush(x_pStorage[i]);
					});
				for(auto it = itBegin; it != itEnd; ++it){
					queTemp.UncheckedPush(*it);
				}
				X_IterateForward(uPos, x_uEnd,
					[&, this](auto i){
						queTemp.UncheckedPush(x_pStorage[i]);
					});
				*this = std::move(queTemp);
			} else {
				RingQueue queTemp;
				queTemp.Reserve(GetCapacity());
				X_IterateForward(x_uBegin, uPos,
					[&, this](auto i){
						queTemp.UncheckedPush(x_pStorage[i]);
					});
				for(auto it = itBegin; it != itEnd; ++it){
					queTemp.Push(*it);
				}
				X_IterateForward(uPos, x_uEnd,
					[&, this](auto i){
						queTemp.Push(x_pStorage[i]);
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
		if(pBegin == pEnd){
			return const_cast<Element *>(pEnd);
		}
		ASSERT(pBegin);

		const auto uBegin = static_cast<std::size_t>(pBegin - x_pStorage);
		if(!pEnd){
			const auto uDeltaSize = X_Retreat(x_uEnd, uBegin);

			Pop(uDeltaSize);
			return nullptr;
		}
		const auto uEnd = static_cast<std::size_t>(pEnd - x_pStorage);

		const auto uCountBefore = X_Retreat(uBegin, x_uBegin);
		if(std::is_nothrow_move_constructible<Element>::value){
			const auto uCountAfter = X_Retreat(x_uEnd, uEnd);
			if(uCountBefore >= uCountAfter){
				X_IterateForward(uBegin, uEnd,
					[&, this](auto i){
						Destruct(x_pStorage + i);
					});

				auto uNewEnd = uBegin;
				X_IterateForward(uEnd, x_uEnd,
					[&, this](auto i){
						Construct(x_pStorage + uNewEnd, std::move(x_pStorage[i]));
						uNewEnd = X_Advance(uNewEnd, 1);
						Destruct(x_pStorage + i);
					});
				x_uEnd = uNewEnd;
			} else {
				X_IterateBackward(uBegin, uEnd,
					[&, this](auto i){
						Destruct(x_pStorage + i);
					});

				auto uNewBegin = uEnd;
				X_IterateBackward(x_uBegin, uBegin,
					[&, this](auto i){
						uNewBegin = X_Retreat(uNewBegin, 1);
						Construct(x_pStorage + uNewBegin, std::move(x_pStorage[i]));
						Destruct(x_pStorage + i);
					});
				x_uBegin = uNewBegin;
			}
		} else {
			RingQueue queTemp;
			queTemp.Reserve(GetCapacity());
			X_IterateForward(x_uBegin, uBegin,
				[&, this](auto i){
					queTemp.UncheckedPush(x_pStorage[i]);
				});
			X_IterateForward(uEnd, x_uEnd,
				[&, this](auto i){
					queTemp.UncheckedPush(x_pStorage[i]);
				});
			*this = std::move(queTemp);
		}

		return x_pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(DeclVal<RingQueue &>().Erase(pPos, pPos))) {
		ASSERT(pPos);

		return Erase(pPos, GetNext(pPos));
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

	friend void swap(RingQueue &lhs, RingQueue &rhs) noexcept {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const RingQueue &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) begin(RingQueue &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const RingQueue &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const RingQueue &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) end(RingQueue &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) cend(const RingQueue &rhs) noexcept {
		return end(rhs);
	}
};

}

#endif
