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
	std::size_t x_uFirst;
	std::size_t x_uLast;
	std::size_t x_uRingCap;

public:
	constexpr RingQueue() noexcept
		: x_pStorage(nullptr), x_uFirst(1), x_uLast(0), x_uRingCap(1)
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
		if(rhs.IsEmpty()){
			return;
		}

		Reserve(rhs.GetSize());
		const auto pStorage = rhs.X_GetStorage();
		X_UnsafeCopyFrom<false>(rhs, pStorage + rhs.x_uFirst, pStorage + rhs.x_uLast + 1);
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

	ElementT *X_PrepareForInsertion(const ElementT *pPos, std::size_t uDeltaSize){
		ASSERT(std::is_nothrow_move_constructible<ElementT>::value);
		ASSERT(!IsEmpty());
		ASSERT(pPos);

		auto uIndex = static_cast<std::size_t>(pPos - X_GetStorage());
		if(uIndex < x_uFirst){
			uIndex += x_uRingCap;
		}
		uIndex -= x_uFirst;
		ReserveMore(uDeltaSize);
		const auto pStorage = X_GetStorage();
		auto uOffset = uIndex + x_uFirst;
		if(uOffset >= x_uRingCap){
			uOffset -= x_uRingCap;
		}

		auto uReadOffset = x_uLast;
		for(;;){
			auto uWriteOffset = uReadOffset + uDeltaSize;
			if(uWriteOffset >= x_uRingCap){
				uWriteOffset -= x_uRingCap;
			}
			Construct(pStorage + uWriteOffset, std::move(pStorage[uReadOffset]));
			Destruct(pStorage + uReadOffset);

			if(uReadOffset == uOffset){
				break;
			}

			if(uReadOffset == 0){
				uReadOffset = x_uRingCap;
			}
			--uReadOffset;
		}

		return pStorage + uOffset;
	}
	void X_UndoPreparation(ElementT *pPrepared, std::size_t uDeltaSize) noexcept {
		ASSERT(std::is_nothrow_move_constructible<ElementT>::value);
		ASSERT(!IsEmpty());
		ASSERT(pPrepared);
		ASSERT(uDeltaSize <= GetCapacity() - GetSize());

		const auto pStorage = X_GetStorage();
		const auto uOffset = static_cast<std::size_t>(pPrepared - pStorage);

		auto uWriteOffset = uOffset;
		for(;;){
			auto uReadOffset = uWriteOffset + uDeltaSize;
			if(uReadOffset >= x_uRingCap){
				uReadOffset -= x_uRingCap;
			}
			Construct(pStorage + uWriteOffset, std::move(pStorage[uReadOffset]));
			Destruct(pStorage + uReadOffset);

			if(uWriteOffset == x_uLast){
				break;
			}

			++uWriteOffset;
			if(uWriteOffset == x_uRingCap){
				uWriteOffset = 0;
			}
		}
	}

	// 基本异常安全保证。
	template<bool kChecked>
	void X_UnsafeCopyFrom(const RingQueue &queFrom, const ElementT *pBegin, const ElementT *pEnd){
		ASSERT(!queFrom.IsEmpty());
 		ASSERT(&queFrom != this);
		ASSERT(pBegin && pEnd);

		const auto pStorage = queFrom.X_GetStorage();
		if(pBegin < pEnd){
			for(auto pRead = pBegin; pRead != pEnd; ++pRead){
				if(kChecked){
					Push(*pRead);
				} else {
					UncheckedPush(*pRead);
				}
			}
		} else {
			const auto pWrapAt = pStorage + queFrom.x_uRingCap;
			for(auto pRead = pBegin; pRead != pWrapAt; ++pRead){
				if(kChecked){
					Push(*pRead);
				} else {
					UncheckedPush(*pRead);
				}
			}
			for(auto pRead = pStorage; pRead != pEnd; ++pRead){
				if(kChecked){
					Push(*pRead);
				} else {
					UncheckedPush(*pRead);
				}
			}
		}
	}

