// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_MULTI_INDEXED_HPP__
#define __MCF_CRT_MULTI_INDEXED_HPP__

#include "../../c/ext/offset_of.h"
#include "../../env/avl_tree.h"
#include <tuple>
#include <type_traits>

namespace MCF {

template<typename Index_t, typename Comparer_t = std::less<Index_t>>
struct Index {
	Index_t m_vIndex;

	template<typename... Params_t>
	Index(Params_t &&... vParams)
		: m_vIndex(std::forward<Params_t>(vParams)...)
	{
	}

	bool operator<(const Index &rhs) const noexcept {
		return Comparer_t()(m_vIndex, rhs.m_vIndex);
	}
};

template<typename Index_t>
struct MakeIndex {
	typedef Index<Index_t> IndexType;
};

template<typename IndexParam_t>
struct MakeIndex<Index<IndexParam_t>> {
	typedef Index<IndexParam_t> IndexType;
};

template<class Element_t, class... Indexes_t>
class MultiIndexedMap {
public:
	enum : std::size_t {
		INDEX_COUNT = sizeof...(Indexes_t)
	};
	typedef typename std::tuple<typename MakeIndex<Indexes_t>::IndexType...> Indexes;

	static_assert(INDEX_COUNT > 0, "No index?");

	class Node {
		friend class MultiIndexedMap;

	private:
		Element_t xm_vElement;
		Indexes xm_vIndexes;

	private:
		__MCF_AVL_NODE_HEADER xm_arHeaders[INDEX_COUNT];

	public:
		template<typename ElementParam_t, typename FirstIndexParam_t, typename... IndexParams_t>
		Node(ElementParam_t &&vElementParam, FirstIndexParam_t &&vFirstIndexParam, IndexParams_t &&... vIndexParams)
			: xm_vElement(std::forward<ElementParam_t>(vElementParam))
			, xm_vIndexes(std::forward<FirstIndexParam_t>(vFirstIndexParam), std::forward<IndexParams_t>(vIndexParams)...)
		{
		}

	public:
		const Element_t &GetElement() const noexcept {
			return xm_vElement;
		}
		Element_t &GetElement() noexcept {
			return xm_vElement;
		}

		template<std::size_t INDEX>
		auto GetIndex() const noexcept -> const decltype(std::get<INDEX>(xm_vIndexes).m_vIndex) & {
			static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

			return std::get<INDEX>(xm_vIndexes).m_vIndex;
		}
	};

private:
	template<std::size_t INDEX>
	static const __MCF_AVL_NODE_HEADER *xAvlFromNode(const Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return &(pNode->xm_arHeaders[INDEX]);
	}
	template<std::size_t INDEX>
	static __MCF_AVL_NODE_HEADER *xAvlFromNode(Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return &(pNode->xm_arHeaders[INDEX]);
	}

	template<std::size_t INDEX>
	static const Node *xNodeFromAvl(const __MCF_AVL_NODE_HEADER *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return (const Node *)((const char *)pNode - OFFSET_OF(Node, xm_arHeaders[INDEX]));
	}
	template<std::size_t INDEX>
	static Node *xNodeFromAvl(__MCF_AVL_NODE_HEADER *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		if(!pNode){
			return nullptr;
		}
		return (Node *)((char *)pNode - OFFSET_OF(Node, xm_arHeaders[INDEX]));
	}

	template<std::size_t INDEX>
	static int xComparer(std::intptr_t lhs, std::intptr_t rhs) noexcept {
		typedef typename std::tuple_element<INDEX, Indexes>::type Index;
		return *(const Index *)lhs < *(const Index *)rhs;
	}

public:
	template<std::size_t INDEX>
	static const Node *Prev(const Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlPrev(xAvlFromNode<INDEX>(pNode)));
	}
	template<std::size_t INDEX>
	static Node *Prev(Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlPrev(xAvlFromNode<INDEX>(pNode)));
	}

	template<std::size_t INDEX>
	static const Node *Next(const Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlNext(xAvlFromNode<INDEX>(pNode)));
	}
	template<std::size_t INDEX>
	static Node *Next(Node *pNode) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlNext(xAvlFromNode<INDEX>(pNode)));
	}

