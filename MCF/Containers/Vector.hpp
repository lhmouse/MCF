// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_VECTOR_HPP_
#define MCF_CONTAINERS_VECTOR_HPP_

#include "_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class Vector {
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
	Vector(std::initializer_list<ElementT> rhs)
		: Vector()
	{
		Append(rhs.begin(), rhs.end());
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

public:
	// 容器需求。
	using ElementType = ElementT;

	struct AdvanceOnce {
		void operator()(const Vector &v, const ElementType *&p) noexcept {
			if(p + 1 == v.GetEnd()){
				p = nullptr;
			} else {
				++p;
			}
		}
		void operator()(Vector &v, ElementType *&p) noexcept {
			if(p + 1 == v.GetEnd()){
				p = nullptr;
			} else {
				++p;
			}
		}
	};
	struct RetreatOnce {
		void operator()(const Vector &v, const ElementType *&p) noexcept {
			if(p == v.GetBegin()){
				p = nullptr;
			} else {
				--p;
			}
		}
		void operator()(Vector &v, ElementType *&p) noexcept {
			if(p == v.GetBegin()){
				p = nullptr;
			} else {
				--p;
			}
		}
	};

	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <Vector, AdvanceOnce, RetreatOnce>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <Vector, AdvanceOnce, RetreatOnce>;

	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear() noexcept {
		Pop(x_uSize);
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		const auto pBegin = GetBegin();
		if(pBegin == GetEnd()){
			return ConstEnumerator(*this, nullptr);
		} else {
			return ConstEnumerator(*this, pBegin);
		}
	}
	Enumerator EnumerateFirst() noexcept {
		auto pBegin = GetBegin();
		if(pBegin == GetEnd()){
			return Enumerator(*this, nullptr);
		} else {
			return Enumerator(*this, pBegin);
		}
	}

	ConstEnumerator EnumerateLast() const noexcept {
		const auto pEnd = GetEnd();
		if(GetBegin() == pEnd){
			return ConstEnumerator(*this, nullptr);
		} else {
			return ConstEnumerator(*this, pEnd - 1);
		}
	}
	Enumerator EnumerateLast() noexcept {
		auto pEnd = GetEnd();
		if(GetBegin() == pEnd){
			return Enumerator(*this, nullptr);
		} else {
			return Enumerator(*this, pEnd - 1);
		}
	}

	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}

	void Swap(Vector &rhs) noexcept {
		std::swap(x_pStorage,  rhs.x_pStorage);
		std::swap(x_uSize,     rhs.x_uSize);
		std::swap(x_uCapacity, rhs.x_uCapacity);
	}

	// Vector 需求。
	const ElementT *GetData() const noexcept {
		return static_cast<const ElementT *>(x_pStorage);
	}
	ElementT *GetData() noexcept {
		return static_cast<ElementT *>(x_pStorage);
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	std::size_t GetCapacity() noexcept {
		return x_uCapacity;
	}

	const ElementT *GetBegin() const noexcept {
		return GetData();
	}
	ElementT *GetBegin() noexcept {
		return GetData();
	}
	const ElementT *GetEnd() const noexcept {
		return GetData() + x_uSize;
	}
	ElementT *GetEnd() noexcept {
		return GetData() + x_uSize;
	}

	const ElementT &Get(std::size_t uIndex) const {
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, __PRETTY_FUNCTION__);
		}
		return GetData()[uIndex];
	}
	ElementT &Get(std::size_t uIndex){
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, __PRETTY_FUNCTION__);
		}
		return GetData()[uIndex];
	}
	const ElementT &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}
	ElementT &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}

	template<typename ...ParamsT>
	void Resize(std::size_t uSize, ParamsT &&...vParams){
		if(uSize > x_uSize){
			Append(uSize - x_uSize, std::forward<ParamsT>(vParams)...);
		} else {
			Pop(x_uSize - uSize);
		}
	}
	template<typename ...ParamsT>
	ElementT *ResizeMore(std::size_t uDeltaSize, ParamsT &&...vParams){
		const auto uOldSize = x_uSize;
		Append(uDeltaSize - x_uSize, std::forward<ParamsT>(vParams)...);
		return GetData() + uOldSize;
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
		const std::size_t uBytesToAlloc = sizeof(ElementT) * uElementsToAlloc;
		if(uBytesToAlloc / sizeof(ElementT) != uElementsToAlloc){
			throw std::bad_alloc();
		}

		const auto pOldStorage = static_cast<ElementT *>(x_pStorage);
		std::size_t uCopied = 0;
		const auto pNewStorage = static_cast<ElementT *>(::operator new[](uBytesToAlloc));
		try {
			while(uCopied < x_uSize){
				Construct(pNewStorage + uCopied, std::move_if_noexcept(pOldStorage[uCopied]));
				++uCopied;
			}
		} catch(...){
			while(uCopied != 0){
				--uCopied;
				Destruct(pNewStorage + uCopied);
			}
			::operator delete[](pNewStorage);
			throw;
		}
		while(uCopied != 0){
			--uCopied;
			Destruct(pOldStorage + uCopied);
		}
		::operator delete[](pOldStorage);

		x_pStorage  = pNewStorage;
		x_uCapacity = uElementsToAlloc;
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = x_uSize;
		const auto uNewCapacity = uOldSize + uDeltaCapacity;
		if(uNewCapacity < uOldSize){
			throw std::bad_alloc();
		}
		Reserve(uNewCapacity);
	}

	template<typename ...ParamsT>
	void Push(ParamsT &&...vParams){
		ReserveMore(1);
		UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	void UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - x_uSize > 0);

		DefaultConstruct(GetData() + x_uSize, std::forward<ParamsT>(vParams)...);
		++x_uSize;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= x_uSize);

		for(std::size_t i = 0; i < uCount; ++i){
			Destruct(GetData() + x_uSize - i - 1);
		}
		x_uSize -= uCount;
	}

	template<typename ...ParamsT>
	void Append(std::size_t uSize, const ParamsT &...vParams){
		const auto uOldSize = x_uSize;
		ReserveMore(uSize);
		try {
			for(std::size_t i = 0; i < uSize; ++i){
				UncheckedPush(vParams...);
			}
		} catch(...){
			Pop(x_uSize - uOldSize);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;
		const auto uOldSize = x_uSize;
		if(kHasDeltaSizeHint){
			ReserveMore(static_cast<std::size_t>(std::distance(itBegin, itEnd)));
		}
		try {
			if(kHasDeltaSizeHint){
				for(auto it = itBegin; it != itEnd; ++it){
					UncheckedPush(*it);
				}
			} else {
				for(auto it = itBegin; it != itEnd; ++it){
					Push(*it);
				}
			}
		} catch(...){
			Pop(x_uSize - uOldSize);
			throw;
		}
	}

	const ElementT &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	ElementT &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
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
decltype(auto) end(const Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) end(Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}

}

#endif
