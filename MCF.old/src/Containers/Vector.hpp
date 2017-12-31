// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_VECTOR_HPP_
#define MCF_CONTAINERS_VECTOR_HPP_

#include "../Core/_CheckedSizeArithmetic.hpp"
#include "../Core/_Enumerator.hpp"
#include "../Core/DefaultAllocator.hpp"
#include "../Core/Assert.hpp"
#include "../Core/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include "../Core/ArrayView.hpp"
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT, class AllocatorT = DefaultAllocator>
class Vector {
public:
	// 容器需求。
	using Element         = ElementT;
	using Allocator       = AllocatorT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <Vector>;
	using Enumerator      = Impl_Enumerator::Enumerator      <Vector>;

private:
	Element *x_pStorage;
	std::size_t x_uSize;
	std::size_t x_uCapacity;

public:
	constexpr Vector() noexcept
		: x_pStorage(nullptr), x_uSize(0), x_uCapacity(0)
	{ }
	template<typename ...ParamsT>
	explicit Vector(std::size_t uSize, const ParamsT &...vParams)
		: Vector()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	Vector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: Vector()
	{
		Append(itBegin, itEnd);
	}
	Vector(std::initializer_list<Element> ilInitList)
		: Vector(ilInitList.begin(), ilInitList.end())
	{ }
	Vector(const Vector &vOther)
		: Vector()
	{
		Reserve(vOther.GetSize());
		for(std::size_t uIndex = 0; uIndex < vOther.GetSize(); ++uIndex){
			UncheckedPush(vOther.x_pStorage[uIndex]);
		}
	}
	Vector(Vector &&vOther) noexcept
		: Vector()
	{
		vOther.Swap(*this);
	}
	Vector &operator=(const Vector &vOther){
		if(std::is_nothrow_copy_constructible<Element>::value || IsEmpty()){
			Reserve(vOther.GetSize());
			try {
				for(std::size_t uIndex = 0; uIndex < vOther.GetSize(); ++uIndex){
					UncheckedPush(vOther.x_pStorage[uIndex]);
				}
			} catch(...){
				Clear();
				throw;
			}
		} else {
			Vector(vOther).Swap(*this);
		}
		return *this;
	}
	Vector &operator=(Vector &&vOther) noexcept {
		vOther.Swap(*this);
		return *this;
	}
	~Vector(){
		Clear();
		Allocator()(static_cast<void *>(x_pStorage));
#ifndef NDEBUG
		__builtin_memset(&x_pStorage, 0xEF, sizeof(x_pStorage));
#endif
	}

private:
	void X_PrepareForInsertion(std::size_t uPos, std::size_t uDeltaSize){
		MCF_DEBUG_CHECK(std::is_nothrow_move_constructible<Element>::value);
		MCF_DEBUG_CHECK(!IsEmpty());
		MCF_DEBUG_CHECK(uPos <= x_uSize);

		ReserveMore(uDeltaSize);
		for(std::size_t uIndex = x_uSize; uIndex > uPos; --uIndex){
			Construct(x_pStorage + uIndex - 1 + uDeltaSize, std::move(x_pStorage[uIndex - 1]));
			Destruct(x_pStorage + uIndex - 1);
		}
	}
	void X_UndoPreparation(std::size_t uPos, std::size_t uDeltaSize) noexcept {
		MCF_DEBUG_CHECK(std::is_nothrow_move_constructible<Element>::value);
		MCF_DEBUG_CHECK(!IsEmpty());
		MCF_DEBUG_CHECK(uPos <= x_uSize);
		MCF_DEBUG_CHECK(uDeltaSize <= x_uSize - uPos);

		for(std::size_t uIndex = uPos; uIndex < x_uSize; ++uIndex){
			Construct(x_pStorage + uIndex, std::move(x_pStorage[uIndex + uDeltaSize]));
			Destruct(x_pStorage + uIndex + uDeltaSize);
		}
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear() noexcept {
		Pop(GetSize());
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			for(std::size_t uIndex = 0; uIndex < x_uSize; ++uIndex){
				*itOutput = std::move(x_pStorage[uIndex]);
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
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

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

	void Swap(Vector &vOther) noexcept {
		using std::swap;
		swap(x_pStorage,  vOther.x_pStorage);
		swap(x_uSize,     vOther.x_uSize);
		swap(x_uCapacity, vOther.x_uCapacity);
	}

	// Vector 需求。
	const Element *GetData() const noexcept {
		return x_pStorage;
	}
	Element *GetData() noexcept {
		return x_pStorage;
	}
	const Element *GetConstData() const noexcept {
		return GetData();
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	std::size_t GetCapacity() const noexcept {
		return x_uCapacity;
	}
	std::size_t GetCapacityRemaining() const noexcept {
		return GetCapacity() - GetSize();
	}

	const Element *GetBegin() const noexcept {
		return x_pStorage;
	}
	Element *GetBegin() noexcept {
		return x_pStorage;
	}
	const Element *GetConstBegin() const noexcept {
		return GetBegin();
	}
	const Element *GetEnd() const noexcept {
		return x_pStorage + x_uSize;
	}
	Element *GetEnd() noexcept {
		return x_pStorage + x_uSize;
	}
	const Element *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Vector: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Vector: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex < GetSize());

		return GetBegin()[uIndex];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		MCF_DEBUG_CHECK(uIndex < GetSize());

		return GetBegin()[uIndex];
	}

	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity <= uOldCapacity){
			return;
		}

		auto uElementsToAlloc = uOldCapacity + 1;
		uElementsToAlloc += (uElementsToAlloc >> 1);
		uElementsToAlloc = (uElementsToAlloc + 0x0F) & (std::size_t)-0x10;
		if(uElementsToAlloc < uNewCapacity){
			uElementsToAlloc = uNewCapacity;
		}
		const auto uBytesToAlloc = Impl_CheckedSizeArithmetic::Mul(sizeof(Element), uElementsToAlloc);
		const auto pNewStorage = static_cast<Element *>(Allocator()(uBytesToAlloc));
		const auto pOldStorage = x_pStorage;
		auto pWrite = pNewStorage;
		try {
			for(std::size_t uIndex = 0; uIndex < GetSize(); ++uIndex){
				Construct(pWrite, std::move_if_noexcept(pOldStorage[uIndex]));
				++pWrite;
			}
		} catch(...){
			while(pWrite != pNewStorage){
				--pWrite;
				Destruct(pWrite);
			}
			Allocator()(static_cast<void *>(pNewStorage));
			throw;
		}
		for(std::size_t uIndex = GetSize(); uIndex > 0; --uIndex){
			Destruct(pOldStorage + uIndex - 1);
		}
		Allocator()(static_cast<void *>(pOldStorage));

		x_pStorage  = pNewStorage;
		x_uCapacity = uElementsToAlloc;
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaCapacity, GetSize());
		Reserve(uNewCapacity);
	}

	template<typename ...ParamsT>
	Element *Resize(std::size_t uSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		if(uSize > uOldSize){
			Append(uSize - uOldSize, vParams...);
		} else {
			Pop(uOldSize - uSize);
		}
		return GetData();
	}
	template<typename ...ParamsT>
	Element *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		Append(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}
	std::pair<Element *, std::size_t> ResizeToCapacity() noexcept {
		const auto uOldSize = GetSize();
		const auto uNewSize = GetCapacity();
		const auto uDeltaSize = uNewSize - uOldSize;
		UncheckedAppend(uDeltaSize);
		return std::make_pair(GetData() + uOldSize, uDeltaSize);
	}

	template<typename ...ParamsT>
	Element &Push(ParamsT &&...vParams){
		ReserveMore(1);
		return UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Element &UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
		MCF_DEBUG_CHECK(GetCapacity() - GetSize() > 0);

		const auto pElement = x_pStorage + x_uSize;
		DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		++x_uSize;

		return *pElement;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= GetSize());

		for(std::size_t uIndex = 0; uIndex < uCount; ++uIndex){
			Destruct(x_pStorage + x_uSize - 1 - uIndex);
		}
		x_uSize -= uCount;
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
			for(auto itCur = itBegin; itCur != itEnd; ++itCur){
				if(kHasDeltaSizeHint){
					UncheckedPush(*itCur);
				} else {
					Push(*itCur);
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
			for(auto itCur = itBegin; itCur != itEnd; ++itCur){
				UncheckedPush(*itCur);
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
			uOffset = x_uSize;
		}

		if(std::is_nothrow_move_constructible<Element>::value){
			X_PrepareForInsertion(uOffset, 1);
			auto uWrite = uOffset;
			try {
				DefaultConstruct(x_pStorage + uWrite, std::forward<ParamsT>(vParams)...);
			} catch(...){
				X_UndoPreparation(uOffset, 1);
				throw;
			}
			x_uSize += 1;
		} else {
			auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(1, x_uSize);
			const auto uCapacity = GetCapacity();
			if(uNewCapacity < uCapacity){
				uNewCapacity = uCapacity;
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(std::size_t uIndex = 0; uIndex < uOffset; ++uIndex){
				vecTemp.UncheckedPush(x_pStorage[uIndex]);
			}
			vecTemp.UncheckedPush(std::forward<ParamsT>(vParams)...);
			for(std::size_t uIndex = uOffset; uIndex < x_uSize; ++uIndex){
				vecTemp.UncheckedPush(x_pStorage[uIndex]);
			}
			*this = std::move(vecTemp);
		}

		return x_pStorage + uOffset;
	}

	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		std::size_t uOffset;
		if(pPos){
			uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		} else {
			uOffset = x_uSize;
		}

		if(uDeltaSize != 0){
			if(std::is_nothrow_move_constructible<Element>::value){
				X_PrepareForInsertion(uOffset, uDeltaSize);
				auto uWrite = uOffset;
				try {
					for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
						DefaultConstruct(x_pStorage + uWrite, vParams...);
						++uWrite;
					}
				} catch(...){
					while(uWrite != uOffset){
						--uWrite;
						Destruct(x_pStorage + uWrite);
					}
					X_UndoPreparation(uOffset, uDeltaSize);
					throw;
				}
				x_uSize += uDeltaSize;
			} else {
				auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaSize, x_uSize);
				const auto uCapacity = GetCapacity();
				if(uNewCapacity < uCapacity){
					uNewCapacity = uCapacity;
				}
				Vector vecTemp;
				vecTemp.Reserve(uNewCapacity);
				for(std::size_t uIndex = 0; uIndex < uOffset; ++uIndex){
					vecTemp.UncheckedPush(x_pStorage[uIndex]);
				}
				for(std::size_t uIndex = 0; uIndex < uDeltaSize; ++uIndex){
					vecTemp.UncheckedPush(vParams...);
				}
				for(std::size_t uIndex = uOffset; uIndex < x_uSize; ++uIndex){
					vecTemp.UncheckedPush(x_pStorage[uIndex]);
				}
				*this = std::move(vecTemp);
			}
		}

		return x_pStorage + uOffset;
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	Element *Insert(const Element *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		std::size_t uOffset;
		if(pPos){
			uOffset = static_cast<std::size_t>(pPos - x_pStorage);
		} else {
			uOffset = x_uSize;
		}

		if(itBegin != itEnd){
			if(std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value){
				const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
				if(std::is_nothrow_move_constructible<Element>::value){
					X_PrepareForInsertion(uOffset, uDeltaSize);
					auto uWrite = uOffset;
					try {
						for(auto itCur = itBegin; itCur != itEnd; ++itCur){
							DefaultConstruct(x_pStorage + uWrite, *itCur);
							++uWrite;
						}
					} catch(...){
						while(uWrite != uOffset){
							--uWrite;
							Destruct(x_pStorage + uWrite);
						}
						X_UndoPreparation(uOffset, uDeltaSize);
						throw;
					}
					x_uSize += uDeltaSize;
				} else {
					auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaSize, x_uSize);
					const auto uCapacity = GetCapacity();
					if(uNewCapacity < uCapacity){
						uNewCapacity = uCapacity;
					}
					Vector vecTemp;
					vecTemp.Reserve(uNewCapacity);
					for(std::size_t uIndex = 0; uIndex < uOffset; ++uIndex){
						vecTemp.UncheckedPush(x_pStorage[uIndex]);
					}
					for(auto itCur = itBegin; itCur != itEnd; ++itCur){
						vecTemp.UncheckedPush(*itCur);
					}
					for(std::size_t uIndex = uOffset; uIndex < x_uSize; ++uIndex){
						vecTemp.UncheckedPush(x_pStorage[uIndex]);
					}
					*this = std::move(vecTemp);
				}
			} else {
				Vector vecTemp;
				const auto uCapacity = GetCapacity();
				vecTemp.Reserve(uCapacity);
				for(std::size_t uIndex = 0; uIndex < uOffset; ++uIndex){
					vecTemp.UncheckedPush(x_pStorage[uIndex]);
				}
				for(auto itCur = itBegin; itCur != itEnd; ++itCur){
					vecTemp.Push(*itCur);
				}
				for(std::size_t uIndex = uOffset; uIndex < x_uSize; ++uIndex){
					vecTemp.Push(x_pStorage[uIndex]);
				}
				*this = std::move(vecTemp);
			}
		}

		return x_pStorage + uOffset;
	}
	Element *Insert(const Element *pPos, std::initializer_list<Element> ilElements){
		return Insert(pPos, ilElements.begin(), ilElements.end());
	}

	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(std::is_nothrow_move_constructible<Element>::value) {
		std::size_t uOffsetBegin, uOffsetEnd;
		if(pBegin){
			uOffsetBegin = static_cast<std::size_t>(pBegin - x_pStorage);
		} else {
			uOffsetBegin = x_uSize;
		}
		if(pEnd){
			uOffsetEnd = static_cast<std::size_t>(pEnd - x_pStorage);
		} else {
			uOffsetEnd = x_uSize;
		}

		if(uOffsetBegin != uOffsetEnd){
			if(uOffsetEnd == x_uSize){
				const auto uDeltaSize = uOffsetEnd - uOffsetBegin;
				Pop(uDeltaSize);
			} else if(std::is_nothrow_move_constructible<Element>::value){
				const auto uDeltaSize = uOffsetEnd - uOffsetBegin;
				for(std::size_t uIndex = uOffsetBegin; uIndex < uOffsetEnd; ++uIndex){
					Destruct(x_pStorage + uIndex);
				}
				for(std::size_t uIndex = uOffsetEnd; uIndex < x_uSize; ++uIndex){
					Construct(x_pStorage + uIndex - uDeltaSize, std::move(x_pStorage[uIndex]));
					Destruct(x_pStorage + uIndex);
				}
				x_uSize -= uDeltaSize;
			} else {
				Vector vecTemp;
				const auto uCapacity = GetCapacity();
				vecTemp.Reserve(uCapacity);
				for(std::size_t uIndex = 0; uIndex < uOffsetBegin; ++uIndex){
					vecTemp.UncheckedPush(x_pStorage[uIndex]);
				}
				for(std::size_t uIndex = uOffsetEnd; uIndex < x_uSize; ++uIndex){
					vecTemp.UncheckedPush(x_pStorage[uIndex]);
				}
				*this = std::move(vecTemp);
			}
		}

		return x_pStorage + uOffsetBegin;
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(std::declval<Vector &>().Erase(pPos, pPos))) {
		MCF_DEBUG_CHECK(pPos);

		return Erase(pPos, pPos + 1);
	}

public:
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

	friend void swap(Vector &vSelf, Vector &vOther) noexcept {
		vSelf.Swap(vOther);
	}

	friend decltype(auto) begin(const Vector &vOther) noexcept {
		return vOther.EnumerateFirst();
	}
	friend decltype(auto) begin(Vector &vOther) noexcept {
		return vOther.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const Vector &vOther) noexcept {
		return begin(vOther);
	}
	friend decltype(auto) end(const Vector &vOther) noexcept {
		return vOther.EnumerateSingular();
	}
	friend decltype(auto) end(Vector &vOther) noexcept {
		return vOther.EnumerateSingular();
	}
	friend decltype(auto) cend(const Vector &vOther) noexcept {
		return end(vOther);
	}
};

}

#endif
