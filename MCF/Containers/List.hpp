// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_LIST_HPP_
#define MCF_CONTAINERS_LIST_HPP_

#include "DefaultAllocator.hpp"
#include "_Enumerator.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/AlignedStorage.hpp"
#include "../Utilities/ConstructDestruct.hpp"
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
		AlignedStorage<0, Element> vElement;
		X_Node *pPrev;
		X_Node *pNext;
	};

public:
	enum : std::size_t {
		kNodeSize = sizeof(X_Node),
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
			const auto pElement = reinterpret_cast<const Element *>(pNode);
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
			const auto pElement = reinterpret_cast<Element *>(pNode);
			Destruct(pElement);
			Allocator()(pNode);
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
				const auto pElement = reinterpret_cast<Element *>(pNode);
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
		return reinterpret_cast<const Element *>(pNode);
	}
	Element *GetFirst() noexcept {
		const auto pNode = x_pFirst;
		return reinterpret_cast<Element *>(pNode);
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		const auto pNode = x_pLast;
		return reinterpret_cast<const Element *>(pNode);
	}
	Element *GetLast() noexcept {
		const auto pNode = x_pLast;
		return reinterpret_cast<Element *>(pNode);
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	static const Element *GetPrev(const Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pPrev = reinterpret_cast<const X_Node *>(pPos)->pPrev;
		return reinterpret_cast<const Element *>(pPrev);
	}
	static Element *GetPrev(Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pPrev = reinterpret_cast<const X_Node *>(pPos)->pPrev;
		return reinterpret_cast<Element *>(pPrev);
	}
	static const Element *GetNext(const Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pNext = reinterpret_cast<const X_Node *>(pPos)->pNext;
		return reinterpret_cast<const Element *>(pNext);
	}
	static Element *GetNext(Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pNext = reinterpret_cast<const X_Node *>(pPos)->pNext;
		return reinterpret_cast<Element *>(pNext);
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
		const auto pNode = static_cast<X_Node *>(Allocator()(kNodeSize));
		const auto pElement = reinterpret_cast<Element *>(&(pNode->vElement));
		try {
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		} catch(...){
			Allocator()(pNode);
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

		return *pElement;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		MCF_ASSERT(uCount <= CountElements());

		auto pNode = x_pFirst;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pNext = pNode->pNext;
			const auto pElement = reinterpret_cast<Element *>(&(pNode->vElement));
			Destruct(pElement);
			Allocator()(pNode);
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
	Element &Push(ParamsT &&...vParams){
		const auto pNode = static_cast<X_Node *>(Allocator()(kNodeSize));
		const auto pElement = reinterpret_cast<Element *>(&(pNode->vElement));
		try {
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		} catch(...){
			Allocator()(pNode);
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

		return *pElement;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		MCF_ASSERT(uCount <= CountElements());

		auto pNode = x_pLast;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pPrev = pNode->pPrev;
			const auto pElement = reinterpret_cast<Element *>(&(pNode->vElement));
			Destruct(pElement);
			Allocator()(pNode);
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
		lstNew.Append(uDeltaSize, vParams...);
		Splice(nullptr, lstNew);
	}
	template<typename IteratorT, std::enable_if_t<
		std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value,
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		List lstNew;
		lstNew.Append(itBegin, itEnd);
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
		const auto pInsertNode = reinterpret_cast<X_Node *>(const_cast<Element *>(pInsert));
		const auto pBeginNode = reinterpret_cast<X_Node *>(const_cast<Element *>(pBegin));
		const auto pEndNode = reinterpret_cast<X_Node *>(const_cast<Element *>(pEnd));

		if(pBeginNode == pEndNode){
			return const_cast<Element *>(pInsert);
		}
		MCF_ASSERT(pBeginNode);

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
