// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_VECTOR_HPP_
#define MCF_CONTAINERS_VECTOR_HPP_

#include "_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
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
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <Vector>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <Vector>;

private:
	void *x_pStorage;
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
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	Vector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: Vector()
	{
		Append(itBegin, itEnd);
	}
	Vector(std::initializer_list<Element> rhs)
		: Vector(rhs.begin(), rhs.end())
	{
	}
	Vector(const Vector &rhs)
		: Vector()
	{
		Reserve(rhs.GetSize());
		for(auto pElem = rhs.GetBegin(); pElem != rhs.GetEnd(); ++pElem){
			UncheckedPush(*pElem);
		}
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
		const auto pBegin = GetBegin();
		for(std::size_t i = x_uSize; i > uPos; --i){
			const auto pRead = pBegin + i - 1;
			Construct(pRead + uDeltaSize, std::move(pRead[0]));
			Destruct(pRead);
		}
	}
	void X_UndoPreparation(std::size_t uPos, std::size_t uDeltaSize) noexcept {
		ASSERT(std::is_nothrow_move_constructible<Element>::value);
		ASSERT(!IsEmpty());
		ASSERT(uPos <= x_uSize);
		ASSERT(uDeltaSize <= x_uSize - uPos);

		const auto pBegin = GetBegin();
		for(std::size_t i = uPos; i < x_uSize; ++i){
			const auto pWrite = pBegin + i;
			Construct(pWrite, std::move(pWrite[uDeltaSize]));
			Destruct(pWrite + uDeltaSize);
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
		return static_cast<const Element *>(x_pStorage);
	}
	Element *GetData() noexcept {
		return static_cast<Element *>(x_pStorage);
	}
	const Element *GetConstData() const noexcept {
		return GetData();
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	std::size_t GetCapacity() noexcept {
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
		return GetData() + x_uSize;
	}
	Element *GetEnd() noexcept {
		return GetData() + x_uSize;
	}
	const Element *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "Vector: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "Vector: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}

	template<typename ...ParamsT>
	void Resize(std::size_t uSize, const ParamsT &...vParams){
		if(uSize > x_uSize){
			Append(uSize - x_uSize, vParams...);
		} else {
			Pop(x_uSize - uSize);
		}
	}
	template<typename ...ParamsT>
	Element *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = x_uSize;
		const auto uNewSize = uOldSize + uDeltaSize;
		if(uNewSize < uOldSize){
			throw std::bad_array_new_length();
		}
		Append(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}
	template<typename ...ParamsT>
	std::pair<Element *, std::size_t> ResizeToCapacity(const ParamsT &...vParams){
		const auto uOldSize = x_uSize;
		const auto uNewSize = x_uCapacity;
		const auto uDeltaSize = uNewSize - uOldSize;
		Append(uDeltaSize, vParams...);
		return std::make_pair(GetData() + uOldSize, uDeltaSize);
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

		const auto pNewStorage = ::operator new[](uBytesToAlloc);
		const auto pOldStorage = x_pStorage;
		const auto pNewBegin = static_cast<Element *>(pNewStorage);
		const auto pOldBegin = static_cast<Element *>(pOldStorage);
		auto pWrite = pNewBegin;
		try {
			for(std::size_t i = 0; i < x_uSize; ++i){
				Construct(pWrite, std::move_if_noexcept(pOldBegin[i]));
				++pWrite;
			}
		} catch(...){
			while(pWrite != pNewBegin){
				--pWrite;
				Destruct(pWrite);
			}
			::operator delete[](pNewStorage);
			throw;
		}
		for(std::size_t i = x_uSize; i > 0; --i){
			Destruct(pOldBegin + i - 1);
		}
		::operator delete[](pOldStorage);

		x_pStorage  = pNewStorage;
		x_uCapacity = uElementsToAlloc;
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = x_uSize;
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
		ASSERT(x_uCapacity - x_uSize > 0);

		const auto pBegin = GetBegin();
		const auto pElem = pBegin + x_uSize;
		DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
		++x_uSize;

		return *pElem;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= x_uSize);

		const auto pBegin = GetBegin();
		for(std::size_t i = 0; i < uCount; ++i){
			Destruct(pBegin + x_uSize - i - 1);
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
	void Append(std::initializer_list<Element> ilElements){
		Append(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	Element *Emplace(const Element *pPos, ParamsT &&...vParams){
		if(!pPos || (pPos == GetEnd())){
			Push(std::forward<ParamsT>(vParams)...);
			return nullptr;
		}
		ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

		const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

		if(std::is_nothrow_move_constructible<Element>::value){
			X_PrepareForInsertion(uOffset, 1);
			const auto pBegin = GetBegin();
			try {
				DefaultConstruct(pBegin + uOffset, std::forward<ParamsT>(vParams)...);
			} catch(...){
				X_UndoPreparation(uOffset, 1);
				throw;
			}
			++x_uSize;
		} else {
			auto uNewCapacity = x_uSize + 1;
			if(uNewCapacity < x_uSize){
				throw std::bad_array_new_length();
			}
			if(uNewCapacity < x_uCapacity){
				uNewCapacity = x_uCapacity;
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			vecTemp.UncheckedPush(std::forward<ParamsT>(vParams)...);
			for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			*this = std::move(vecTemp);
		}

		return GetBegin() + uOffset;
	}

	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		if(!pPos || (pPos == GetEnd())){
			Append(uDeltaSize, vParams...);
			return nullptr;
		}
		ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

		const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

		if(std::is_nothrow_move_constructible<Element>::value){
			X_PrepareForInsertion(uOffset, uDeltaSize);
			const auto pWriteBegin = GetBegin() + uOffset;
			std::size_t uWrite = 0;
			try {
				for(std::size_t i = 0; i < uDeltaSize; ++i){
					DefaultConstruct(pWriteBegin + uWrite, vParams...);
					++uWrite;
				}
			} catch(...){
				while(uWrite != 0){
					--uWrite;
					Destruct(pWriteBegin + uWrite);
				}
				X_UndoPreparation(uWrite, uDeltaSize);
				throw;
			}
			x_uSize += uDeltaSize;
		} else {
			auto uNewCapacity = x_uSize + uDeltaSize;
			if(uNewCapacity < x_uSize){
				throw std::bad_array_new_length();
			}
			if(uNewCapacity < x_uCapacity){
				uNewCapacity = x_uCapacity;
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				vecTemp.UncheckedPush(vParams...);
			}
			for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			*this = std::move(vecTemp);
		}

		return GetBegin() + uOffset;
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	Element *Insert(const Element *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		if(!pPos || (pPos == GetEnd())){
			Append(itBegin, itEnd);
			return nullptr;
		}
		ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

		if(kHasDeltaSizeHint && std::is_nothrow_move_constructible<Element>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			X_PrepareForInsertion(uOffset, uDeltaSize);
			const auto pWriteBegin = GetBegin() + uOffset;
			std::size_t uWrite = 0;
			try {
				for(auto it = itBegin; it != itEnd; ++it){
					Construct(pWriteBegin + uWrite, *it);
					++uWrite;
				}
			} catch(...){
				while(uWrite != 0){
					--uWrite;
					Destruct(pWriteBegin + uWrite);
				}
				X_UndoPreparation(uWrite, uDeltaSize);
				throw;
			}
			x_uSize += uDeltaSize;
		} else {
			if(kHasDeltaSizeHint){
				const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
				auto uNewCapacity = x_uSize + uDeltaSize;
				if(uNewCapacity < x_uSize){
					throw std::bad_array_new_length();
				}
				if(uNewCapacity < x_uCapacity){
					uNewCapacity = x_uCapacity;
				}
				Vector vecTemp;
				vecTemp.Reserve(uNewCapacity);
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				for(auto it = itBegin; it != itEnd; ++it){
					vecTemp.UncheckedPush(*it);
				}
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				*this = std::move(vecTemp);
			} else {
				Vector vecTemp;
				vecTemp.Reserve(x_uCapacity);
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				for(auto it = itBegin; it != itEnd; ++it){
					vecTemp.Push(*it);
				}
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vecTemp.Push(*pCur);
				}
				*this = std::move(vecTemp);
			}
		}

		return GetBegin() + uOffset;
	}
	Element *Insert(const Element *pPos, std::initializer_list<Element> ilElements){
		return Insert(pPos, ilElements.begin(), ilElements.end());
	}

	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(std::is_nothrow_move_constructible<Element>::value) {
		if(pBegin == pEnd){
			return const_cast<Element *>(pEnd);
		}
		ASSERT(pBegin);

		if(!pEnd || (pEnd == GetEnd())){
			const auto uDeltaSize = static_cast<std::size_t>(GetEnd() - pBegin);

			Pop(uDeltaSize);
			return nullptr;
		}
		ASSERT((GetBegin() <= pBegin) && (pBegin <= pEnd) && (pEnd <= GetEnd()));

		const auto uOffset = static_cast<std::size_t>(pBegin - GetBegin());

		if(std::is_nothrow_move_constructible<Element>::value){
			const auto uDeltaSize = static_cast<std::size_t>(pEnd - pBegin);

			auto pWrite = const_cast<Element *>(pBegin);
			for(auto pCur = pWrite; pCur != pEnd; ++pCur){
				Destruct(pCur);
			}
			for(auto pCur = const_cast<Element *>(pEnd); pCur != GetEnd(); ++pCur){
				Construct(pWrite, std::move(*pCur));
				Destruct(pCur);
				++pWrite;
			}
			x_uSize -= uDeltaSize;
		} else {
			Vector vecTemp;
			vecTemp.Reserve(x_uCapacity);
			for(auto pCur = GetBegin(); pCur != pBegin; ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			for(auto pCur = pEnd; pCur != GetEnd(); ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			*this = std::move(vecTemp);
		}

		return GetBegin() + uOffset;
	}
	Element *Erase(const Element *pPos) noexcept(noexcept(std::declval<Vector &>().Erase(pPos, pPos))) {
		ASSERT(pPos);

		return Erase(pPos, pPos + 1);
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

	operator ArrayView<ElementT>() noexcept {
		return ArrayView<ElementT>(GetData(), GetSize());
	}
};

template<typename ElementT>
void swap(Vector<ElementT> &lhs, Vector<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ElementT>
decltype(auto) begin(const Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT>
decltype(auto) begin(Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT>
decltype(auto) cbegin(const Vector<ElementT> &rhs) noexcept {
	return begin(rhs);
}
template<typename ElementT>
decltype(auto) end(const Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) end(Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) cend(const Vector<ElementT> &rhs) noexcept {
	return end(rhs);
}

}

#endif
