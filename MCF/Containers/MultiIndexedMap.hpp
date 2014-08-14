// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_MULTI_INDEXED_HPP_
#define MCF_MULTI_INDEXED_HPP_

#include "../../MCFCRT/ext/offset_of.h"
#include "../../MCFCRT/env/avl_tree.h"
#include "../Utilities/Utilities.hpp"
#include "../Containers/VVector.hpp"
#include <tuple>
#include <type_traits>
#include <functional>

namespace MCF {

// 这并不是真正的索引，因为它不能用来检索元素，而只能用来遍历。
// 但是这个索引保证了元素创建的顺序。
struct SequenceIndex {
};

template<class Element, class... Indices>
class MultiIndexedMap;

template<class Element, class... Indices>
class MultiIndexedMapNode {
	static_assert(sizeof...(Indices) > 0, "No index?");

	template<class, class...>
	friend class MultiIndexedMap;

private:
	typedef std::tuple<Indices...> xIndexTuple;

private:
	Element xm_vElement;
	xIndexTuple xm_vIndices;
	MCF_AVL_NODE_HEADER xm_aHeaders[sizeof...(Indices)];

	const MultiIndexedMapNode *xm_pReserved; // 复制树结构的时候用到。
	MCF_AVL_NODE_HEADER xm_vReservedHeader;

public:
	explicit constexpr MultiIndexedMapNode(Element vElement, Indices... vIndices)
		: xm_vElement(std::move(vElement))
		, xm_vIndices(std::move(vIndices)...)
		, xm_aHeaders()
	{
	}

public:
	constexpr const Element &GetElement() const noexcept {
		return xm_vElement;
	}
	Element &GetElement() noexcept {
		return xm_vElement;
	}
	template<std::size_t INDEX>
	constexpr auto &GetIndex() const noexcept {
		return std::get<INDEX>(xm_vIndices);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetPrev() const noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(xm_aHeaders + INDEX);
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetPrev() noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(xm_aHeaders + INDEX);
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetNext() const noexcept {
		const auto pNextAvl = ::MCF_AvlNext(xm_aHeaders + INDEX);
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetNext() noexcept {
		const auto pNextAvl = ::MCF_AvlNext(xm_aHeaders + INDEX);
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
};

template<class... Indices>
class MultiIndexedMapNode<void, Indices...> {
	static_assert(sizeof...(Indices) > 0, "No index?");

	template<class, class...>
	friend class MultiIndexedMap;

private:
	typedef std::tuple<Indices...> xIndexTuple;

private:
	xIndexTuple xm_vIndices;
	MCF_AVL_NODE_HEADER xm_aHeaders[sizeof...(Indices)];

	const MultiIndexedMapNode *xm_pReserved; // 复制树结构的时候用到。
	MCF_AVL_NODE_HEADER xm_vReservedHeader;

public:
	explicit constexpr MultiIndexedMapNode(Indices... vIndices)
		: xm_vIndices(std::move(vIndices)...)
		, xm_aHeaders()
	{
	}

public:
	template<std::size_t INDEX>
	constexpr auto &GetIndex() const noexcept {
		return std::get<INDEX>(xm_vIndices);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetPrev() const noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(xm_aHeaders + INDEX);
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetPrev() noexcept {
		const auto pPrevAvl = ::MCF_AvlPrev(xm_aHeaders + INDEX);
		if(!pPrevAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pPrevAvl);
	}

	template<std::size_t INDEX>
	const MultiIndexedMapNode *GetNext() const noexcept {
		const auto pNextAvl = ::MCF_AvlNext(xm_aHeaders + INDEX);
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(const MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
	template<std::size_t INDEX>
	MultiIndexedMapNode *GetNext() noexcept {
		const auto pNextAvl = ::MCF_AvlNext(xm_aHeaders + INDEX);
		if(!pNextAvl){
			return nullptr;
		}
		return DOWN_CAST(MultiIndexedMapNode, xm_aHeaders[INDEX], pNextAvl);
	}
};

template<class Element, class... Indices>
class MultiIndexedMap {
	static_assert(sizeof...(Indices) > 0, "No index?");

public:
	typedef MultiIndexedMapNode<Element, Indices...> Node;

private:
	struct xBstNodes {
		MCF_AVL_ROOT pRoot;
		MCF_AVL_NODE_HEADER *pFirst;
		MCF_AVL_NODE_HEADER *pLast;
	};

	template<std::size_t INDEX>
	struct xComparators {
		typedef typename std::tuple_element<
			INDEX, typename Node::xIndexTuple
			>::type IndexType;

		static bool Nodes(const MCF_AVL_NODE_HEADER *pAvl1, const MCF_AVL_NODE_HEADER *pAvl2) noexcept {
			const auto pNode1 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl1);
			const auto pNode2 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl2);

			static_assert(
				noexcept(std::less<void>()(std::get<INDEX>(pNode1->xm_vIndices), std::get<INDEX>(pNode2->xm_vIndices))),
				"Do not throw exceptions inside comparators."
			);
			return std::less<void>()(std::get<INDEX>(pNode1->xm_vIndices), std::get<INDEX>(pNode2->xm_vIndices));
		}
		template<typename Other>
		static bool NodeOther(const MCF_AVL_NODE_HEADER *pAvl1, std::intptr_t nOther) noexcept {
			const auto pNode1 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl1);

			static_assert(
				noexcept(std::less<void>()(std::get<INDEX>(pNode1->xm_vIndices), *(const Other *)nOther)),
				"Do not throw exceptions inside comparators."
			);
			return std::less<void>()(std::get<INDEX>(pNode1->xm_vIndices), *(const Other *)nOther);
		}
		template<typename Other>
		static bool OtherNode(std::intptr_t nOther, const MCF_AVL_NODE_HEADER *pAvl2) noexcept {
			const auto pNode2 = DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl2);

			static_assert(
				noexcept(std::less<void>()(*(const Other *)nOther, std::get<INDEX>(pNode2->xm_vIndices))),
				"Do not throw exceptions inside comparators."
			);
			return std::less<void>()(*(const Other *)nOther, std::get<INDEX>(pNode2->xm_vIndices));
		}
	};

	struct xAddressComparators {
		static bool Nodes(const MCF_AVL_NODE_HEADER *pAvl1, const MCF_AVL_NODE_HEADER *pAvl2) noexcept {
			const auto pNode1 = DOWN_CAST(const Node, xm_vReservedHeader, pAvl1);
			const auto pNode2 = DOWN_CAST(const Node, xm_vReservedHeader, pAvl2);
			return std::less<void>()(pNode1->xm_pReserved, pNode2->xm_pReserved);
		}
		static bool NodeOther(const MCF_AVL_NODE_HEADER *pAvl1, std::intptr_t nOther) noexcept {
			const auto pNode1 = DOWN_CAST(const Node, xm_vReservedHeader, pAvl1);
			return std::less<void>()(pNode1->xm_pReserved, (const Node *)nOther);
		}
		static bool OtherNode(std::intptr_t nOther, const MCF_AVL_NODE_HEADER *pAvl2) noexcept {
			const auto pNode2 = DOWN_CAST(const Node, xm_vReservedHeader, pAvl2);
			return std::less<void>()((const Node *)nOther, pNode2->xm_pReserved);
		}
	};

private:
	xBstNodes xm_aNodes[sizeof...(Indices)];

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
		// 稳定地复制树结构。

		// 第一步，分配全部新节点。
		MCF_AVL_ROOT pavlNodeMap = nullptr;
		try {
			auto pNode = rhs.GetBegin<0>();
			while(pNode){
				const auto pNewNode = new Node(*pNode);
				pNewNode->xm_pReserved = pNode; // 放置源节点的地址。
				::MCF_AvlAttach(
					&pavlNodeMap,
					&(pNewNode->xm_vReservedHeader),
					&(xAddressComparators::Nodes)
				);
				pNode = pNode->template GetNext<0>();
			}
		} catch(...){
			if(pavlNodeMap){
				auto pCur = ::MCF_AvlPrev(pavlNodeMap);
				while(pCur){
					const auto pPrev = ::MCF_AvlPrev(pCur);
					delete DOWN_CAST(Node, xm_vReservedHeader, pCur);
					pCur = pPrev;
				}
				pCur = ::MCF_AvlNext(pavlNodeMap);
				while(pCur){
					const auto pNext = ::MCF_AvlNext(pCur);
					delete DOWN_CAST(Node, xm_vReservedHeader, pCur);
					pCur = pNext;
				}
				delete DOWN_CAST(Node, xm_vReservedHeader, pavlNodeMap);
			}
			throw;
		}

		// 所有节点都分配完成，现在进行第二步，重建每个二叉树，保证无异常抛出。
		xCloneTreeRecur<0>(rhs.xm_aNodes, pavlNodeMap);

		// 最后一步，填写 size。
		xm_uSize = rhs.xm_uSize;
	}
	MultiIndexedMap(MultiIndexedMap &&rhs) noexcept
		: MultiIndexedMap()
	{
		Swap(rhs);
	}
	MultiIndexedMap &operator=(const MultiIndexedMap &rhs){
		MultiIndexedMap(rhs).Swap(*this);
		return *this;
	}
	MultiIndexedMap &operator=(MultiIndexedMap &&rhs) noexcept {
		MultiIndexedMap(std::move(rhs)).Swap(*this);
		return *this;
	}
	~MultiIndexedMap() noexcept {
		Clear();
	}

private:
	template<std::size_t INDEX>
	void xAttach(
		Node *pNode,
		Node *pHint,
		typename std::enable_if<
			!std::is_same<
				typename std::tuple_element<INDEX, std::tuple<Indices...>>::type, SequenceIndex
				>::value, int
			>::type = 0
	) noexcept {
		const auto pAvl = pNode->xm_aHeaders + INDEX;
		const auto pHintAvl = pHint ? (pHint->xm_aHeaders + INDEX) : nullptr;

		::MCF_AvlAttachHint(
			&(xm_aNodes[INDEX].pRoot),
			pHintAvl,
			pAvl,
			&(xComparators<INDEX>::Nodes)
		);

		if(!::MCF_AvlPrev(pAvl)){
			xm_aNodes[INDEX].pFirst = pAvl;
		}
		if(!::MCF_AvlNext(pAvl)){
			xm_aNodes[INDEX].pLast = pAvl;
		}
	}
	template<std::size_t INDEX>
	void xDetach(
		Node *pNode,
		typename std::enable_if<
			!std::is_same<
				typename std::tuple_element<INDEX, std::tuple<Indices...>>::type, SequenceIndex
				>::value, int
			>::type = 0
	) noexcept {
		const auto pAvl = pNode->xm_aHeaders + INDEX;

		if(pAvl == xm_aNodes[INDEX].pFirst){
			xm_aNodes[INDEX].pFirst = ::MCF_AvlNext(pAvl);
		}
		if(pAvl == xm_aNodes[INDEX].pLast){
			xm_aNodes[INDEX].pLast = ::MCF_AvlPrev(pAvl);
		}

		::MCF_AvlDetach(pAvl);
	}

	template<std::size_t INDEX>
	void xAttach(
		Node *pNode,
		Node *pHint,
		typename std::enable_if<
			std::is_same<
				typename std::tuple_element<INDEX, std::tuple<Indices...>>::type, SequenceIndex
				>::value, int
			>::type = 0
	) noexcept {
		const auto pAvl = pNode->xm_aHeaders + INDEX;
		(void)pHint;

		auto &pLast = xm_aNodes[INDEX].pLast;
		pAvl->pPrev = pLast;
		pAvl->pNext = nullptr;

		if(pLast){
			pLast->pNext = pAvl;
		} else {
			xm_aNodes[INDEX].pFirst = pAvl;
		}
		pLast = pAvl;
	}
	template<std::size_t INDEX>
	void xDetach(
		Node *pNode,
		typename std::enable_if<
			std::is_same<
				typename std::tuple_element<INDEX, std::tuple<Indices...>>::type, SequenceIndex
				>::value, int
			>::type = 0
	) noexcept {
		const auto pAvl = pNode->xm_aHeaders + INDEX;

		const auto pPrev = ::MCF_AvlPrev(pAvl);
		const auto pNext = ::MCF_AvlNext(pAvl);
		if(pPrev){
			pPrev->pNext = pNext;
		}
		if(pNext){
			pNext->pPrev = pPrev;
		}

		if(pAvl == xm_aNodes[INDEX].pFirst){
			xm_aNodes[INDEX].pFirst = pNext;
		}
		if(pAvl == xm_aNodes[INDEX].pLast){
			xm_aNodes[INDEX].pLast = pPrev;
		}
	}

	template<std::size_t INDEX>
	void xAttachRecur(
		Node *pNode,
		Node *pHint,
		typename std::enable_if<(INDEX < sizeof...(Indices)), int>::type = 0
	) noexcept {
		xAttach<INDEX>(pNode, pHint);
		xAttachRecur<INDEX + 1>(pNode, pHint);
	}
	template<std::size_t INDEX>
	void xAttachRecur(
		Node *,
		Node *,
		typename std::enable_if<(INDEX == sizeof...(Indices)), int>::type = 0
	) noexcept {
	}

	template<std::size_t INDEX>
	void xDetachRecur(
		Node *pNode,
		typename std::enable_if<(INDEX < sizeof...(Indices)), int>::type = 0
	) noexcept {
		xDetachRecur<INDEX + 1>(pNode);
		xDetach<INDEX>(pNode);
	}
	template<std::size_t INDEX>
	void xDetachRecur(
		Node *pNode,
		typename std::enable_if<(INDEX == sizeof...(Indices)), int>::type = 0
	) noexcept {
#ifdef NDEBUG
		(void)pNode;
#else
		__builtin_memset(&(pNode->xm_aHeaders), 0xCD, sizeof((pNode->xm_aHeaders)));
#endif
	}

	template<std::size_t INDEX>
	void xCloneTreeRecur(
		const xBstNodes *pSrcNodes,
		const MCF_AVL_ROOT &pavlAllocatedNodes,
		typename std::enable_if<(INDEX < sizeof...(Indices)), int>::type = 0
	) noexcept {
		Node *pHint = nullptr;
		for(auto pSrc = pSrcNodes[INDEX].pFirst; pSrc; pSrc = ::MCF_AvlNext(pSrc)){
			const auto pNewAvl = ::MCF_AvlFind(
				&pavlAllocatedNodes,
				(std::intptr_t)DOWN_CAST(const Node, xm_aHeaders[INDEX], pSrc),
				&(xAddressComparators::NodeOther),
				&(xAddressComparators::OtherNode)
			);
			ASSERT(pNewAvl);

			const auto pNode = DOWN_CAST(Node, xm_vReservedHeader, pNewAvl);
			xAttach<INDEX>(pNode, pHint);
			pHint = pNode;
		}
		xCloneTreeRecur<INDEX + 1>(pSrcNodes, pavlAllocatedNodes);
	}
	template<std::size_t INDEX>
	void xCloneTreeRecur(
		const xBstNodes *,
		const MCF_AVL_ROOT &,
		typename std::enable_if<(INDEX == sizeof...(Indices)), int>::type = 0
	) noexcept {
	}

public:
	template<typename... Params>
	Node *Insert(Params &&... vParams){
		return InsertHint(nullptr, std::forward<Params>(vParams)...);
	}
	template<typename... Params>
	Node *InsertHint(Node *pHint, Params &&... vParams){
		const auto pNode = new Node(std::forward<Params>(vParams)...);
		xAttachRecur<0>(pNode, pHint);
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
			for(std::size_t i = 0; i < sizeof...(Indices); ++i){
				::MCF_AvlSwap	(&(xm_aNodes[i].pRoot),	&(rhs.xm_aNodes[i].pRoot));
				std::swap		(xm_aNodes[i].pFirst,	rhs.xm_aNodes[i].pFirst);
				std::swap		(xm_aNodes[i].pLast,	rhs.xm_aNodes[i].pLast);
			}
			std::swap(xm_uSize, rhs.xm_uSize);
		}
	}

