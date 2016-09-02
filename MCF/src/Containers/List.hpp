// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_LIST_HPP_
#define MCF_CONTAINERS_LIST_HPP_

#include "../Core/_Enumerator.hpp"
#include "../Core/DefaultAllocator.hpp"
#include "../Core/Assert.hpp"
#include "../Core/AlignedStorage.hpp"
#include "../Core/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT, class AllocatorT = DefaultAllocator>
class List {
public:
	// 容器需求。
	using Element         = ElementT;
	using Allocator       = AllocatorT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <List>;
	using Enumerator      = Impl_Enumerator::Enumerator      <List>;

private:
	struct X_Node {
		X_Node *pPrev;
		X_Node *pNext;
		AlignedStorage<0, Element> vElement;
	};

public:
	enum : std::size_t {
		kNodeSize = sizeof(X_Node),
	};

private:
	static const Element *X_GetElementFromNode(const X_Node *pNode) noexcept {
		MCF_DEBUG_CHECK(pNode);

		return reinterpret_cast<const Element *>(reinterpret_cast<const char *>(pNode) + __builtin_offsetof(X_Node, vElement));
	}
	static Element *X_GetElementFromNode(X_Node *pNode) noexcept {
		MCF_DEBUG_CHECK(pNode);

		return reinterpret_cast<Element *>(reinterpret_cast<char *>(pNode) + __builtin_offsetof(X_Node, vElement));
	}
	static const X_Node *X_GetNodeFromElement(const Element *pElement) noexcept {
		MCF_DEBUG_CHECK(pElement);

		return reinterpret_cast<const X_Node *>(reinterpret_cast<const char *>(pElement) - __builtin_offsetof(X_Node, vElement));
	}
	static X_Node *X_GetNodeFromElement(Element *pElement) noexcept {
		MCF_DEBUG_CHECK(pElement);

		return reinterpret_cast<X_Node *>(reinterpret_cast<char *>(pElement) - __builtin_offsetof(X_Node, vElement));
	}

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
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	List(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: List()
	{
		Append(itBegin, itEnd);
	}
	List(std::initializer_list<Element> rhs)
		: List(rhs.begin(), rhs.end())
	{
	}
	List(const List &rhs)
		: List()
	{
		auto pNode = rhs.x_pFirst;
		while(pNode){
			const auto pNext = pNode->pNext;
			const auto pElement = X_GetElementFromNode(pNode);
			Push(*pElement);
			pNode = pNext;
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
	bool IsEmpty() const noexcept {
		return !x_pFirst;
	}
	void Clear() noexcept {
		auto pNode = x_pLast;
		while(pNode){
			const auto pPrev = pNode->pPrev;
			const auto pElement = X_GetElementFromNode(pNode);
			Destruct(pElement);
			Allocator()(const_cast<void *>(static_cast<const void *>(pNode)));
			pNode = pPrev;
		}
		x_pFirst = nullptr;
		x_pLast  = nullptr;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			auto pNode = x_pFirst;
			while(pNode){
				const auto pNext = pNode->pNext;
				const auto pElement = X_GetElementFromNode(pNode);
				*itOutput = std::move(*pElement);
				++itOutput;
				pNode = pNext;
			}
		} catch(...){
			Clear();
			throw;
		}
		Clear();
		return itOutput;
	}

	const Element *GetFirst() const noexcept {
		const auto pNode = x_pFirst;
		if(!pNode){
			return nullptr;
		}
		return X_GetElementFromNode(pNode);
	}
	Element *GetFirst() noexcept {
		const auto pNode = x_pFirst;
		if(!pNode){
			return nullptr;
		}
		return X_GetElementFromNode(pNode);
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		const auto pNode = x_pLast;
		if(!pNode){
			return nullptr;
		}
		return X_GetElementFromNode(pNode);
	}
	Element *GetLast() noexcept {
		const auto pNode = x_pLast;
		if(!pNode){
			return nullptr;
		}
		return X_GetElementFromNode(pNode);
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	static const Element *GetPrev(const Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pPrev = X_GetNodeFromElement(pPos)->pPrev;
		if(!pPrev){
			return nullptr;
		}
		return X_GetElementFromNode(pPrev);
	}
	static Element *GetPrev(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pPrev = X_GetNodeFromElement(pPos)->pPrev;
		if(!pPrev){
			return nullptr;
		}
		return X_GetElementFromNode(pPrev);
	}
	static const Element *GetNext(const Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pNext = X_GetNodeFromElement(pPos)->pNext;
		if(!pNext){
			return nullptr;
		}
		return X_GetElementFromNode(pNext);
	}
	static Element *GetNext(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pNext = X_GetNodeFromElement(pPos)->pNext;
		if(!pNext){
			return nullptr;
		}
		return X_GetElementFromNode(pNext);
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

	void Swap(List &rhs) noexcept {
		using std::swap;
		swap(x_pFirst, rhs.x_pFirst);
		swap(x_pLast,  rhs.x_pLast);
	}

	// List 需求。
	std::size_t CountElements() const noexcept {
		std::size_t uCount = 0;
		for(auto pElem = GetFirst(); pElem; pElem = GetNext(pElem)){
			++uCount;
		}
		return uCount;
	}

	template<typename ...ParamsT>
	Element &Unshift(ParamsT &&...vParams){
		const auto pNewFirst = static_cast<X_Node *>(Allocator()(kNodeSize));
		const auto pElement = X_GetElementFromNode(pNewFirst);
		try {
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		} catch(...){
			Allocator()(const_cast<void *>(static_cast<const void *>(pNewFirst)));
			throw;
		}
		pNewFirst->pPrev = nullptr;
		pNewFirst->pNext = x_pFirst;

		const auto pOldFirst = x_pFirst;
		(pOldFirst ? pOldFirst->pPrev : x_pLast) = pNewFirst;
		x_pFirst = pNewFirst;

		return *pElement;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= CountElements());

		auto pNewFirst = x_pFirst;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pNext = pNewFirst->pNext;
			const auto pElement = X_GetElementFromNode(pNewFirst);
			Destruct(pElement);
			Allocator()(const_cast<void *>(static_cast<const void *>(pNewFirst)));
			pNewFirst = pNext;
		}
		(pNewFirst ? pNewFirst->pPrev : x_pLast) = nullptr;
		x_pFirst = pNewFirst;
	}

	template<typename ...ParamsT>
	Element &Push(ParamsT &&...vParams){
		const auto pNewLast = static_cast<X_Node *>(Allocator()(kNodeSize));
		const auto pElement = X_GetElementFromNode(pNewLast);
		try {
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		} catch(...){
			Allocator()(const_cast<void *>(static_cast<const void *>(pNewLast)));
			throw;
		}
		pNewLast->pPrev = x_pLast;
		pNewLast->pNext = nullptr;

		const auto pOldLast = x_pLast;
		(pOldLast ? pOldLast->pNext : x_pFirst) = pNewLast;
		x_pLast = pNewLast;

		return *pElement;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= CountElements());

		auto pNewLast = x_pLast;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pPrev = pNewLast->pPrev;
			const auto pElement = X_GetElementFromNode(pNewLast);
			Destruct(pElement);
			Allocator()(const_cast<void *>(static_cast<const void *>(pNewLast)));
			pNewLast = pPrev;
		}
		(pNewLast ? pNewLast->pNext : x_pFirst) = nullptr;
		x_pLast = pNewLast;
	}

