// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MULTI_INDEXED_HPP_
#define MCF_MULTI_INDEXED_HPP_

#include "../../MCFCRT/stdc/ext/offset_of.h"
#include "../../MCFCRT/stdc/ext/unref_param.h"
#include "../../MCFCRT/env/avl_tree.h"
#include "Utilities.hpp"
#include <tuple>
#include <type_traits>
#include <functional>

namespace MCF {

template<class Element_t, class... Indexes_t>
class MultiIndexedMap;

template<class Element_t, class... Indexes_t>
class MultiIndexedMapNode {
	static_assert(sizeof...(Indexes_t) > 0, "No index?");

	template<class, class...>
	friend class MultiIndexedMap;

private:
	typedef std::tuple<Indexes_t...> xIndexTuple;

private:
	Element_t xm_vElement;
	xIndexTuple xm_vIndexes;
	::MCF_AVL_NODE_HEADER xm_aHeaders[sizeof...(Indexes_t)];

public:
	explicit constexpr MultiIndexedMapNode(Element_t vElement, Indexes_t... vIndexes)
		: xm_vElement(std::move(vElement))
		, xm_vIndexes(std::move(vIndexes)...)
		, xm_aHeaders()
	{
	}

public:
	constexpr const Element_t &GetElement() const noexcept {
		return xm_vElement;
	}
	Element_t &GetElement() noexcept {
		return xm_vElement;
	}
	template<std::size_t INDEX>
	constexpr auto GetIndex() const noexcept
		-> const typename std::tuple_element<INDEX, xIndexTuple>::type &
	{
		return std::get<INDEX>(xm_vIndexes);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetPrev() const noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(&(xm_aHeaders[INDEX]));
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetPrev() noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(&(xm_aHeaders[INDEX]));
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetNext() const noexcept {
		const auto pNextAvl = ::MCF_AvlNext(&(xm_aHeaders[INDEX]));
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetNext() noexcept {
		const auto pNextAvl = ::MCF_AvlNext(&(xm_aHeaders[INDEX]));
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
};

template<class... Indexes_t>
class MultiIndexedMapNode<void, Indexes_t...> {
	static_assert(sizeof...(Indexes_t) > 0, "No index?");

	template<class, class...>
	friend class MultiIndexedMap;

private:
	typedef std::tuple<Indexes_t...> xIndexTuple;

private:
	xIndexTuple xm_vIndexes;
	::MCF_AVL_NODE_HEADER xm_aHeaders[sizeof...(Indexes_t)];

public:
	explicit constexpr MultiIndexedMapNode(Indexes_t... vIndexes)
		: xm_vIndexes(std::move(vIndexes)...)
		, xm_aHeaders()
	{
	}

public:
	template<std::size_t INDEX>
	constexpr auto GetIndex() const noexcept
		-> const typename std::tuple_element<INDEX, xIndexTuple>::type &
	{
		return std::get<INDEX>(xm_vIndexes);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetPrev() const noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(&(xm_aHeaders[INDEX]));
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetPrev() noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(&(xm_aHeaders[INDEX]));
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetNext() const noexcept {
		const auto pNextAvl = ::MCF_AvlNext(&(xm_aHeaders[INDEX]));
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetNext() noexcept {
		const auto pNextAvl = ::MCF_AvlNext(&(xm_aHeaders[INDEX]));
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
};

template<class Element_t, class... Indexes_t>
class MultiIndexedMap {
	static_assert(sizeof...(Indexes_t) > 0, "No index?");

public:
	typedef MultiIndexedMapNode<Element_t, Indexes_t...> Node;

private:
	template<std::size_t INDEX>
	struct xComparers {
		static int Nodes(
			const MCF_AVL_NODE_HEADER *pAvl1,
			const MCF_AVL_NODE_HEADER *pAvl2
		) noexcept {
			const auto pNode1 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl1);
			const auto pNode2 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl2);

			static_assert(noexcept(std::get<INDEX>(pNode1->xm_vIndexes) < std::get<INDEX>(pNode2->xm_vIndexes)), "Comparer must not throw any exceptions.");

			return std::get<INDEX>(pNode1->xm_vIndexes) < std::get<INDEX>(pNode2->xm_vIndexes);
		}
		template<typename Other_t>
		static int NodeOther(
			const MCF_AVL_NODE_HEADER *pAvl1,
			std::intptr_t nOther
		) noexcept {
			const auto pNode1 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl1);

			static_assert(noexcept(std::get<INDEX>(pNode1->xm_vIndexes) < *(const Other_t *)nOther), "Comparer must not throw any exceptions.");

			return std::get<INDEX>(pNode1->xm_vIndexes) < *(const Other_t *)nOther;
		}
		template<typename Other_t>
		static int OtherNode(
			std::intptr_t nOther,
			const MCF_AVL_NODE_HEADER *pAvl2
		) noexcept {
			const auto pNode2 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl2);

			static_assert(noexcept(*(const Other_t *)nOther < std::get<INDEX>(pNode2->xm_vIndexes)), "Comparer must not throw any exceptions.");

			return *(const Other_t *)nOther < std::get<INDEX>(pNode2->xm_vIndexes);
		}
	};

private:
	struct {
		::MCF_AVL_NODE_HEADER *pRoot;
		::MCF_AVL_NODE_HEADER *pFirst;
		::MCF_AVL_NODE_HEADER *pLast;
	} xm_aNodes[sizeof...(Indexes_t)];

	std::size_t xm_uSize;

public:
	constexpr MultiIndexedMap() noexcept
		: xm_aNodes()
		, xm_uSize(0)
	{
	}
	MultiIndexedMap(const MultiIndexedMap &rhs)
		: MultiIndexedMap()
	{
		auto pNode = rhs.GetBegin<0>();
		while(pNode){
			Insert(*pNode);
			pNode = pNode->template GetNext<0>();
		}
	}
	MultiIndexedMap(MultiIndexedMap &&rhs) noexcept
		: MultiIndexedMap()
	{
		Swap(rhs);
	}
	MultiIndexedMap &operator=(const MultiIndexedMap &rhs){
		if(this != &rhs){
			Clear();
			MultiIndexedMap(rhs).Swap(*this);
		}
		return *this;
	}
	MultiIndexedMap &operator=(MultiIndexedMap &&rhs) noexcept {
		Swap(rhs);
		return *this;
	}
	~MultiIndexedMap() noexcept {
		Clear();
	}

private:
	template<std::size_t INDEX>
	void xAttach(Node *pNode) noexcept {
		const auto pAvl = &(pNode->xm_aHeaders[INDEX]);
		::MCF_AvlAttach(
			&(xm_aNodes[INDEX].pRoot),
			pAvl,
			&(xComparers<INDEX>::Nodes)
		);
		if(!::MCF_AvlPrev(pAvl)){
			xm_aNodes[INDEX].pFirst = pAvl;
		}
		if(!::MCF_AvlNext(pAvl)){
			xm_aNodes[INDEX].pLast = pAvl;
		}
	}
	template<std::size_t INDEX>
	void xDetach(Node *pNode) noexcept {
		const auto pAvl = &(pNode->xm_aHeaders[INDEX]);
		if(pAvl == xm_aNodes[INDEX].pFirst){
			xm_aNodes[INDEX].pFirst = ::MCF_AvlNext(pAvl);
		}
		if(pAvl == xm_aNodes[INDEX].pLast){
			xm_aNodes[INDEX].pLast = ::MCF_AvlPrev(pAvl);
		}
		::MCF_AvlDetach(pAvl);
	}

	template<std::size_t INDEX>
	void xAttachRecur(Node *pNode, typename std::enable_if<(INDEX < sizeof...(Indexes_t)), int>::type = 0) noexcept {
		xAttach<INDEX>(pNode);
		xAttachRecur<INDEX + 1>(pNode);
	}
	template<std::size_t INDEX>
	void xAttachRecur(Node *, typename std::enable_if<(INDEX == sizeof...(Indexes_t)), int>::type = 0) noexcept {
	}

	template<std::size_t INDEX>
	void xDetachRecur(Node *pNode, typename std::enable_if<(INDEX < sizeof...(Indexes_t)), int>::type = 0) noexcept {
		xDetachRecur<INDEX + 1>(pNode);
		xDetach<INDEX>(pNode);
	}
	template<std::size_t INDEX>
	void xDetachRecur(Node *, typename std::enable_if<(INDEX == sizeof...(Indexes_t)), int>::type = 0) noexcept {
	}

public:
	template<typename... Params_t>
	Node *Insert(Params_t &&... vParams){
		const auto pNode = new Node(std::forward<Params_t>(vParams)...);
		xAttachRecur<0>(pNode);
		++xm_uSize;
		return pNode;
	}
	void Erase(Node *pNode) noexcept {
		xDetachRecur<0>(pNode);
		--xm_uSize;
		delete pNode;
	}
	void Clear() noexcept {
		auto pNode = GetBegin<0>();
		while(pNode){
			const auto pNext = pNode->template GetNext<0>();
			delete pNode;
			pNode = pNext;
		}

		BZero(xm_aNodes);
		xm_uSize = 0;
	}

	std::size_t GetSize() const noexcept {
		return xm_uSize;
	}
	bool IsEmpty() const noexcept {
		return xm_uSize == 0;
	}

	void Swap(MultiIndexedMap &rhs) noexcept {
		if(this != &rhs){
			for(std::size_t i = 0; i < sizeof...(Indexes_t); ++i){
				::MCF_AvlSwap	(&(xm_aNodes[i].pRoot),	&(rhs.xm_aNodes[i].pRoot));
				std::swap		(xm_aNodes[i].pFirst,	rhs.xm_aNodes[i].pFirst);
				std::swap		(xm_aNodes[i].pLast,	rhs.xm_aNodes[i].pLast);
			}
			std::swap(xm_uSize, rhs.xm_uSize);
		}
	}

	template<std::size_t INDEX, typename... Params_t>
	void SetIndex(Node *pNode, Params_t &&... vParams){
		auto &vIndex = std::get<INDEX>(pNode->xm_vIndexes);
		std::get<INDEX>(pNode->xm_vIndexes) =
			typename std::remove_reference<decltype(vIndex)>::type(std::forward<Params_t>(vParams)...);

		xDetach<INDEX>(pNode);
		xAttach<INDEX>(pNode);
	}

	template<std::size_t INDEX>
	const Node *GetBegin() const noexcept {
		const auto pAvl = xm_aNodes[INDEX].pFirst;
		if(!pAvl){
			return nullptr;
		}
		return DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl);
	}
	template<std::size_t INDEX>
	Node *GetBegin() noexcept {
		const auto pAvl = xm_aNodes[INDEX].pFirst;
		if(!pAvl){
			return nullptr;
		}
		return DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl);
	}
	template<std::size_t INDEX>
	const Node *GetCBegin() const noexcept {
		return GetBegin();
	}