public:
	struct {
		__MCF_AVL_NODE_HEADER *pFront;
		__MCF_AVL_NODE_HEADER *pRoot;
		__MCF_AVL_NODE_HEADER *pBack;
	} xm_arvIndexPointers[INDEX_COUNT];

public:
	MultiIndexedMap() noexcept {
		for(auto &vPointers : xm_arvIndexPointers){
			vPointers.pFront	= nullptr;
			vPointers.pRoot		= nullptr;
			vPointers.pBack		= nullptr;
		}
	}
	MultiIndexedMap(const MultiIndexedMap &rhs) noexcept
		: MultiIndexedMap()
	{
		xCloneRecur(rhs);
	}
	MultiIndexedMap &operator=(const MultiIndexedMap &rhs) noexcept {
		if(this != &rhs){
			Clear();
			xCloneRecur(rhs);
		}
		return *this;
	}
	MultiIndexedMap(MultiIndexedMap &&rhs) noexcept
		: MultiIndexedMap()
	{
		Swap(rhs);
	}
	MultiIndexedMap &operator=(MultiIndexedMap &&rhs) noexcept {
		Swap(rhs);
		return *this;
	}
	~MultiIndexedMap(){
		Clear();
	}

private:
	template<std::size_t INDEX>
	void xAttach(Node *pNode) noexcept {
		::__MCF_AvlAttachCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&std::get<INDEX>(pNode->xm_vIndexes),
			xAvlFromNode<INDEX>(pNode),
			&(xComparer<INDEX>)
		);

		const auto pAvl = xAvlFromNode<INDEX>(pNode);
		const auto pPrev = ::__MCF_AvlPrev(pAvl);
		if(!pPrev){
			xm_arvIndexPointers[INDEX].pFront = pAvl;
		}
		const auto pNext = ::__MCF_AvlNext(pAvl);
		if(!pNext){
			xm_arvIndexPointers[INDEX].pBack = pAvl;
		}
	}
	template<std::size_t INDEX>
	void xAttachAll(Node *pNode, typename std::enable_if<INDEX != (std::size_t)-1, int>::type = 0) noexcept {
		xAttach<INDEX>(pNode);
		xAttachAll<INDEX - 1>(pNode);
	}
	template<std::size_t INDEX>
	void xAttachAll(Node *, typename std::enable_if<INDEX == (std::size_t)-1, int>::type = 0) noexcept {
		// 空的。
	}

	template<std::size_t INDEX>
	void xDetach(Node *pNode) noexcept {
		const auto pAvl = xAvlFromNode<INDEX>(pNode);
		if(pAvl == xm_arvIndexPointers[INDEX].pFront){
			xm_arvIndexPointers[INDEX].pFront = ::__MCF_AvlNext(pAvl);
		}
		if(pAvl == xm_arvIndexPointers[INDEX].pBack){
			xm_arvIndexPointers[INDEX].pBack = ::__MCF_AvlPrev(pAvl);
		}

		::__MCF_AvlDetach(pAvl);
	}
	template<std::size_t INDEX>
	void xDetachAll(Node *pNode, typename std::enable_if<INDEX != (std::size_t)-1, int>::type = 0) noexcept {
		xDetach<INDEX>(pNode);
		xDetachAll<INDEX - 1>(pNode);
	}
	template<std::size_t INDEX>
	void xDetachAll(Node *, typename std::enable_if<INDEX == (std::size_t)-1, int>::type = 0) noexcept {
		// 空的。
	}

	void xCloneRecur(const MultiIndexedMap &rhs){
		ASSERT(this != &rhs);

		__MCF_AVL_NODE_HEADER *pAvl = rhs.xm_arvIndexPointers[0].pFront;
		while(pAvl){
			Node *const pNode = xNodeFromAvl<0>(pAvl);
			pAvl = ::__MCF_AvlNext(pAvl);
			Insert(*pNode);
		}
	}