public:
	// 容器需求。
	using ElementType     = ElementT;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <RingQueue>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <RingQueue>;

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		Pop(GetSize());
	}

	const ElementType *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + x_uFirst;
	}
	ElementType *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + x_uFirst;
	}
	const ElementType *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + x_uLast;
	}
	ElementType *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		const auto pStorage = X_GetStorage();
		return pStorage + x_uLast;
	}

	const ElementType *GetPrev(const ElementType *pPos) const noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		if(uOffset == x_uFirst){
			return nullptr;
		}
		if(uOffset == 0){
			uOffset = x_uRingCap;
		}
		--uOffset;
		return pStorage + uOffset;
	}
	ElementType *GetPrev(ElementType *pPos) noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		if(uOffset == x_uFirst){
			return nullptr;
		}
		if(uOffset == 0){
			uOffset = x_uRingCap;
		}
		--uOffset;
		return pStorage + uOffset;
	}
	const ElementType *GetNext(const ElementType *pPos) const noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		if(uOffset == x_uLast){
			return nullptr;
		}
		++uOffset;
		if(uOffset == x_uRingCap){
			uOffset = 0;
		}
		return pStorage + uOffset;
	}
	ElementType *GetNext(ElementType *pPos) noexcept {
		ASSERT(pPos);

		const auto pStorage = X_GetStorage();
		auto uOffset = static_cast<std::size_t>(pPos - pStorage);
		if(uOffset == x_uLast){
			return nullptr;
		}
		++uOffset;
		if(uOffset == x_uRingCap){
			uOffset = 0;
		}
		return pStorage + uOffset;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}

	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}

	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}

	void Swap(RingQueue &rhs) noexcept {
		std::swap(x_pStorage,  rhs.x_pStorage);
		std::swap(x_uFirst,    rhs.x_uFirst);
		std::swap(x_uLast,     rhs.x_uLast);
		std::swap(x_uRingCap,  rhs.x_uRingCap);
	}

	// RingQueue 需求。
	std::size_t GetSize() const noexcept {
		std::size_t uSize;
		if(x_uFirst <= x_uLast){
			uSize = x_uLast - x_uFirst + 1;
		} else {
			uSize = x_uRingCap - x_uFirst + x_uLast + 1;
		}
		if(uSize >= x_uRingCap){
			uSize -= x_uRingCap;
		}
		return uSize;
	}
	std::size_t GetCapacity() noexcept {
		return x_uRingCap - 1;
	}

	const ElementType &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	ElementType &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	const ElementType &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		const auto pStorage = static_cast<const ElementType *>(x_pStorage);
		auto uOffset = x_uFirst + uIndex;
		if(uOffset >= x_uRingCap){
			uOffset -= x_uRingCap;
		}
		return pStorage[uOffset];
	}
	ElementType &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		const auto pStorage = static_cast<ElementType *>(x_pStorage);
		auto uOffset = x_uFirst + uIndex;
		if(uOffset >= x_uRingCap){
			uOffset -= x_uRingCap;
		}
		return pStorage[uOffset];
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
		const auto uBytesToAlloc = sizeof(ElementType) * uElementsToAlloc;
		if(uBytesToAlloc / sizeof(ElementType) != uElementsToAlloc){
			throw std::bad_array_new_length();
		}

		const auto pNewStorage = static_cast<ElementType *>(::operator new[](uBytesToAlloc));
		const auto pOldStorage = static_cast<ElementType *>(x_pStorage);
		auto pWrite = pNewStorage;
		if(!IsEmpty()){
			try {
				if(x_uFirst < x_uLast){
					for(std::size_t i = x_uFirst; i <= x_uLast; ++i){
						Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
						++pWrite;
					}
				} else {
					for(std::size_t i = x_uFirst; i < x_uRingCap; ++i){
						Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
						++pWrite;
					}
					for(std::size_t i = 0; i <= x_uLast; ++i){
						Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
						++pWrite;
					}
				}
			} catch(...){
				while(pWrite != pNewStorage){
					--pWrite;
					Destruct(pWrite);
				}
				::operator delete[](pNewStorage);
				throw;
			}
			if(x_uFirst < x_uLast){
				for(std::size_t i = x_uFirst; i <= x_uLast; ++i){
					Destruct(pOldStorage + i);
				}
			} else {
				for(std::size_t i = x_uFirst; i < x_uRingCap; ++i){
					Destruct(pOldStorage + i);
				}
				for(std::size_t i = 0; i <= x_uLast; ++i){
					Destruct(pOldStorage + i);
				}
			}
		}
		::operator delete[](pOldStorage);

		std::size_t uNewFirst, uNewLast;
		if(pWrite == pNewStorage){
			uNewFirst = 1;
			uNewLast = 0;
		} else {
			uNewFirst = 0;
			uNewLast = static_cast<std::size_t>(pWrite - pNewStorage) - 1;
		}

		x_pStorage  = pNewStorage;
		x_uFirst    = uNewFirst;
		x_uLast     = uNewLast;
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
	void Push(ParamsT &&...vParams){
		ReserveMore(1);
		UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	void UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementType, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto pStorage = static_cast<ElementType *>(x_pStorage);
		auto uNewLast = x_uLast;
		++uNewLast;
		if(uNewLast == x_uRingCap){
			uNewLast = 0;
		}
		Construct(pStorage + uNewLast, std::forward<ParamsT>(vParams)...);
		x_uLast = uNewLast;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		if(uCount == 0){
			return;
		}

		const auto pStorage = X_GetStorage();
		auto uNewLast = x_uLast;
		if(uNewLast < uCount){
			uNewLast += x_uRingCap;
		}
		uNewLast -= uCount;
		if(uNewLast <= x_uLast){
			for(std::size_t i = 0; i < uCount; ++i){
				Destruct(pStorage + x_uLast - i);
			}
		} else {
			const auto uCountWrapped = x_uLast + 1;
			for(std::size_t i = 0; i < uCountWrapped; ++i){
				Destruct(pStorage + x_uLast - i);
			}
			for(std::size_t i = 0; i < uCount - uCountWrapped; ++i){
				Destruct(pStorage + x_uRingCap - i - 1);
			}
		}
		x_uLast = uNewLast;
	}

	template<typename ...ParamsT>
	void Unshift(ParamsT &&...vParams){
		ReserveMore(1);
		UncheckedUnshift(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	void UncheckedUnshift(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementType, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - GetSize() > 0);

		const auto pStorage = static_cast<ElementType *>(x_pStorage);
		auto uNewFirst = x_uFirst;
		if(uNewFirst == 0){
			uNewFirst = x_uRingCap;
		}
		--uNewFirst;
		Construct(pStorage + uNewFirst, std::forward<ParamsT>(vParams)...);
		x_uFirst = uNewFirst;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= GetSize());

		if(uCount == 0){
			return;
		}

		const auto pStorage = X_GetStorage();
		auto uNewFirst = x_uFirst;
		uNewFirst += uCount;
		if(uNewFirst >= x_uRingCap){
			uNewFirst -= x_uRingCap;
		}
		if(x_uFirst <= uNewFirst){
			for(std::size_t i = 0; i < uCount; ++i){
				Destruct(pStorage + x_uFirst + i);
			}
		} else {
			const auto uCountNonWrapped = x_uRingCap - x_uFirst;
			for(std::size_t i = 0; i < uCountNonWrapped; ++i){
				Destruct(pStorage + x_uFirst + i);
			}
			for(std::size_t i = 0; i < uCount - uCountNonWrapped; ++i){
				Destruct(pStorage + i);
			}
		}
		x_uFirst = uNewFirst;
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
				for(auto it = itBegin; it != itEnd; ++it){
					UncheckedPush(*it);
					++uElementsPushed;
				}
			} else {
				for(auto it = itBegin; it != itEnd; ++it){
					Push(*it);
					++uElementsPushed;
				}
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	void Append(std::initializer_list<ElementType> ilElements){
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
				for(auto it = itBegin; it != itEnd; ++it){
					UncheckedUnshift(*it);
					++uElementsUnshifted;
				}
			} else {
				for(auto it = itBegin; it != itEnd; ++it){
					Unshift(*it);
					++uElementsUnshifted;
				}
			}
		} catch(...){
			Shift(uElementsUnshifted);
			throw;
		}
	}
	void Prepend(std::initializer_list<ElementType> ilElements){
		Prepend(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	void Emplace(const ElementType *pPos, ParamsT &&...vParams){
		if(!pPos){
			Push(std::forward<ParamsT>(vParams)...);
			return;
		}

		const auto pStorage = X_GetStorage();

		if(std::is_nothrow_move_constructible<ElementType>::value){
			const auto pWriteBegin = X_PrepareForInsertion(pPos, 1);
			try {
				DefaultConstruct(pWriteBegin, std::forward<ParamsT>(vParams)...);
			} catch(...){
				X_UndoPreparation(pWriteBegin, 1);
				throw;
			}
			++x_uLast;
			if(x_uLast == x_uRingCap){
				x_uLast = 0;
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
			queTemp.X_UnsafeCopyFrom<false>(*this, GetFirst(), pPos);
			queTemp.UncheckedPush(std::forward<ParamsT>(vParams)...);
			queTemp.X_UnsafeCopyFrom<false>(*this, pPos, pStorage + x_uLast + 1);
			*this = std::move(queTemp);
		}
	}

	template<typename ...ParamsT>
	void Insert(const ElementType *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		if(!pPos){
			Append(uDeltaSize, vParams...);
			return;
		}

		const auto pStorage = X_GetStorage();

		if(std::is_nothrow_move_constructible<ElementType>::value){
			const auto pWriteBegin = X_PrepareForInsertion(pPos, uDeltaSize);
			auto pWrite = pWriteBegin;
			try {
				const auto uWrapAt = static_cast<std::size_t>(pStorage + x_uRingCap - pWriteBegin);
				if(uDeltaSize <= uWrapAt){
					for(std::size_t i = 0; i < uDeltaSize; ++i){
						DefaultConstruct(pWrite, vParams...);
						++pWrite;
					}
				} else {
					for(std::size_t i = 0; i < uWrapAt; ++i){
						DefaultConstruct(pWrite, vParams...);
						++pWrite;
					}
					pWrite = pStorage;
					for(std::size_t i = uWrapAt; i < uDeltaSize; ++i){
						DefaultConstruct(pWrite, vParams...);
						++pWrite;
					}
				}
			} catch(...){
				while(pWrite != pWriteBegin){
					if(pWrite == pStorage){
						pWrite = pStorage + x_uRingCap;
					}
					--pWrite;
					Destruct(pWrite);
				}
				X_UndoPreparation(pWriteBegin, uDeltaSize);
				throw;
			}
			x_uLast += uDeltaSize;
			if(x_uLast >= x_uRingCap){
				x_uLast -= x_uRingCap;
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
			queTemp.X_UnsafeCopyFrom<false>(*this, GetFirst(), pPos);
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				queTemp.UncheckedPush(vParams...);
			}
			queTemp.X_UnsafeCopyFrom<false>(*this, pPos, pStorage + x_uLast + 1);
			*this = std::move(queTemp);
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Insert(const ElementType *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		if(!pPos){
			Append(itBegin, itEnd);
			return;
		}

		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		const auto pStorage = X_GetStorage();

		if(kHasDeltaSizeHint && std::is_nothrow_move_constructible<ElementType>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			const auto pWriteBegin = X_PrepareForInsertion(pPos, uDeltaSize);
			auto pWrite = pWriteBegin;
			try {
				const auto uWrapAt = static_cast<std::size_t>(pStorage + x_uRingCap - pWriteBegin);
				if(uDeltaSize <= uWrapAt){
					for(auto it = itBegin; it != itEnd; ++it){
						Construct(pWrite, *it);
						++pWrite;
					}
				} else {
					auto it = itBegin;
					for(std::size_t i = 0; i < uWrapAt; ++i, ++it){
						Construct(pWrite, *it);
						++pWrite;
					}
					for(; it != itEnd; ++it){
						Construct(pWrite, *it);
						++pWrite;
					}
				}
			} catch(...){
				while(pWrite != pWriteBegin){
					if(pWrite == pStorage){
						pWrite = pStorage + x_uRingCap;
					}
					--pWrite;
					Destruct(pWrite);
				}
				X_UndoPreparation(pWriteBegin, uDeltaSize);
				throw;
			}
			x_uLast += uDeltaSize;
			if(x_uLast >= x_uRingCap){
				x_uLast -= x_uRingCap;
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
				queTemp.X_UnsafeCopyFrom<false>(*this, GetFirst(), pPos);
				for(auto it = itBegin; it != itEnd; ++it){
					queTemp.UncheckedPush(*it);
				}
				queTemp.X_UnsafeCopyFrom<false>(*this, pPos, pStorage + x_uLast + 1);
				*this = std::move(queTemp);
			} else {
				RingQueue queTemp;
				queTemp.Reserve(GetCapacity());
				queTemp.X_UnsafeCopyFrom<false>(*this, GetFirst(), pPos);
				for(auto it = itBegin; it != itEnd; ++it){
					queTemp.Push(*it);
				}
				queTemp.X_UnsafeCopyFrom<false>(*this, pPos, pStorage + x_uLast + 1);
				*this = std::move(queTemp);
			}
		}
	}
	void Insert(const ElementType *pPos, std::initializer_list<ElementType> ilElements){
		Insert(pPos, ilElements.begin(), ilElements.end());
	}

	void Erase(const ElementType *pBegin, const ElementType *pEnd) noexcept(std::is_nothrow_move_constructible<ElementType>::value) {
		if(pBegin == pEnd){
			return;
		}
		ASSERT(pBegin);

		const auto pStorage = X_GetStorage();
		const auto uBeginOffset = static_cast<std::size_t>(pBegin - pStorage);

		if(!pEnd){
			auto uDeltaCount = x_uLast;
			if(uDeltaCount < uBeginOffset){
				uDeltaCount += x_uRingCap;
			}
			uDeltaCount -= uBeginOffset;
			++uDeltaCount;

			Pop(uDeltaCount);
			return;
		}

		if(std::is_nothrow_move_constructible<ElementType>::value){
			auto uDeltaCount = static_cast<std::size_t>(pEnd - pStorage);
			if(uDeltaCount < uBeginOffset){
				uDeltaCount += x_uRingCap;
			}
			uDeltaCount -= uBeginOffset;

			auto pWrite = const_cast<ElementType *>(pBegin);

			if(pBegin < pEnd){
				for(auto pCur = pWrite; pCur != pEnd; ++pCur){
					Destruct(pCur);
				}
			} else {
				const auto pWrapAt = pStorage + x_uRingCap;
				for(auto pCur = pWrite; pCur != pWrapAt; ++pCur){
					Destruct(pCur);
				}
				for(auto pCur = pStorage; pCur != pEnd; ++pCur){
					Destruct(pCur);
				}
			}

			const auto pReadBegin = const_cast<ElementType *>(pEnd);
			const auto pReadEnd = pStorage + x_uLast + 1;
			if(pReadBegin < pReadEnd){
				for(auto pRead = pReadBegin; pRead != pReadEnd; ++pRead){
					Construct(pWrite, std::move(*pRead));
					Destruct(pRead);
					++pWrite;
					if(pWrite == pStorage + x_uRingCap){
						pWrite = pStorage;
					}
				}
			} else {
				const auto pWrapAt = pStorage + x_uRingCap;
				for(auto pRead = pReadBegin; pRead != pWrapAt; ++pRead){
					Construct(pWrite, std::move(*pRead));
					Destruct(pRead);
					++pWrite;
					if(pWrite == pStorage + x_uRingCap){
						pWrite = pStorage;
					}
				}
				for(auto pRead = pStorage; pRead != pReadEnd; ++pRead){
					Construct(pWrite, std::move(*pRead));
					Destruct(pRead);
					++pWrite;
					if(pWrite == pStorage + x_uRingCap){
						pWrite = pStorage;
					}
				}
			}

			if(x_uLast < uDeltaCount){
				x_uLast += x_uRingCap;
			}
			x_uLast -= uDeltaCount;
		} else {
			RingQueue queTemp;
			queTemp.Reserve(GetCapacity());
			queTemp.X_UnsafeCopyFrom<false>(*this, GetFirst(), pBegin);
			queTemp.X_UnsafeCopyFrom<false>(*this, pEnd, pStorage + x_uLast + 1);
			*this = std::move(queTemp);
		}
	}
	void Erase(const ElementType *pPos) noexcept(noexcept(std::declval<RingQueue &>().Erase(pPos, pPos))) {
		ASSERT(pPos);

		Erase(pPos, GetNext(pPos));
	}

	const ElementType &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	ElementType &operator[](std::size_t uIndex) noexcept {
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
decltype(auto) end(const RingQueue<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) end(RingQueue<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}

}

#endif
