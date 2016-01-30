// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_VECTOR_HPP_
#define MCF_CONTAINERS_VECTOR_HPP_

#include "_Enumerator.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/DeclVal.hpp"
#include "../Core/Exception.hpp"
#include "../Core/ArrayView.hpp"
#include <utility>
#include <new>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class Vector {
public:
	// 容器需求。
	using Element         = ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <Vector>;
	using Enumerator      = Impl_Enumerator::Enumerator      <Vector>;

private:
	Element *x_pStorage;
	std::size_t x_uSize;
	std::size_t x_uCapacity;

public:
	constexpr Vector() noexcept
		: x_pStorage(nullptr), x_uSize(0), x_uCapacity(0)
	{
	}
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
	Vector(std::initializer_list<Element> rhs)
		: Vector()
	{
		Append(rhs);
	}
	Vector(const Vector &rhs)
		: Vector()
	{
		Append(rhs.GetBegin(), rhs.GetEnd());
	}
	Vector(Vector &&rhs) noexcept
		: Vector()
	{
		rhs.Swap(*this);
	}
	Vector &operator=(const Vector &rhs){
		Vector(rhs).Swap(*this);
		return *this;
	}
	Vector &operator=(Vector &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~Vector(){
		Clear();
		::operator delete[](x_pStorage);
	}

private:
	void X_PrepareForInsertion(std::size_t uPos, std::size_t uDeltaSize){
		ASSERT(std::is_nothrow_move_constructible<Element>::value);
		ASSERT(!IsEmpty());
		ASSERT(uPos <= x_uSize);

		ReserveMore(uDeltaSize);
		for(std::size_t i = x_uSize; i > uPos; --i){
			Construct(x_pStorage + i - 1 + uDeltaSize, std::move(x_pStorage[i - 1]));
			Destruct(x_pStorage + i - 1);
		}
	}
	void X_UndoPreparation(std::size_t uPos, std::size_t uDeltaSize) noexcept {
		ASSERT(std::is_nothrow_move_constructible<Element>::value);
		ASSERT(!IsEmpty());
		ASSERT(uPos <= x_uSize);
		ASSERT(uDeltaSize <= x_uSize - uPos);

		for(std::size_t i = uPos; i < x_uSize; ++i){
			Construct(x_pStorage + i, std::move(x_pStorage[i + uDeltaSize]));
			Destruct(x_pStorage + i + uDeltaSize);
		}
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
			for(std::size_t i = 0; i < x_uSize; ++i){
				*itOutput = std::move(x_pStorage[i]);
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

	void Swap(Vector &rhs) noexcept {
		using std::swap;
		swap(x_pStorage,  rhs.x_pStorage);
		swap(x_uSize,     rhs.x_uSize);
		swap(x_uCapacity, rhs.x_uCapacity);
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
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "Vector: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "Vector: Subscript out of range"_rcs);
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
		const auto uOldCapacity = x_uCapacity;
		if(uNewCapacity <= uOldCapacity){
			return;
		}

		auto uElementsToAlloc = uOldCapacity + 1;
		uElementsToAlloc += (uElementsToAlloc >> 1);
		uElementsToAlloc = (uElementsToAlloc + 0x0F) & (std::size_t)-0x10;
		if(uElementsToAlloc < uNewCapacity){
			uElementsToAlloc = uNewCapacity;
		}
		const std::size_t uBytesToAlloc = sizeof(Element) * uElementsToAlloc;
		if(uBytesToAlloc / sizeof(Element) != uElementsToAlloc){
			throw std::bad_array_new_length();
		}

		const auto pNewStorage = static_cast<Element *>(::operator new[](uBytesToAlloc));
		const auto pOldStorage = x_pStorage;
		auto pWrite = pNewStorage;
		try {
			for(std::size_t i = 0; i < x_uSize; ++i){
				Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
				++pWrite;
			}
		} catch(...){
			while(pWrite != pNewStorage){
				--pWrite;
				Destruct(pWrite);
			}
			::operator delete[](pNewStorage);
			throw;
		}
		for(std::size_t i = x_uSize; i > 0; --i){
			Destruct(pOldStorage + i - 1);
		}
		::operator delete[](pOldStorage);

		x_pStorage  = pNewStorage;
		x_uCapacity = uElementsToAlloc;
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
			const auto uSize = GetSize();
			auto uNewCapacity = uSize + 1;
			if(uNewCapacity < uSize){
				throw std::bad_array_new_length();
			}
			const auto uCapacity = GetCapacity();
			if(uNewCapacity < uCapacity){
				uNewCapacity = uCapacity;
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(std::size_t i = 0; i < uOffset; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
			}
			vecTemp.UncheckedPush(std::forward<ParamsT>(vParams)...);
			for(std::size_t i = uOffset; i < x_uSize; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
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

		if(std::is_nothrow_move_constructible<Element>::value){
			X_PrepareForInsertion(uOffset, uDeltaSize);
			auto uWrite = uOffset;
			try {
				for(std::size_t i = 0; i < uDeltaSize; ++i){
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
			const auto uSize = GetSize();
			auto uNewCapacity = uSize + uDeltaSize;
			if(uNewCapacity < uSize){
				throw std::bad_array_new_length();
			}
			const auto uCapacity = GetCapacity();
			if(uNewCapacity < uCapacity){
				uNewCapacity = uCapacity;
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(std::size_t i = 0; i < uOffset; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
			}
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				vecTemp.UncheckedPush(vParams...);
			}
			for(std::size_t i = uOffset; i < x_uSize; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
			}
			*this = std::move(vecTemp);
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

		if(std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			if(std::is_nothrow_move_constructible<Element>::value){
				X_PrepareForInsertion(uOffset, uDeltaSize);
				auto uWrite = uOffset;
				try {
					for(auto it = itBegin; it != itEnd; ++it){
						DefaultConstruct(x_pStorage + uWrite, *it);
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
				const auto uSize = GetSize();
				auto uNewCapacity = uSize + uDeltaSize;
				if(uNewCapacity < uSize){
					throw std::bad_array_new_length();
				}
				const auto uCapacity = GetCapacity();
				if(uNewCapacity < uCapacity){
					uNewCapacity = uCapacity;
				}
				Vector vecTemp;
				vecTemp.Reserve(uNewCapacity);
				for(std::size_t i = 0; i < uOffset; ++i){
					vecTemp.UncheckedPush(x_pStorage[i]);
				}
				for(auto it = itBegin; it != itEnd; ++it){
					vecTemp.UncheckedPush(*it);
				}
				for(std::size_t i = uOffset; i < x_uSize; ++i){
					vecTemp.UncheckedPush(x_pStorage[i]);
				}
				*this = std::move(vecTemp);
			}
		} else {
			Vector vecTemp;
			const auto uCapacity = GetCapacity();
			vecTemp.Reserve(uCapacity);
			for(std::size_t i = 0; i < uOffset; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
			}
			for(auto it = itBegin; it != itEnd; ++it){
				vecTemp.Push(*it);
			}
			for(std::size_t i = uOffset; i < x_uSize; ++i){
				vecTemp.Push(x_pStorage[i]);
			}
			*this = std::move(vecTemp);
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

		if(std::is_nothrow_move_constructible<Element>::value){
			const auto uDeltaSize = uOffsetEnd - uOffsetBegin;
			for(std::size_t i = uOffsetBegin; i < uOffsetEnd; ++i){
				Destruct(x_pStorage + i);
			}
			for(std::size_t i = uOffsetEnd; i < x_uSize; ++i){
				Construct(x_pStorage + i - uDeltaSize, std::move(x_pStorage[i]));
				Destruct(x_pStorage + i);
			}
			x_uSize -= uDeltaSize;
		} else {
			Vector vecTemp;
			const auto uCapacity = GetCapacity();
			vecTemp.Reserve(uCapacity);
			for(std::size_t i = 0; i < uOffsetBegin; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
			}
			for(std::size_t i = uOffsetEnd; i < x_uSize; ++i){
				vecTemp.UncheckedPush(x_pStorage[i]);
			}
			*this = std::move(vecTemp);
		}

		return x_pStorage + uOffsetBegin;
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(DeclVal<Vector &>().Erase(pPos, pPos))) {
		ASSERT(pPos);

		return Erase(pPos, pPos + 1);
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

	friend void swap(Vector &lhs, Vector &rhs) noexcept {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const Vector &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) begin(Vector &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const Vector &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const Vector &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) end(Vector &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) cend(const Vector &rhs) noexcept {
		return end(rhs);
	}
};

}

#endif
