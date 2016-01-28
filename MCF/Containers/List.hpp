// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_LIST_HPP_
#define MCF_CONTAINERS_LIST_HPP_

#include "_Enumerator.hpp"
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
public:
	// 容器需求。
	using Element         = ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <List>;
	using Enumerator      = Impl_Enumerator::Enumerator      <List>;

private:
	struct X_Node {
		alignas(Element) char aStorage[sizeof(Element)];
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
	List(std::initializer_list<Element> rhs)
		: List(rhs.begin(), rhs.end())
	{
	}
	List(const List &rhs)
		: List()
	{
		Append(rhs.EnumerateFirst(), rhs.EnumerateSingular());
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
			Destruct(reinterpret_cast<Element *>(pNode->aStorage));
			::delete pNode;
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
				*itOutput = std::move(*reinterpret_cast<Element *>(pNode->aStorage));
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
		return reinterpret_cast<const Element *>(pNode->aStorage);
	}
	Element *GetFirst() noexcept {
		const auto pNode = x_pFirst;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<Element *>(pNode->aStorage);
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		const auto pNode = x_pLast;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<const Element *>(pNode->aStorage);
	}
	Element *GetLast() noexcept {
		const auto pNode = x_pLast;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<Element *>(pNode->aStorage);
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	static const Element *GetPrev(const Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pPrev;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<const Element *>(pNode->aStorage);
	}
	static Element *GetPrev(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pPrev;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<Element *>(pNode->aStorage);
	}
	static const Element *GetNext(const Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pNext;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<const Element *>(pNode->aStorage);
	}
	static Element *GetNext(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pNode = reinterpret_cast<const X_Node *>(pPos)->pNext;
		if(!pNode){
			return nullptr;
		}
		return reinterpret_cast<Element *>(pNode->aStorage);
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
		const auto pNode = ::new X_Node;
		const auto pElem = reinterpret_cast<Element *>(pNode->aStorage);
		try {
			DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
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

		return *pElem;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= CountElements());

		auto pNode = x_pFirst;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pNext = pNode->pNext;
			Destruct(reinterpret_cast<Element *>(pNode->aStorage));
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
	Element &Push(ParamsT &&...vParams){
		const auto pNode = ::new X_Node;
		const auto pElem = reinterpret_cast<Element *>(pNode->aStorage);
		try {
			DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
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

		return *pElem;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= CountElements());

		auto pNode = x_pLast;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pPrev = pNode->pPrev;
			Destruct(reinterpret_cast<Element *>(pNode->aStorage));
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
	void Prepend(std::size_t uDeltaSize, const ParamsT &...vParams){
		List lstNew;
		lstNew.Append(uDeltaSize, vParams...);
		Splice(GetFirst(), lstNew);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
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
	void Append(std::initializer_list<Element> ilElements){
		Append(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	Element *Emplace(const Element *pPos, ParamsT &&...vParams){
		List lstNew;
		lstNew.Push(std::forward<ParamsT>(vParams)...);
		return Splice(pPos, lstNew);
	}
	template<typename ...ParamsT>
	Element *Insert(const Element *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		List lstNew;
		lstNew.Append(uDeltaSize, vParams...);
		return Splice(pPos, lstNew);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	Element *Insert(const Element *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		List lstNew;
		lstNew.Append(itBegin, itEnd);
		return Splice(pPos, lstNew);
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
		ASSERT(pBeginNode);

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
		auto pNode = x_pFirst;
		x_pFirst = x_pLast;
		x_pLast = pNode;

		while(pNode){
			const auto pNext = pNode->pNext;
			pNode->pNext = pNode->pPrev;
			pNode->pPrev = pNext;
			pNode = pNext;
		}
	}

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
