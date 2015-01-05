// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_VECTOR_HPP_
#define MCF_CONTAINERS_VECTOR_HPP_

#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/Assert.hpp"
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <memory>
#include <cstddef>

namespace MCF {

template<class ElementT>
class Vector {
private:
	ElementT *xm_pBegin;
	ElementT *xm_pEnd;
	ElementT *xm_pEndOfStor;

public:
	constexpr Vector() noexcept
		: xm_pBegin(nullptr), xm_pEnd(nullptr), xm_pEndOfStor(nullptr)
	{
	}
	template<typename ...ParamsT>
	explicit Vector(std::size_t uCount, const ParamsT &...vParams)
		: Vector()
	{
		AppendFill(uCount, vParams...);
	}
	template<class IteratorT>
	Vector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: Vector()
	{
		AppendCopy(itBegin, itEnd);
	}
	template<class IteratorT>
	Vector(IteratorT itBegin, std::size_t uCount)
		: Vector()
	{
		AppendCopy(itBegin, uCount);
	}
	explicit Vector(std::initializer_list<ElementT> rhs)
		: Vector()
	{
		AppendCopy(rhs.begin(), rhs.size());
	}
	Vector(const Vector &rhs)
		: Vector()
	{
		AppendCopy(rhs.GetBegin(), rhs.GetEnd());
	}
	Vector(Vector &&rhs) noexcept
		: Vector()
	{
		Swap(rhs);
	}
	Vector &operator=(std::initializer_list<ElementT> rhs){
		Vector(rhs).Swap(*this);
		return *this;
	}
	Vector &operator=(const Vector &rhs){
		if(&rhs != this){
			Vector(rhs).Swap(*this);
		}
		return *this;
	}
	Vector &operator=(Vector &&rhs) noexcept {
		Swap(rhs);
		return *this;
	}
	~Vector(){
		Clear(true);
	}

public:
	const ElementT *GetBegin() const noexcept {
		return xm_pBegin;
	}
	ElementT *GetBegin() noexcept {
		return xm_pBegin;
	}
	const ElementT *GetEnd() const noexcept {
		return xm_pEnd;
	}
	ElementT *GetEnd() noexcept {
		return xm_pEnd;
	}

	const ElementT &GetFront() const noexcept {
		ASSERT(!IsEmpty());
		return xm_pBegin[0];
	}
	ElementT &GetFront() noexcept {
		ASSERT(!IsEmpty());
		return xm_pBegin[0];
	}
	const ElementT &GetBack() const noexcept {
		ASSERT(!IsEmpty());
		return xm_pEnd[-1];
	}
	ElementT &GetBack() noexcept {
		ASSERT(!IsEmpty());
		return xm_pEnd[-1];
	}

	const ElementT *GetData() const noexcept {
		return GetBegin();
	}
	ElementT *GetData() noexcept {
		return GetBegin();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	template<typename ...ParamsT>
	void Resize(std::size_t uNewSize, const ParamsT &...vParams){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			AppendFill(uNewSize - uOldSize, vParams...);
		} else if(uNewSize < uOldSize){
			Truncate(uOldSize - uNewSize);
		}
	}
	template<typename ...ParamsT>
	ElementT *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		AppendFill(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetBegin();
	}
	void Clear(bool bDeallocateBuffer = false) noexcept {
		Truncate(GetSize());

		if(bDeallocateBuffer){
			::operator delete[](xm_pBegin);
			xm_pBegin = nullptr;
			xm_pEnd = nullptr;
			xm_pEndOfStor = nullptr;
		}
	}

	std::size_t GetCapacity() const noexcept {
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity > uOldCapacity){
			auto uSizeToAlloc = uOldCapacity + 1;
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0x0F) & (std::size_t)-0x10;
			if(uSizeToAlloc < uNewCapacity){
				uSizeToAlloc = uNewCapacity;
			}

			const auto pOldBegin = GetBegin();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (ElementT *)::operator new[](sizeof(ElementT) * uSizeToAlloc);
			auto pRead = pOldBegin;
			auto pWrite = pNewBegin;
			try {
				while(pRead != pOldEnd){
					Construct(pWrite, std::move_if_noexcept(*pRead));
					++pWrite;
					++pRead;
				}
			} catch(...){
				while(pWrite != pNewBegin){
					--pWrite;
					Destruct(pWrite);
				}
				::operator delete[](pNewBegin);
				throw;
			}
			while(pRead != pOldBegin){
				--pRead;
				Destruct(pRead);
			}

			::operator delete[](xm_pBegin);
			xm_pBegin = pNewBegin;
			xm_pEnd = pWrite;
			xm_pEndOfStor = xm_pBegin + uSizeToAlloc;
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		Reserve(GetSize() + uDeltaCapacity);
	}

