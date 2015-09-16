// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_RING_QUEUE_HPP_
#define MCF_CONTAINERS_RING_QUEUE_HPP_

#include "_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <new>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class RingQueue {
private:
	void *x_pStorage;
	std::size_t x_uBegin;
	std::size_t x_uEnd;
	std::size_t x_uRingCap;

public:
	constexpr RingQueue() noexcept
		: x_pStorage(nullptr), x_uBegin(1), x_uEnd(0), x_uRingCap(1)
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
	RingQueue(std::initializer_list<ElementT> rhs)
		: RingQueue(rhs.begin(), rhs.end())
	{
	}
	RingQueue(const RingQueue &rhs)
		: RingQueue()
	{
		const auto uSize = rhs.GetSize();
		if(uSize != 0){
			Reserve(uSize);
			X_UncheckedUnsafeCopyFrom(rhs, rhs.x_uBegin, rhs.x_uEnd);
		}
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
	const ElementT *X_GetStorage() const noexcept {
		ASSERT(!IsEmpty());

		return static_cast<const ElementT *>(x_pStorage);
	}
	ElementT *X_GetStorage() noexcept {
		ASSERT(!IsEmpty());

		return static_cast<ElementT *>(x_pStorage);
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
		ASSERT(std::is_nothrow_move_constructible<ElementT>::value);
		ASSERT(!IsEmpty());
		ASSERT(X_Advance(x_uBegin, uPos) <= GetSize());

		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		const auto uCountAfter = X_Retreat(x_uEnd, uPos);

		ReserveMore(uDeltaSize);
		const auto pStorage = X_GetStorage();
		uPos = X_Advance(x_uBegin, uCountBefore);

		if(0 &&uCountBefore >= uCountAfter){
			const auto uReadBegin = uPos;
			const auto uReadEnd = x_uEnd;
			auto uWriteEnd = X_Advance(uReadEnd, uDeltaSize);
			if(uReadBegin <= uReadEnd){
				for(std::size_t i = uReadEnd; i != uReadBegin; --i){
					uWriteEnd = X_Retreat(uWriteEnd, 1);
					Construct(pStorage + uWriteEnd, std::move(pStorage[i - 1]));
					Destruct(pStorage + i - 1);
				}
			} else {
				for(std::size_t i = uReadEnd; i != 0; --i){
					uWriteEnd = X_Retreat(uWriteEnd, 1);
					Construct(pStorage + uWriteEnd, std::move(pStorage[i - 1]));
					Destruct(pStorage + i - 1);
				}
				for(std::size_t i = x_uRingCap; i != uReadBegin; --i){
					uWriteEnd = X_Retreat(uWriteEnd, 1);
					Construct(pStorage + uWriteEnd, std::move(pStorage[i - 1]));
					Destruct(pStorage + i - 1);
				}
			}
			return std::make_pair(uPos, true);
		} else {
			const auto uReadBegin = x_uBegin;
			const auto uReadEnd = uPos;
			auto uWrite = X_Retreat(uReadBegin, uDeltaSize);
			if(uReadBegin <= uReadEnd){
				for(std::size_t i = uReadBegin; i != uReadEnd; ++i){
					Construct(pStorage + uWrite, std::move(pStorage[i]));
					Destruct(pStorage + i);
					uWrite = X_Advance(uWrite, 1);
				}
			} else {
				for(std::size_t i = uReadBegin; i != x_uRingCap; ++i){
					Construct(pStorage + uWrite, std::move(pStorage[i]));
					Destruct(pStorage + i);
					uWrite = X_Advance(uWrite, 1);
				}
				for(std::size_t i = 0; i != uReadEnd; ++i){
					Construct(pStorage + uWrite, std::move(pStorage[i]));
					Destruct(pStorage + i);
					uWrite = X_Advance(uWrite, 1);
				}
			}
			return std::make_pair(X_Retreat(uPos, uDeltaSize), false);
		}
	}
	void X_UndoPreparation(const std::pair<std::size_t, bool> &vPrepared, std::size_t uDeltaSize) noexcept {
		ASSERT(std::is_nothrow_move_constructible<ElementT>::value);
		ASSERT(!IsEmpty());
		ASSERT(uDeltaSize <= GetCapacity() - GetSize());
		ASSERT(X_Advance(x_uBegin, vPrepared.second ? vPrepared.first : X_Advance(vPrepared.first, uDeltaSize)) <= GetSize());

		const auto pStorage = X_GetStorage();

		if(vPrepared.second){
			const auto uWriteBegin = vPrepared.first;
			const auto uWriteEnd = x_uEnd;
			auto uRead = X_Advance(uWriteBegin, uDeltaSize);
			if(uWriteBegin <= uWriteEnd){
				for(std::size_t i = uWriteBegin; i != uWriteEnd; ++i){
					Construct(pStorage + i, std::move(pStorage[uRead]));
					Destruct(pStorage + uRead);
					uRead = X_Advance(uRead, 1);
				}
			} else {
				for(std::size_t i = uWriteBegin; i != x_uRingCap; ++i){
					Construct(pStorage + i, std::move(pStorage[uRead]));
					Destruct(pStorage + uRead);
					uRead = X_Advance(uRead, 1);
				}
				for(std::size_t i = 0; i != uWriteEnd; ++i){
					Construct(pStorage + i, std::move(pStorage[uRead]));
					Destruct(pStorage + uRead);
					uRead = X_Advance(uRead, 1);
				}
			}
		} else {
			const auto uWriteBegin = x_uBegin;
			const auto uWriteEnd = X_Advance(vPrepared.first, uDeltaSize);
			auto uReadEnd = vPrepared.first;
			if(uWriteBegin <= uWriteEnd){
				for(std::size_t i = uWriteEnd; i != uWriteBegin; --i){
					uReadEnd = X_Retreat(uReadEnd, 1);
					Construct(pStorage + i - 1, std::move(pStorage[uReadEnd]));
					Destruct(pStorage + uReadEnd);
				}
			} else {
				for(std::size_t i = uWriteEnd; i != 0; --i){
					uReadEnd = X_Retreat(uReadEnd, 1);
					Construct(pStorage + i - 1, std::move(pStorage[uReadEnd]));
					Destruct(pStorage + uReadEnd);
				}
				for(std::size_t i = x_uRingCap; i != uWriteBegin; --i){
					uReadEnd = X_Retreat(uReadEnd, 1);
					Construct(pStorage + i - 1, std::move(pStorage[uReadEnd]));
					Destruct(pStorage + uReadEnd);
				}
			}
		}
	}

	// 基本异常安全保证。
	void X_UnsafeCopyFrom(const RingQueue &queFrom, std::size_t uBegin, std::size_t uEnd){
		ASSERT(&queFrom != this);

		const auto pFromStorage = queFrom.X_GetStorage();
		if(uBegin <= uEnd){
			for(std::size_t i = uBegin; i != uEnd; ++i){
				Push(pFromStorage[i]);
			}
		} else {
			for(std::size_t i = uBegin; i != queFrom.x_uRingCap; ++i){
				Push(pFromStorage[i]);
			}
			for(std::size_t i = 0; i != uEnd; ++i){
				Push(pFromStorage[i]);
			}
		}
	}
	void X_UncheckedUnsafeCopyFrom(const RingQueue &queFrom, std::size_t uBegin, std::size_t uEnd){
		ASSERT(&queFrom != this);

		const auto pFromStorage = queFrom.X_GetStorage();
		if(uBegin <= uEnd){
			for(std::size_t i = uBegin; i != uEnd; ++i){
				UncheckedPush(pFromStorage[i]);
			}
		} else {
			for(std::size_t i = uBegin; i != queFrom.x_uRingCap; ++i){
				UncheckedPush(pFromStorage[i]);
			}
			for(std::size_t i = 0; i != uEnd; ++i){
				UncheckedPush(pFromStorage[i]);
			}
		}
	}

public:
	// 容器需求。
	using Element         = ElementT;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <RingQueue>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <RingQueue>;

	bool IsEmpty() const noexcept {
		return x_uBegin == x_uEnd;
	}
	void Clear() noexcept {
		Shift(GetSize());
	}

	const Element *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + x_uBegin;
	}
	Element *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + x_uBegin;
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + X_Retreat(x_uEnd, 1);
	}
	Element *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + X_Retreat(x_uEnd, 1);
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		if(uOffset == x_uBegin){
			return nullptr;
		}
		uOffset = X_Retreat(uOffset, 1);
		return pStorage + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		if(uOffset == x_uBegin){
			return nullptr;
		}
		uOffset = X_Retreat(uOffset, 1);
		return pStorage + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		uOffset = X_Advance(uOffset, 1);
		if(uOffset == x_uEnd){
			return nullptr;
		}
		return pStorage + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		uOffset = X_Advance(uOffset, 1);
		if(uOffset == x_uEnd){
			return nullptr;
		}
		return pStorage + uOffset;
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
		std::swap(x_pStorage, rhs.x_pStorage);
		std::swap(x_uBegin,   rhs.x_uBegin);
		std::swap(x_uEnd,     rhs.x_uEnd);
		std::swap(x_uRingCap, rhs.x_uRingCap);
	}

	// RingQueue 需求。
	std::size_t GetSize() const noexcept {
		if(x_uBegin <= x_uEnd){
			return x_uEnd - x_uBegin;
		} else {
			return x_uEnd + x_uRingCap - x_uBegin;
		}
	}
	std::size_t GetCapacity() noexcept {
		return x_uRingCap - 1;
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		const auto pStorage = static_cast<const Element *>(x_pStorage);
		return pStorage[X_Advance(x_uBegin, uIndex)];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		const auto pStorage = static_cast<Element *>(x_pStorage);
		return pStorage[X_Advance(x_uBegin, uIndex)];
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

		const auto pNewStorage = ::operator new[](uBytesToAlloc);
		const auto pOldStorage = x_pStorage;
		const auto pNewBegin = static_cast<Element *>(pNewStorage);
		const auto pOldBegin = static_cast<Element *>(pOldStorage);
		auto pWrite = pNewBegin;
		try {
			if(x_uBegin <= x_uEnd){
				for(std::size_t i = x_uBegin; i != x_uEnd; ++i){
					Construct(pWrite, std::move_if_noexcept(pOldBegin[i]));
					++pWrite;
				}
			} else {
				for(std::size_t i = x_uBegin; i != x_uRingCap; ++i){
					Construct(pWrite, std::move_if_noexcept(pOldBegin[i]));
					++pWrite;
				}
				for(std::size_t i = 0; i != x_uEnd; ++i){
					Construct(pWrite, std::move_if_noexcept(pOldBegin[i]));
					++pWrite;
				}
			}
		} catch(...){
			while(pWrite != pNewBegin){
				--pWrite;
				Destruct(pWrite);
			}
			::operator delete[](pNewStorage);
			throw;
		}
		if(x_uBegin <= x_uEnd){
			for(std::size_t i = x_uBegin; i != x_uEnd; ++i){
				Destruct(pOldBegin + i);
			}
		} else {
			for(std::size_t i = x_uBegin; i != x_uRingCap; ++i){
				Destruct(pOldBegin + i);
			}
			for(std::size_t i = 0; i != x_uEnd; ++i){
				Destruct(pOldBegin + i);
			}
		}
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
	Element &Push(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto pStorage = static_cast<Element *>(x_pStorage);
		const auto uNewEnd = X_Advance(x_uEnd, 1);
		const auto pElem = pStorage + x_uEnd;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		x_uEnd = uNewEnd;

		return *pElem;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		const auto pStorage = static_cast<Element *>(x_pStorage);
		const auto uNewEnd = X_Retreat(x_uEnd, uCount);
		if(uNewEnd <= x_uEnd){
			for(std::size_t i = x_uEnd; i != uNewEnd; --i){
				Destruct(pStorage + i - 1);
			}
		} else {
			for(std::size_t i = x_uEnd; i != 0; --i){
				Destruct(pStorage + i - 1);
			}
			for(std::size_t i = x_uRingCap; i != uNewEnd; --i){
				Destruct(pStorage + i - 1);
			}
		}
		x_uEnd = uNewEnd;
	}

	template<typename ...ParamsT>
	Element &Unshift(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedUnshift(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedUnshift(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto pStorage = static_cast<Element *>(x_pStorage);
		const auto uNewBegin = X_Retreat(x_uBegin, 1);
		const auto pElem = pStorage + uNewBegin;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		x_uBegin = uNewBegin;

		return *pElem;
	}

	void Shift(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		const auto pStorage = static_cast<Element *>(x_pStorage);
		const auto uNewBegin = X_Advance(x_uBegin, uCount);
		if(x_uBegin <= uNewBegin){
			for(std::size_t i = x_uBegin; i != uNewBegin; ++i){
				Destruct(pStorage + i);
			}
		} else {
			for(std::size_t i = x_uBegin; i != x_uRingCap; ++i){
				Destruct(pStorage + i);
			}
			for(std::size_t i = 0; i != uNewBegin; ++i){
				Destruct(pStorage + i);
			}
		}
		x_uBegin = uNewBegin;
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
	Element *Emplace(const Element *pPos, ParamsT &&...vParams){
		if(!pPos){
			Push(std::forward<ParamsT>(vParams)...);
			return nullptr;
		}

		const auto uPos = static_cast<std::size_t>(pPos - X_GetStorage());
		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		if(std::is_nothrow_move_constructible<Element>::value){
			const auto vPrepared = X_PrepareForInsertion(uPos, 1);
			const auto pStorage = X_GetStorage();
			try {
				DefaultConstruct(pStorage + vPrepared.first, vParams...);
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
			queTemp.X_UncheckedUnsafeCopyFrom(*this, x_uBegin, uPos);
			queTemp.UncheckedPush(vParams...);
			queTemp.X_UncheckedUnsafeCopyFrom(*this, uPos, x_uEnd);
			*this = std::move(queTemp);
		}

		return X_GetStorage() + X_Advance(x_uBegin, uCountBefore);
	}

	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		if(!pPos){
			Append(uDeltaSize, vParams...);
			return nullptr;
		}

		const auto uPos = static_cast<std::size_t>(pPos - X_GetStorage());
		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		if(std::is_nothrow_move_constructible<Element>::value){
			const auto vPrepared = X_PrepareForInsertion(uPos, uDeltaSize);
			const auto pStorage = X_GetStorage();
			auto uWrite = vPrepared.first;
			try {
				for(std::size_t i = 0; i < uDeltaSize; ++i){
					DefaultConstruct(pStorage + uWrite, vParams...);
					uWrite = X_Advance(uWrite, 1);
				}
			} catch(...){
				while(uWrite != vPrepared.first){
					uWrite = X_Retreat(uWrite, 1);
					Destruct(pStorage + uWrite);
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
			queTemp.X_UncheckedUnsafeCopyFrom(*this, x_uBegin, uPos);
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				queTemp.UncheckedPush(vParams...);
			}
			queTemp.X_UncheckedUnsafeCopyFrom(*this, uPos, x_uEnd);
			*this = std::move(queTemp);
		}

		return X_GetStorage() + X_Advance(x_uBegin, uCountBefore);
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

		const auto uPos = static_cast<std::size_t>(pPos - X_GetStorage());

		const auto uCountBefore = X_Retreat(uPos, x_uBegin);
		if(kHasDeltaSizeHint && std::is_nothrow_move_constructible<Element>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));

			const auto vPrepared = X_PrepareForInsertion(uPos, uDeltaSize);
			const auto pStorage = X_GetStorage();
			auto uWrite = vPrepared.first;
			try {
				for(auto it = itBegin; it != itEnd; ++it){
					Construct(pStorage + uWrite, *it);
					uWrite = X_Advance(uWrite, 1);
				}
			} catch(...){
				while(uWrite != vPrepared.first){
					uWrite = X_Retreat(uWrite, 1);
					Destruct(pStorage + uWrite);
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
				queTemp.X_UncheckedUnsafeCopyFrom(*this, x_uBegin, uPos);
				for(auto it = itBegin; it != itEnd; ++it){
					queTemp.UncheckedPush(*it);
				}
				queTemp.X_UncheckedUnsafeCopyFrom(*this, uPos, x_uEnd);
				*this = std::move(queTemp);
			} else {
				RingQueue queTemp;
				queTemp.Reserve(GetCapacity());
				queTemp.X_UncheckedUnsafeCopyFrom(*this, x_uBegin, uPos);
				for(auto it = itBegin; it != itEnd; ++it){
					queTemp.Push(*it);
				}
				queTemp.X_UnsafeCopyFrom(*this, uPos, x_uEnd);
				*this = std::move(queTemp);
			}
		}

		return X_GetStorage() + X_Advance(x_uBegin, uCountBefore);
	}
	Element *Insert(const Element *pPos, std::initializer_list<Element> ilElements){
		return Insert(pPos, ilElements.begin(), ilElements.end());
	}

	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(std::is_nothrow_move_constructible<Element>::value) {
		if(pBegin == pEnd){
			return const_cast<Element *>(pEnd);
		}
		ASSERT(pBegin);

		const auto pStorage = X_GetStorage();
		const auto uBegin = static_cast<std::size_t>(pBegin - pStorage);
		if(!pEnd){
			const auto uDeltaSize = X_Retreat(x_uEnd, uBegin);

			Pop(uDeltaSize);
			return nullptr;
		}
		const auto uCountBefore = X_Retreat(uBegin, x_uBegin);
		const auto uEnd = static_cast<std::size_t>(pEnd - pStorage);
		if(std::is_nothrow_move_constructible<Element>::value){
			const auto uCountAfter = X_Retreat(x_uEnd, uEnd);
			if(uCountBefore >= uCountAfter){
				if(uBegin <= uEnd){
					for(std::size_t i = uBegin; i != uEnd; ++i){
						Destruct(pStorage + i);
					}
				} else {
					for(std::size_t i = uBegin; i != x_uRingCap; ++i){
						Destruct(pStorage + i);
					}
					for(std::size_t i = 0; i != uEnd; ++i){
						Destruct(pStorage + i);
					}
				}

				auto uNewEnd = uBegin;
				if(uEnd <= x_uEnd){
					for(std::size_t i = uEnd; i != x_uEnd; ++i){
						Construct(pStorage + uNewEnd, std::move(pStorage[i]));
						uNewEnd = X_Advance(uNewEnd, 1);
						Destruct(pStorage + i);
					}
				} else {
					for(std::size_t i = uEnd; i != x_uRingCap; ++i){
						Construct(pStorage + uNewEnd, std::move(pStorage[i]));
						uNewEnd = X_Advance(uNewEnd, 1);
						Destruct(pStorage + i);
					}
					for(std::size_t i = 0; i != x_uEnd; ++i){
						Construct(pStorage + uNewEnd, std::move(pStorage[i]));
						uNewEnd = X_Advance(uNewEnd, 1);
						Destruct(pStorage + i);
					}
				}
				x_uEnd = uNewEnd;
			} else {
				if(uBegin <= uEnd){
					for(std::size_t i = uEnd; i != uBegin; --i){
						Destruct(pStorage + i - 1);
					}
				} else {
					for(std::size_t i = uEnd; i != 0; --i){
						Destruct(pStorage + i - 1);
					}
					for(std::size_t i = x_uRingCap; i != uBegin; --i){
						Destruct(pStorage + i - 1);
					}
				}

				auto uNewBegin = uEnd;
				if(uBegin > x_uBegin){
					for(std::size_t i = uBegin; i != x_uBegin; --i){
						uNewBegin = X_Retreat(uNewBegin, 1);
						Construct(pStorage + uNewBegin, std::move(pStorage[i - 1]));
						Destruct(pStorage + i - 1);
					}
				} else if(uBegin < x_uBegin){
					for(std::size_t i = uBegin; i != 0; --i){
						uNewBegin = X_Retreat(uNewBegin, 1);
						Construct(pStorage + uNewBegin, std::move(pStorage[i - 1]));
						Destruct(pStorage + i - 1);
					}
					for(std::size_t i = x_uRingCap; i != x_uBegin; --i){
						uNewBegin = X_Retreat(uNewBegin, 1);
						Construct(pStorage + uNewBegin, std::move(pStorage[i - 1]));
						Destruct(pStorage + i - 1);
					}
				}
				x_uBegin = uNewBegin;
			}
		} else {
			RingQueue queTemp;
			queTemp.Reserve(GetCapacity());
			queTemp.X_UncheckedUnsafeCopyFrom(*this, x_uBegin, uBegin);
			queTemp.X_UncheckedUnsafeCopyFrom(*this, uEnd, x_uEnd);
			*this = std::move(queTemp);
		}

		return pStorage + X_Advance(x_uBegin, uCountBefore);
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(std::declval<RingQueue &>().Erase(pPos, pPos))) {
		ASSERT(pPos);

		return Erase(pPos, GetNext(pPos));
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}
};

template<typename ElementT>
void swap(RingQueue<ElementT> &lhs, RingQueue<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ElementT>
decltype(auto) begin(const RingQueue<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT>
decltype(auto) begin(RingQueue<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT>
decltype(auto) cbegin(const RingQueue<ElementT> &rhs) noexcept {
	return begin(rhs);
}
template<typename ElementT>
decltype(auto) end(const RingQueue<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) end(RingQueue<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) cend(const RingQueue<ElementT> &rhs) noexcept {
	return end(rhs);
}

}

#endif
