// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_LIST_HPP_
#define MCF_CONTAINERS_LIST_HPP_

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
class List {
private:
	struct X_Node {
		alignas(ElementT) char aStorage[sizeof(ElementT)];
		X_Node *pPrev;
		X_Node *pNext;
	};

private:
	X_Node *x_pFirst;
	X_Node *x_pLast;

public:
	constexpr List() noexcept
		: x_pFirst(nullptr), x_pLast(nullptr)
	{
	}
	template<typename ...ParamsT>
	explicit List(std::size_t uSize, const ParamsT &...vParams)
		: List()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	List(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: List()
	{
		Append(itBegin, itEnd);
	}
	List(std::initializer_list<ElementT> rhs)
		: List()
	{
		Append(rhs.begin(), rhs.end());
	}
	List(const List &rhs)
		: List()
	{
		for(auto pElem = rhs.GetFirst(); pElem; pElem = rhs.GetNext(pElem)){
			Push(*pElem);
		}
	}
	List(List &&rhs) noexcept
		: List()
	{
		rhs.Swap(*this);
	}
	List &operator=(const List &rhs){
		List(rhs).Swap(*this);
		return *this;
	}
	List &operator=(List &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~List(){
		Clear();
	}

public:
	// 容器需求。
	using ElementType     = ElementT;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <List>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <List>;

	bool IsEmpty() const noexcept {
		return !x_pFirst;
	}
	void Clear() noexcept {
		auto pNode = x_pLast;
		while(pNode){
			const auto pPrev = pNode->pPrev;
			Destruct(static_cast<ElementT *>(static_cast<void *>(pNode->aStorage)));
			::delete pNode;
			pNode = pPrev;
		}

		x_pFirst = nullptr;
		x_pLast = nullptr;
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

	static const ElementType *GetNext(const ElementType *pPos) noexcept {
		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pNext;
		if(!pNode){
			return nullptr;
		}
		return static_cast<const ElementT *>(static_cast<void *>(pNode->aStorage));
	}
	static ElementType *GetNext(ElementType *pPos) noexcept {
		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pNext;
		if(!pNode){
			return nullptr;
		}
		return static_cast<ElementT *>(static_cast<void *>(pNode->aStorage));
	}

	static const ElementType *GetPrev(const ElementType *pPos) noexcept {
		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pPrev;
		if(!pNode){
			return nullptr;
		}
		return static_cast<const ElementT *>(static_cast<void *>(pNode->aStorage));
	}
	static ElementType *GetPrev(ElementType *pPos) noexcept {
		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pPrev;
		if(!pNode){
			return nullptr;
		}
		return static_cast<ElementT *>(static_cast<void *>(pNode->aStorage));
	}

	void Swap(List &rhs) noexcept {
		std::swap(x_pFirst, rhs.x_pFirst);
		std::swap(x_pLast,  rhs.x_pLast);
	}

	// List 需求。
	std::size_t CountElements() const noexcept {
		std::size_t uCount = 0;
		for(auto pElem = GetFirst(); pElem; pElem = GetNext(pElem)){
			++uCount;
		}
		return uCount;
	}

	const ElementType *GetFirst() const noexcept {
		const auto pNode = x_pFirst;
		if(!pNode){
			return nullptr;
		}
		return static_cast<const ElementT *>(static_cast<void *>(pNode->aStorage));
	}
	ElementType *GetFirst() noexcept {
		const auto pNode = x_pFirst;
		if(!pNode){
			return nullptr;
		}
		return static_cast<ElementT *>(static_cast<void *>(pNode->aStorage));
	}

	const ElementType *GetLast() const noexcept {
		const auto pNode = x_pLast;
		if(!pNode){
			return nullptr;
		}
		return static_cast<const ElementT *>(static_cast<void *>(pNode->aStorage));
	}
	ElementType *GetLast() noexcept {
		const auto pNode = x_pLast;
		if(!pNode){
			return nullptr;
		}
		return static_cast<ElementT *>(static_cast<void *>(pNode->aStorage));
	}

	template<typename ...ParamsT>
	void Push(ParamsT &&...vParams){
		const auto pNode = ::new X_Node;
		try {
			DefaultConstruct(static_cast<ElementT *>(static_cast<void *>(pNode->aStorage)), std::forward<ParamsT>(vParams)...);
		} catch(...){
			::delete pNode;
			throw;
		}
		pNode->pPrev = x_pLast;
		pNode->pNext = nullptr;

		if(x_pLast){
			x_pLast->pNext = pNode;
		} else {
			x_pFirst = pNode;
		}
		x_pLast = pNode;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= CountElements());

		auto pNode = x_pLast;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pPrev = pNode->pPrev;
			Destruct(static_cast<ElementT *>(static_cast<void *>(pNode->aStorage)));
			::delete pNode;
			pNode = pPrev;
		}
		if(pNode){
			pNode->pNext = nullptr;
		} else {
			x_pFirst = nullptr;
		}
		x_pLast = pNode;
	}

	template<typename ...ParamsT>
	void Unshift(ParamsT &&...vParams){
		const auto pNode = ::new X_Node;
		try {
			DefaultConstruct(static_cast<ElementT *>(static_cast<void *>(pNode->aStorage)), std::forward<ParamsT>(vParams)...);
		} catch(...){
			::delete pNode;
			throw;
		}
		pNode->pPrev = nullptr;
		pNode->pNext = x_pFirst;

		if(x_pFirst){
			x_pFirst->pPrev = pNode;
		} else {
			x_pLast = pNode;
		}
		x_pFirst = pNode;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= CountElements());

		auto pNode = x_pFirst;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pNext = pNode->pNext;
			Destruct(static_cast<ElementT *>(static_cast<void *>(pNode->aStorage)));
			::delete pNode;
			pNode = pNext;
		}
		if(pNode){
			pNode->pPrev = nullptr;
		} else {
			x_pLast = nullptr;
		}
		x_pFirst = pNode;
	}

	template<typename ...ParamsT>
	void Append(std::size_t uDeltaSize, const ParamsT &...vParams){
		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				Push(vParams...);
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
		std::size_t uElementsPushed = 0;
		try {
			for(auto it = itBegin; it != itEnd; ++it){
				Push(*it);
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}

	template<typename ...ParamsT>
	void Prepend(std::size_t uDeltaSize, const ParamsT &...vParams){
		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				Unshift(vParams...);
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
	void Prepend(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		std::size_t uElementsPushed = 0;
		try {
			for(auto it = itBegin; it != itEnd; ++it){
				Unshift(*it);
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}

	template<typename ...ParamsT>
	void Insert(const ElementT *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		List lstNew;
		for(std::size_t i = 0; i < uDeltaSize; ++i){
			lstNew.Push(vParams...);
		}
		Splice(pPos, lstNew);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Insert(const ElementT *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		List lstNew;
		for(auto it = itBegin; it != itEnd; ++it){
			lstNew.Push(*it);
		}
		Splice(pPos, lstNew);
	}

	void Erase(const ElementT *pBegin, const ElementT *pEnd) noexcept {
		List().Splice(nullptr, *this, pBegin, pEnd);
	}
	void Erase(const ElementT *pPos) noexcept {
		List().Splice(nullptr, *this, pPos);
	}

	void Splice(const ElementT *pInsert, List &lstSrc) noexcept {
		Splice(pInsert, lstSrc, lstSrc.GetFirst(), nullptr);
	}
	void Splice(const ElementT *pInsert, List &lstSrc, const ElementT *pPos) noexcept {
		ASSERT(pPos);

		Splice(pInsert, lstSrc, pPos, lstSrc.GetNext(pPos));
	}
	void Splice(const ElementT *pInsert, List &lstSrc, const ElementT *pBegin, const ElementT *pEnd) noexcept {
		ASSERT(pBegin);

		const auto pInsertNode = reinterpret_cast<X_Node *>(const_cast<ElementT *>(pInsert));
		const auto pBeginNode = reinterpret_cast<X_Node *>(const_cast<ElementT *>(pBegin));
		const auto pEndNode = reinterpret_cast<X_Node *>(const_cast<ElementT *>(pEnd));

		if(pBeginNode == pEndNode){
			return;
		}

		const auto pNodeBeforeBegin = pBeginNode->pPrev;
		const auto pNodeBeforeEnd = pEndNode ? pEndNode->pPrev : lstSrc.x_pLast;

		if(pNodeBeforeBegin){
			pNodeBeforeBegin->pNext = pEndNode;
		} else {
			lstSrc.x_pFirst = pEndNode;
		}
		if(pEndNode){
			pEndNode->pPrev = pNodeBeforeBegin;
		} else {
			lstSrc.x_pLast = pNodeBeforeBegin;
		}

		X_Node *pNodeBeforeInsert;
		if(pInsertNode){
			pNodeBeforeInsert = pInsertNode->pPrev;
			pInsertNode->pPrev = pNodeBeforeEnd;
		} else {
			pNodeBeforeInsert = x_pLast;
			x_pLast = pNodeBeforeEnd;
		}
		if(pNodeBeforeInsert){
			pNodeBeforeInsert->pNext = pBeginNode;
		} else {
			x_pFirst = pBeginNode;
		}
		pBeginNode->pPrev = pNodeBeforeInsert;
		pNodeBeforeEnd->pNext = pInsertNode;
	}

	void Splice(const ElementT *pInsert, List &&lstSrc) noexcept {
		Splice(pInsert, lstSrc);
	}
	void Splice(const ElementT *pInsert, List &&lstSrc, const ElementT *pPos) noexcept {
		Splice(pInsert, lstSrc, pPos);
	}
	void Splice(const ElementT *pInsert, List &&lstSrc, const ElementT *pBegin, const ElementT *pEnd) noexcept {
		Splice(pInsert, lstSrc, pBegin, pEnd);
	}
};

template<typename ElementT>
void swap(List<ElementT> &lhs, List<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ElementT>
decltype(auto) begin(const List<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT>
decltype(auto) begin(List<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}

template<typename ElementT>
decltype(auto) end(const List<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) end(List<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}

}

#endif