	template<std::size_t INDEX>
	const Node *GetRBegin() const noexcept {
		const auto pAvl = xm_aNodes[INDEX].pLast;
		if(!pAvl){
			return nullptr;
		}
		return DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl);
	}
	template<std::size_t INDEX>
	Node *GetRBegin() noexcept {
		const auto pAvl = xm_aNodes[INDEX].pLast;
		if(!pAvl){
			return nullptr;
		}
		return DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl);
	}
	template<std::size_t INDEX>
	const Node *GetCRBegin() const noexcept {
		return GetRBegin();
	}

	template<std::size_t INDEX, typename Comparand_t>
	const Node *GetLowerBound(const Comparand_t &vComparand) const noexcept {
		const auto pAvl = ::MCF_AvlLowerBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return pAvl ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}
	template<std::size_t INDEX, typename Comparand_t>
	Node *GetLowerBound(const Comparand_t &vComparand) noexcept {
		const auto pAvl = ::MCF_AvlLowerBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return pAvl ? DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}

	template<std::size_t INDEX, typename Comparand_t>
	const Node *GetUpperBound(const Comparand_t &vComparand) const noexcept {
		const auto pAvl = ::MCF_AvlUpperBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return pAvl ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}
	template<std::size_t INDEX, typename Comparand_t>
	Node *GetUpperBound(const Comparand_t &vComparand) noexcept {
		const auto pAvl = ::MCF_AvlUpperBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return pAvl ? DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}

	template<std::size_t INDEX, typename Comparand_t>
	const Node *Find(const Comparand_t &vComparand) const noexcept {
		const auto pAvl = ::MCF_AvlFind(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return pAvl ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}
	template<std::size_t INDEX, typename Comparand_t>
	Node *Find(const Comparand_t &vComparand) noexcept {
		const auto pAvl = ::MCF_AvlFind(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return pAvl ? DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}

	template<std::size_t INDEX, typename Comparand_t>
	std::pair<const Node *, const Node *> GetEqualRange(const Comparand_t &vComparand) const noexcept {
		::MCF_AVL_NODE_HEADER *pBegin, *pEnd;
		::MCF_AvlEqualRange(
			&pBegin,
			&pEnd,
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return std::make_pair(
			pBegin ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pBegin) : nullptr,
			pEnd ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pEnd) : nullptr
		);
	}
	template<std::size_t INDEX, typename Comparand_t>
	std::pair<Node *, Node *> GetEqualRange(const Comparand_t &vComparand) noexcept {
		::MCF_AVL_NODE_HEADER *pBegin, *pEnd;
		::MCF_AvlEqualRange(
			&pBegin,
			&pEnd,
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparers<INDEX>::template NodeOther<Comparand_t>),
			&(xComparers<INDEX>::template OtherNode<Comparand_t>)
		);
		return std::make_pair(
			pBegin ? DOWN_CAST(Node, xm_aHeaders[INDEX], pBegin) : nullptr,
			pEnd ? DOWN_CAST(Node, xm_aHeaders[INDEX], pEnd) : nullptr
		);
	}
};

}

#endif