	template<std::size_t INDEX, typename... Params>
	void SetIndex(Node *pNode, Params &&... vParams)
		noexcept(
			std::is_nothrow_constructible<
				typename std::tuple_element<INDEX, typename Node::xIndexTuple>::type,
				Params...
			>::value &&
			std::is_nothrow_move_assignable<
				typename std::tuple_element<INDEX, typename Node::xIndexTuple>::type
			>::value
		)
	{
		typename std::tuple_element<INDEX, typename Node::xIndexTuple>::type
			vNewIndex(std::forward<Params>(vParams)...);

		xDetach<INDEX>(pNode);
		try {
			std::get<INDEX>(pNode->xm_vIndices) = std::move(vNewIndex);
			xAttach<INDEX>(pNode, nullptr);
		} catch(...){
			xAttach<INDEX>(pNode, nullptr);
			throw;
		}
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

	template<std::size_t INDEX, typename Comparand>
	const Node *GetLowerBound(const Comparand &vComparand) const noexcept {
		const auto pAvl = ::MCF_AvlLowerBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template NodeOther<Comparand>)
		);
		return pAvl ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}
	template<std::size_t INDEX, typename Comparand>
	Node *GetLowerBound(const Comparand &vComparand) noexcept {
		const auto pAvl = ::MCF_AvlLowerBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template NodeOther<Comparand>)
		);
		return pAvl ? DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}

	template<std::size_t INDEX, typename Comparand>
	const Node *GetUpperBound(const Comparand &vComparand) const noexcept {
		const auto pAvl = ::MCF_AvlUpperBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template OtherNode<Comparand>)
		);
		return pAvl ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}
	template<std::size_t INDEX, typename Comparand>
	Node *GetUpperBound(const Comparand &vComparand) noexcept {
		const auto pAvl = ::MCF_AvlUpperBound(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template OtherNode<Comparand>)
		);
		return pAvl ? DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}

	template<std::size_t INDEX, typename Comparand>
	const Node *Find(const Comparand &vComparand) const noexcept {
		const auto pAvl = ::MCF_AvlFind(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template NodeOther<Comparand>),
			&(xComparators<INDEX>::template OtherNode<Comparand>)
		);
		return pAvl ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}
	template<std::size_t INDEX, typename Comparand>
	Node *Find(const Comparand &vComparand) noexcept {
		const auto pAvl = ::MCF_AvlFind(
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template NodeOther<Comparand>),
			&(xComparators<INDEX>::template OtherNode<Comparand>)
		);
		return pAvl ? DOWN_CAST(Node, xm_aHeaders[INDEX], pAvl) : nullptr;
	}

	template<std::size_t INDEX, typename Comparand>
	std::pair<const Node *, const Node *> GetEqualRange(const Comparand &vComparand) const noexcept {
		MCF_AVL_NODE_HEADER *pBegin, *pEnd;
		::MCF_AvlEqualRange(
			&pBegin,
			&pEnd,
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template NodeOther<Comparand>),
			&(xComparators<INDEX>::template OtherNode<Comparand>)
		);
		return std::make_pair(
			pBegin ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pBegin) : nullptr,
			pEnd ? DOWN_CAST(const Node, xm_aHeaders[INDEX], pEnd) : nullptr
		);
	}
	template<std::size_t INDEX, typename Comparand>
	std::pair<Node *, Node *> GetEqualRange(const Comparand &vComparand) noexcept {
		MCF_AVL_NODE_HEADER *pBegin, *pEnd;
		::MCF_AvlEqualRange(
			&pBegin,
			&pEnd,
			&(xm_aNodes[INDEX].pRoot),
			(std::intptr_t)&vComparand,
			&(xComparators<INDEX>::template NodeOther<Comparand>),
			&(xComparators<INDEX>::template OtherNode<Comparand>)
		);
		return std::make_pair(
			pBegin ? DOWN_CAST(Node, xm_aHeaders[INDEX], pBegin) : nullptr,
			pEnd ? DOWN_CAST(Node, xm_aHeaders[INDEX], pEnd) : nullptr
		);
	}

public:
	typedef Node value_type;

	// std::insert_iterator
	template<typename Param>
	void insert(Node *pHint, Param &&vParam){
		InsertHint(pHint, std::forward<Param>(vParam));
	}
};

template<class... Indices>
using MultiIndexedSet = MultiIndexedMap<void, Indices...>;

}

#endif