	template<typename ...ParamsT>
	void Prepend(std::size_t uDeltaSize, const ParamsT &...vParams){
		List lstNew;
		lstNew.Append(uDeltaSize, vParams...);
		Splice(GetFirst(), lstNew);
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Prepend(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		List lstNew;
		lstNew.Append(itBegin, itEnd);
		Splice(GetFirst(), lstNew);
	}
	void Prepend(std::initializer_list<Element> ilElements){
		Prepend(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	void Append(std::size_t uDeltaSize, const ParamsT &...vParams){
		List lstNew;
		for(std::size_t i = 0; i < uDeltaSize; ++i){
			lstNew.Push(vParams...);
		}
		Splice(nullptr, lstNew);
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		List lstNew;
		for(auto it = itBegin; it != itEnd; ++it){
			lstNew.Push(*it);
		}
		Splice(nullptr, lstNew);
	}
	void Append(std::initializer_list<Element> ilElements){
		Append(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	Element *Emplace(const Element *pPos, ParamsT &&...vParams){
		List lstNew;
		lstNew.Push(std::forward<ParamsT>(vParams)...);
		const auto pRet = lstNew.GetFirst();
		Splice(pPos, lstNew);
		return pRet;
	}
	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		List lstNew;
		lstNew.Append(uDeltaSize, vParams...);
		const auto pRet = lstNew.GetFirst();
		Splice(pPos, lstNew);
		return pRet;
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	Element *Insert(const Element *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		List lstNew;
		lstNew.Append(itBegin, itEnd);
		const auto pRet = lstNew.GetFirst();
		Splice(pPos, lstNew);
		return pRet;
	}
	Element *Insert(const Element *pPos, std::initializer_list<Element> ilElements){
		return Insert(pPos, ilElements.begin(), ilElements.end());
	}

	Element *Erase(const Element *pBegin, const Element *pEnd) noexcept {
		List lstErased;
		lstErased.Splice(nullptr, *this, pBegin, pEnd);
		return const_cast<Element *>(pEnd);
	}
	Element *Erase(const Element *pPos) noexcept {
		return Erase(pPos, GetNext(pPos));
	}

	Element *Splice(const Element *pInsert, List &lstSrc) noexcept {
		return Splice(pInsert, lstSrc, lstSrc.GetFirst(), nullptr);
	}
	Element *Splice(const Element *pInsert, List &lstSrc, const Element *pPos) noexcept {
		return Splice(pInsert, lstSrc, pPos, lstSrc.GetNext(pPos));
	}
	Element *Splice(const Element *pInsert, List &lstSrc, const Element *pBegin, const Element *pEnd) noexcept {
		MCF_DEBUG_CHECK(&lstSrc != this);

		const auto pInsertNode = pInsert ? X_GetNodeFromElement(const_cast<Element *>(pInsert)) : nullptr;
		const auto pBeginNode  = pBegin  ? X_GetNodeFromElement(const_cast<Element *>(pBegin )) : nullptr;
		const auto pEndNode    = pEnd    ? X_GetNodeFromElement(const_cast<Element *>(pEnd   )) : nullptr;

		if(pBeginNode != pEndNode){
			MCF_DEBUG_CHECK(pBeginNode);

			const auto pNodeBeforeBegin = pBeginNode->pPrev;
			const auto pNodeBeforeEnd = std::exchange(pEndNode ? pEndNode->pPrev : lstSrc.x_pLast, pNodeBeforeBegin);
			const auto pNodeBeforeInsert = std::exchange(pInsertNode ? pInsertNode->pPrev : x_pLast, pNodeBeforeEnd);

			(pNodeBeforeInsert ? pNodeBeforeInsert->pNext : x_pFirst) = pBeginNode;
			(pNodeBeforeBegin ? pNodeBeforeBegin->pNext: lstSrc.x_pFirst) = pEndNode;
			pBeginNode->pPrev = pNodeBeforeInsert;
			pNodeBeforeEnd->pNext = pInsertNode;
		}
		return const_cast<Element *>(pInsert);
	}

	Element *Splice(const Element *pInsert, List &&lstSrc) noexcept {
		return Splice(pInsert, lstSrc);
	}
	Element *Splice(const Element *pInsert, List &&lstSrc, const Element *pPos) noexcept {
		return Splice(pInsert, lstSrc, pPos);
	}
	Element *Splice(const Element *pInsert, List &&lstSrc, const Element *pBegin, const Element *pEnd) noexcept {
		return Splice(pInsert, lstSrc, pBegin, pEnd);
	}

	void Reverse() noexcept {
		auto ppNext = &x_pFirst, ppPrev = &x_pLast;
		for(;;){
			const auto pNext = *ppNext;
			*ppNext = *ppPrev;
			*ppPrev = pNext;
			if(!pNext){
				break;
			}
			ppNext = &(pNext->pNext);
			ppPrev = &(pNext->pPrev);
		}
	}

public:
	friend void swap(List &lhs, List &rhs) noexcept {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const List &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) begin(List &rhs) noexcept {
		return rhs.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const List &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const List &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) end(List &rhs) noexcept {
		return rhs.EnumerateSingular();
	}
	friend decltype(auto) cend(const List &rhs) noexcept {
		return end(rhs);
	}
};

}

#endif