	const ElementT &GetAt(std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");
		return xm_pBegin[uIndex];
	}
	ElementT &GetAt(std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");
		return xm_pBegin[uIndex];
	}

	template<typename ...ParamsT>
	ElementT *UncheckedPush(ParamsT &&...vParams)
		noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		DefaultConstruct(xm_pEnd, std::forward<ParamsT>(vParams)...);
		return xm_pEnd++;
	}
	template<typename ...ParamsT>
	ElementT *Push(ParamsT &&...vParams){
		Reserve(GetSize() + 1);
		return UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		Destruct(--xm_pEnd);
	}

	template<typename ...ParamsT>
	void UncheckedAppendFill(std::size_t uCount, const ParamsT &...vParams)
		noexcept(std::is_nothrow_constructible<ElementT, const ParamsT &...>::value)
	{
		std::size_t i = 0;
		try {
			while(i < uCount){
				UncheckedPush(vParams...);
				++i;
			}
		} catch(...){
			while(i > 0){
				Pop();
				--i;
			}
			throw;
		}
	}
	template<class IteratorT>
	void UncheckedAppendCopy(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		noexcept(std::is_nothrow_constructible<ElementT, decltype((*std::declval<IteratorT>()))>::value)
	{
		std::size_t i = 0;
		try {
			while(itBegin != itEnd){
				UncheckedPush(*itBegin);
				++itBegin;
				++i;
			}
		} catch(...){
			while(i > 0){
				Pop();
				--i;
			}
			throw;
		}
	}
	template<class IteratorT>
	void UncheckedAppendCopy(IteratorT itBegin, std::size_t uCount)
		noexcept(std::is_nothrow_constructible<ElementT, decltype((*std::declval<IteratorT>()))>::value)
	{
		std::size_t i = 0;
		try {
			while(i < uCount){
				UncheckedPush(*itBegin);
				++itBegin;
				++i;
			}
		} catch(...){
			while(i > 0){
				Pop();
				--i;
			}
			throw;
		}
	}
	template<typename ...ParamsT>
	void AppendFill(std::size_t uCount, const ParamsT &...vParams){
		Reserve(GetSize() + uCount);
		UncheckedAppendFill(uCount, vParams...);
	}
	template<class IteratorT>
	void AppendCopy(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		if(std::is_same<typename std::iterator_traits<IteratorT>::iterator_category,
			std::random_access_iterator_tag>::value)
		{
			ReserveMore((std::size_t)std::distance(itBegin, itEnd));
		}
		while(itBegin != itEnd){
			Push(*itBegin);
			++itBegin;
		}
	}
	template<class IteratorT>
	void AppendCopy(IteratorT itBegin, std::size_t uCount){
		ReserveMore(uCount);
		UncheckedAppendCopy(std::move(itBegin), uCount);
	}
	void Truncate(std::size_t uCount) noexcept {
		for(std::size_t i = 0; i < uCount; ++i){
			Pop();
		}
	}

	void Swap(Vector &rhs) noexcept {
		std::swap(xm_pBegin, rhs.xm_pBegin);
		std::swap(xm_pEnd, rhs.xm_pEnd);
		std::swap(xm_pEndOfStor, rhs.xm_pEndOfStor);
	}

public:
	using value_type = ElementT;

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Push(std::forward<Param>(vParam));
	}

public:
	explicit operator const ElementT *() const noexcept {
		return GetData();
	}
	explicit operator ElementT *() noexcept {
		return GetData();
	}

	const ElementT &operator[](std::size_t uIndex) const noexcept {
		return GetAt(uIndex);
	}
	ElementT &operator[](std::size_t uIndex) noexcept {
		return GetAt(uIndex);
	}
};

template<class ElementT>
void swap(Vector<ElementT> &lhs, Vector<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<class ElementT>
auto begin(const Vector<ElementT> &lhs) noexcept {
	return lhs.GetBegin();
}
template<class ElementT>
auto begin(Vector<ElementT> &lhs) noexcept {
	return lhs.GetBegin();
}
template<class ElementT>
auto cbegin(const Vector<ElementT> &lhs) noexcept {
	return lhs.GetBegin();
}
template<class ElementT>
auto end(const Vector<ElementT> &lhs) noexcept {
	return lhs.GetEnd();
}
template<class ElementT>
auto end(Vector<ElementT> &lhs) noexcept {
	return lhs.GetEnd();
}
template<class ElementT>
auto cend(const Vector<ElementT> &lhs) noexcept {
	return lhs.GetEnd();
}

}

#endif