public:
	template<typename... Params_t>
	Node *Insert(Params_t &&... vParams){
		const auto pNode = new Node(std::forward<Params_t>(vParams)...);
		xAttachAll<INDEX_COUNT - 1>(pNode);
		return pNode;
	}
	void Erase(Node *pNode) noexcept {
		xDetachAll<INDEX_COUNT - 1>(pNode);
		delete pNode;
	}
	void Clear() noexcept {
		__MCF_AVL_NODE_HEADER *pAvl = xm_arvIndexPointers[0].pFront;
		while(pAvl){
			Node *const pNode = xNodeFromAvl<0>(pAvl);
			pAvl = ::__MCF_AvlNext(pAvl);
			delete pNode;
		}

		for(auto &vPointers : xm_arvIndexPointers){
			vPointers.pFront	= nullptr;
			vPointers.pRoot		= nullptr;
			vPointers.pBack		= nullptr;
		}
	}

	void Swap(MultiIndexedMap &rhs) noexcept {
		if(this != &rhs){
			for(std::size_t i = 0; i < INDEX_COUNT; ++i){
				std::swap(
					xm_arvIndexPointers[i].pFront,
					rhs.xm_arvIndexPointers[i].pFront
				);
				::__MCF_AvlSwap(
					&(xm_arvIndexPointers[i].pRoot),
					&(rhs.xm_arvIndexPointers[i].pRoot)
				);
				std::swap(
					xm_arvIndexPointers[i].pBack,
					rhs.xm_arvIndexPointers[i].pBack
				);
			}
		}
	}

	template<std::size_t INDEX, typename... Params_t>
	void SetIndex(Node *pNode, Params_t &&... vParams){
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		auto vNewIndex = std::tuple_element<INDEX, Indexes>(std::forward<Params_t>(vParams)...);

		xDetach<INDEX>(pNode);
		try {
			std::get<INDEX>(pNode->xm_vIndexes) = std::move(vNewIndex);
			xAttach<INDEX>(pNode);
		} catch(...){
			xAttach<INDEX>(pNode);
			throw;
		}
	}

	template<std::size_t INDEX>
	const Node *Front() const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pFront);
	}
	template<std::size_t INDEX>
	Node *Front() noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pFront);
	}

	template<std::size_t INDEX>
	const Node *Back() const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pBack);
	}
	template<std::size_t INDEX>
	Node *Back() noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(xm_arvIndexPointers[INDEX].pBack);
	}

	template<std::size_t INDEX>
	const Node *LowerBound(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlLowerBoundCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		));
	}
	template<std::size_t INDEX>
	Node *LowerBound(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlLowerBoundCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		));
	}

	template<std::size_t INDEX>
	const Node *UpperBound(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlUpperBoundCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		));
	}
	template<std::size_t INDEX>
	Node *UpperBound(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlUpperBoundCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		));
	}

	template<std::size_t INDEX>
	const Node *Find(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlFindCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		));
	}
	template<std::size_t INDEX>
	Node *Find(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		return xNodeFromAvl<INDEX>(::__MCF_AvlFindCustomComp(
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		));
	}

	template<std::size_t INDEX>
	std::pair<const Node *, const Node *> EqualRange(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) const noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		__MCF_AVL_NODE_HEADER *pFrom, *pTo;
		::__MCF_AvlEqualRangeCustomComp(
			&pFrom,
			&pTo,
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		);
		return std::make_pair(
			xNodeFromAvl<INDEX>(pFrom),
			xNodeFromAvl<INDEX>(pTo)
		);
	}
	template<std::size_t INDEX>
	std::pair<Node *, Node *> EqualRange(const typename std::tuple_element<INDEX, Indexes>::type &vIndex) noexcept {
		static_assert(INDEX < INDEX_COUNT, "INDEX is out of range.");

		__MCF_AVL_NODE_HEADER *pFrom, *pTo;
		::__MCF_AvlEqualRangeCustomComp(
			&pFrom,
			&pTo,
			&(xm_arvIndexPointers[INDEX].pRoot),
			(std::intptr_t)&vIndex,
			&(xComparer<INDEX>)
		);
		return std::make_pair(
			xNodeFromAvl<INDEX>(pFrom),
			xNodeFromAvl<INDEX>(pTo)
		);
	}
};

}

#endif
