// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_MULTI_INDEX_MAP_HPP_
#define MCF_CONTAINERS_MULTI_INDEX_MAP_HPP_

#include "../../MCFCRT/env/avl_tree.h"
#include "../Utilities/Assert.hpp"
#include <utility>
#include <tuple>
#include <type_traits>
#include <cstdlib>

namespace MCF {

namespace Impl {
	template<std::size_t INDEX_ID_T, class NodeComparatorT>
	class MultiOrderedMapIndex;

	template<std::size_t INDEX_ID_T, class NodeComparatorT>
	class UniqueOrderedMapIndex;

	template<std::size_t INDEX_ID_T>
	class OrderedMapIndexNode : private ::MCF_AvlNodeHeader {
		template<std::size_t, class>
		friend class MultiOrderedMapIndex;

		template<std::size_t, class>
		friend class UniqueOrderedMapIndex;

	public:
		const OrderedMapIndexNode *GetPrev() const noexcept {
			return static_cast<const OrderedMapIndexNode *>(::MCF_AvlPrev(this));
		}
		OrderedMapIndexNode *GetPrev() noexcept {
			return static_cast<OrderedMapIndexNode *>(::MCF_AvlPrev(this));
		}
		const OrderedMapIndexNode *GetNext() const noexcept {
			return static_cast<const OrderedMapIndexNode *>(::MCF_AvlNext(this));
		}
		OrderedMapIndexNode *GetNext() noexcept {
			return static_cast<OrderedMapIndexNode *>(::MCF_AvlNext(this));
		}
	};

	template<class NodeT, class ElementT, std::size_t INDEX_ID_T, class IndexNodeT, class ComparatorT>
	struct OrderedMapIndexNodeComparator {
		using MapIndexNode = OrderedMapIndexNode<INDEX_ID_T>;

		static auto GetElement(const MapIndexNode *pMapIndexNode) noexcept {
			return static_cast<const ElementT *>(NodeT::template GetNodeFromIndexNode<INDEX_ID_T>(pMapIndexNode));
		}

		bool operator()(const MapIndexNode *lhs, const MapIndexNode *rhs) const noexcept {
			return ComparatorT()(*GetElement(lhs), *GetElement(rhs));
		}
		template<typename ComparandT>
		bool operator()(const MapIndexNode *lhs, const ComparandT *rhs) const noexcept {
			return ComparatorT()(*GetElement(lhs), *rhs);
		}
		template<typename ComparandT>
		bool operator()(const ComparandT *lhs, const MapIndexNode *rhs) const noexcept {
			return ComparatorT()(*lhs, *GetElement(rhs));
		}
	};

	template<class ElementT, class KeyTypeT, KeyTypeT ElementT::*PTR_TO_KEY_T, class ComparatorT>
	struct MemberComparator {
		bool operator()(const ElementT &lhs, const ElementT &rhs) const noexcept {
			return ComparatorT()(lhs.*PTR_TO_KEY_T, rhs.*PTR_TO_KEY_T);
		}
		template<typename ComparandT>
		bool operator()(const ElementT &lhs, const ComparandT &rhs) const noexcept {
			return ComparatorT()(lhs.*PTR_TO_KEY_T, rhs);
		}
		template<typename ComparandT>
		bool operator()(const ComparandT &lhs, const ElementT &rhs) const noexcept {
			return ComparatorT()(lhs, rhs.*PTR_TO_KEY_T);
		}
	};

	template<std::size_t INDEX_ID_T, class NodeComparatorT>
	class MultiOrderedMapIndex {
	public:
		using IndexNode = OrderedMapIndexNode<INDEX_ID_T>;

	private:
		::MCF_AvlRoot xm_avlRoot;
		IndexNode *xm_pFirst;
		IndexNode *xm_pLast;

	public:
		constexpr MultiOrderedMapIndex() noexcept
			: xm_avlRoot(nullptr), xm_pFirst(nullptr), xm_pLast(nullptr)
		{
		}

	public:
		void Clear() noexcept {
			xm_avlRoot = nullptr;
			xm_pFirst = nullptr;
			xm_pLast = nullptr;
		}

		const IndexNode *GetFirst() const noexcept {
			return xm_pFirst;
		}
		IndexNode *GetFirst() noexcept {
			return xm_pFirst;
		}
		const IndexNode *GetLast() const noexcept {
			return xm_pLast;
		}
		IndexNode *GetLast() noexcept {
			return xm_pLast;
		}

		IndexNode *Attach(IndexNode *pPos, IndexNode *pIndexNode) noexcept {
			::MCF_AvlAttachHint(&xm_avlRoot, pPos, pIndexNode,
				[](const ::MCF_AvlNodeHeader *lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			);
			if(!pIndexNode->GetPrev()){
				xm_pFirst = pIndexNode;
			}
			if(!pIndexNode->GetNext()){
				xm_pLast = pIndexNode;
			}
			return nullptr;
		}
		void Detach(IndexNode *pIndexNode) noexcept {
			if(xm_pFirst == pIndexNode){
				xm_pFirst = pIndexNode->GetNext();
			}
			if(xm_pLast == pIndexNode){
				xm_pLast = pIndexNode->GetPrev();
			}
			::MCF_AvlDetach(pIndexNode);
		}

		template<typename ComparandT>
		const IndexNode *GetLowerBound(const ComparandT *pComparand) const noexcept {
			return static_cast<const IndexNode *>(::MCF_AvlLowerBound(&xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}
		template<typename ComparandT>
		IndexNode *GetLowerBound(const ComparandT *pComparand) noexcept {
			return static_cast<IndexNode *>(::MCF_AvlLowerBound(&xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}

		template<typename ComparandT>
		const IndexNode *GetUpperBound(const ComparandT *pComparand) const noexcept {
			return static_cast<const IndexNode *>(::MCF_AvlUpperBound(&xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(reinterpret_cast<const ComparandT *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			));
		}
		template<typename ComparandT>
		IndexNode *GetUpperBound(const ComparandT *pComparand) noexcept {
			return static_cast<IndexNode *>(::MCF_AvlUpperBound(&xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(reinterpret_cast<const ComparandT *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			));
		}

		template<typename ComparandT>
		const IndexNode *Find(const ComparandT *pComparand) const noexcept {
			return static_cast<const IndexNode *>(::MCF_AvlFind(&xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), reinterpret_cast<const ComparandT *>(rhs));
				},
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(reinterpret_cast<const ComparandT *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			));
		}
		template<typename ComparandT>
		IndexNode *Find(const ComparandT *pComparand) noexcept {
			return static_cast<IndexNode *>(::MCF_AvlFind(&xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), reinterpret_cast<const ComparandT *>(rhs));
				},
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(reinterpret_cast<const ComparandT *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			));
		}

		template<typename ComparandT>
		std::pair<const IndexNode *, const IndexNode *> GetEqualRange(const ComparandT *pComparand) const noexcept {
			::MCF_AvlNodeHeader *pBegin, *pEnd;
			::MCF_AvlEqualRange(&pBegin, &pEnd, &xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), reinterpret_cast<const ComparandT *>(rhs));
				},
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(reinterpret_cast<const ComparandT *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			);
			return std::make_pair(static_cast<const IndexNode *>(pBegin), static_cast<const IndexNode *>(pEnd));
		}
		template<typename ComparandT>
		std::pair<IndexNode *, IndexNode *> GetEqualRange(const ComparandT *pComparand) noexcept {
			::MCF_AvlNodeHeader *pBegin, *pEnd;
			::MCF_AvlEqualRange(&pBegin, &pEnd, &xm_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return NodeComparatorT()(static_cast<const IndexNode *>(lhs), reinterpret_cast<const ComparandT *>(rhs));
				},
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return NodeComparatorT()(reinterpret_cast<const ComparandT *>(lhs), static_cast<const IndexNode *>(rhs));
				}
			);
			return std::make_pair(static_cast<IndexNode *>(pBegin), static_cast<IndexNode *>(pEnd));
		}
	};

	template<std::size_t INDEX_ID_T, class NodeComparatorT>
	class UniqueOrderedMapIndex {
	public:
		using IndexNode = OrderedMapIndexNode<INDEX_ID_T>;

	private:
		MultiOrderedMapIndex<INDEX_ID_T, NodeComparatorT> xm_vDelegate;

	public:
		void Clear() noexcept {
			xm_vDelegate.Clear();
		}

		const IndexNode *GetFirst() const noexcept {
			return xm_vDelegate.GetFirst();
		}
		IndexNode *GetFirst() noexcept {
			return xm_vDelegate.GetFirst();
		}
		const IndexNode *GetLast() const noexcept {
			return xm_vDelegate.GetLast();
		}
		IndexNode *GetLast() noexcept {
			return xm_vDelegate.GetLast();
		}

		IndexNode *Attach(IndexNode *pPos, IndexNode *pIndexNode) noexcept {
			auto pHint = xm_vDelegate.GetLowerBound(pIndexNode);
			if(pHint){	// *pHint >= *pIndexNode
				if(!NodeComparatorT()(pIndexNode, pHint)){	// *pIndexNode >= *pHint
					return pHint;
				}
			} else {
				pHint = pPos;
			}
			return xm_vDelegate.Attach(pHint, pIndexNode);
		}
		void Detach(IndexNode *pIndexNode) noexcept {
			xm_vDelegate.Detach(pIndexNode);
		}

		template<typename ComparandT>
		const IndexNode *GetLowerBound(const ComparandT *pComparand) const noexcept {
			return xm_vDelegate.GetLowerBound(pComparand);
		}
		template<typename ComparandT>
		IndexNode *GetLowerBound(const ComparandT *pComparand) noexcept {
			return xm_vDelegate.GetLowerBound(pComparand);
		}

		template<typename ComparandT>
		const IndexNode *GetUpperBound(const ComparandT *pComparand) const noexcept {
			return xm_vDelegate.GetUpperBound(pComparand);
		}
		template<typename ComparandT>
		IndexNode *GetUpperBound(const ComparandT *pComparand) noexcept {
			return xm_vDelegate.GetUpperBound(pComparand);
		}

		template<typename ComparandT>
		const IndexNode *Find(const ComparandT *pComparand) const noexcept {
			return xm_vDelegate.Find(pComparand);
		}
		template<typename ComparandT>
		IndexNode *Find(const ComparandT *pComparand) noexcept {
			return xm_vDelegate.Find(pComparand);
		}

		template<typename ComparandT>
		std::pair<const IndexNode *, const IndexNode *> GetEqualRange(const ComparandT *pComparand) const noexcept {
			return xm_vDelegate.GetEqualRange(pComparand);
		}
		template<typename ComparandT>
		std::pair<IndexNode *, IndexNode *> GetEqualRange(const ComparandT *pComparand) noexcept {
			return xm_vDelegate.GetEqualRange(pComparand);
		}
	};

	template<std::size_t INDEX_ID_T>
	class SequencedMapIndexNode {
		template<std::size_t>
		friend class SequencedMapIndex;

	private:
		SequencedMapIndexNode *xm_pPrev;
		SequencedMapIndexNode *xm_pNext;

	public:
		const SequencedMapIndexNode *GetPrev() const noexcept {
			return xm_pPrev;
		}
		SequencedMapIndexNode *GetPrev() noexcept {
			return xm_pPrev;
		}
		const SequencedMapIndexNode *GetNext() const noexcept {
			return xm_pNext;
		}
		SequencedMapIndexNode *GetNext() noexcept {
			return xm_pNext;
		}
	};

	template<std::size_t INDEX_ID_T>
	class SequencedMapIndex {
	public:
		using IndexNode = SequencedMapIndexNode<INDEX_ID_T>;

	private:
		IndexNode *xm_pFirst;
		IndexNode *xm_pLast;

	public:
		constexpr SequencedMapIndex() noexcept
			: xm_pFirst(nullptr), xm_pLast(nullptr)
		{
		}

	public:
		void Clear() noexcept {
			xm_pFirst = nullptr;
			xm_pLast = nullptr;
		}

		const IndexNode *GetFirst() const noexcept {
			return xm_pFirst;
		}
		IndexNode *GetFirst() noexcept {
			return xm_pFirst;
		}
		const IndexNode *GetLast() const noexcept {
			return xm_pLast;
		}
		IndexNode *GetLast() noexcept {
			return xm_pLast;
		}

		IndexNode *Attach(IndexNode *pPos, IndexNode *pIndexNode) noexcept {
			const auto pPrev = std::exchange((pPos ? pPos->xm_pPrev : xm_pLast), pIndexNode);
			(pPrev ? pPrev->xm_pNext : xm_pFirst) = pIndexNode;
			pIndexNode->xm_pPrev = pPrev;
			pIndexNode->xm_pNext = pPos;
			return nullptr;
		}
		void Detach(IndexNode *pIndexNode) noexcept {
			const auto pNext = pIndexNode->xm_pNext;
			const auto pOldPrev = pIndexNode->xm_pPrev;
			(pOldPrev ? pOldPrev->xm_pNext : xm_pFirst) = pNext;
			(pNext ? pNext->xm_pPrev : xm_pLast) = pOldPrev;
		}
	};
}

template<class ElementT,
	class ComparatorT = std::less<void>>
struct MultiOrderedIndex {
	template<std::size_t INDEX_ID_T>
	using IndexNode = Impl::OrderedMapIndexNode<INDEX_ID_T>;

	template<class NodeT, std::size_t INDEX_ID_T>
	using IndexType = Impl::MultiOrderedMapIndex<INDEX_ID_T,
		Impl::OrderedMapIndexNodeComparator<
			NodeT, ElementT, INDEX_ID_T, IndexNode<INDEX_ID_T>, ComparatorT>
		>;

	static ElementT &GetKey(ElementT &vElement) noexcept {
		return vElement;
	}
};

template<class ElementT,
	class ComparatorT = std::less<void>>
struct UniqueOrderedIndex {
	template<std::size_t INDEX_ID_T>
	using IndexNode = Impl::OrderedMapIndexNode<INDEX_ID_T>;

	template<class NodeT, std::size_t INDEX_ID_T>
	using IndexType = Impl::UniqueOrderedMapIndex<INDEX_ID_T,
		Impl::OrderedMapIndexNodeComparator<
			NodeT, ElementT, INDEX_ID_T, IndexNode<INDEX_ID_T>, ComparatorT>
		>;

	static ElementT &GetKey(ElementT &vElement) noexcept {
		return vElement;
	}
};

template<class ElementT, class KeyTypeT, KeyTypeT ElementT::*PTR_TO_KEY_T,
	class ComparatorT = std::less<void>>
struct MultiOrderedMemberIndex {
	template<std::size_t INDEX_ID_T>
	using IndexNode = Impl::OrderedMapIndexNode<INDEX_ID_T>;

	template<class NodeT, std::size_t INDEX_ID_T>
	using IndexType = Impl::MultiOrderedMapIndex<INDEX_ID_T,
		Impl::OrderedMapIndexNodeComparator<
			NodeT, ElementT, INDEX_ID_T, IndexNode<INDEX_ID_T>,
			Impl::MemberComparator<ElementT, KeyTypeT, PTR_TO_KEY_T, ComparatorT>>
		>;

	static KeyTypeT &GetKey(ElementT &vElement) noexcept {
		return vElement.*PTR_TO_KEY_T;
	}
};

template<class ElementT, class KeyTypeT, KeyTypeT ElementT::*PTR_TO_KEY_T,
	class ComparatorT = std::less<void>>
struct UniqueOrderedMemberIndex {
	template<std::size_t INDEX_ID_T>
	using IndexNode = Impl::OrderedMapIndexNode<INDEX_ID_T>;

	template<class NodeT, std::size_t INDEX_ID_T>
	using IndexType = Impl::UniqueOrderedMapIndex<INDEX_ID_T,
		Impl::OrderedMapIndexNodeComparator<
			NodeT, ElementT, INDEX_ID_T, IndexNode<INDEX_ID_T>,
			Impl::MemberComparator<ElementT, KeyTypeT, PTR_TO_KEY_T, ComparatorT>>
		>;

	static KeyTypeT &GetKey(ElementT &vElement) noexcept {
		return vElement.*PTR_TO_KEY_T;
	}
};

template<class ElementT>
struct SequencedIndex {
	template<std::size_t INDEX_ID_T>
	using IndexNode = Impl::SequencedMapIndexNode<INDEX_ID_T>;

	template<class NodeT, std::size_t INDEX_ID_T>
	using IndexType = Impl::SequencedMapIndex<INDEX_ID_T>;

	static ElementT &GetKey(ElementT &vElement) noexcept {
		return vElement;
	}
};

template<class ElementT, typename ...IndicesT>
class MultiIndexMap {
private:
	struct xAddressNode
		: public ::MCF_AvlNodeHeader
	{
	};

	template<std::size_t INDEX_ID_T>
	using xIndexNode = typename std::tuple_element_t<INDEX_ID_T, std::tuple<IndicesT...>>::template IndexNode<INDEX_ID_T>;

	template<std::size_t ...INDEX_IDS_T>
	class xNodeImpl
		: public ElementT, private IndicesT::template IndexNode<INDEX_IDS_T>...
		, private xAddressNode	// 复制树结构的时候用到。
	{
		friend MultiIndexMap;

	public:
		template<std::size_t INDEX_ID_T>
		static auto GetNodeFromIndexNode(const xIndexNode<INDEX_ID_T> *pIndexNode) noexcept {
			return static_cast<const xNodeImpl *>(pIndexNode);
		}

	private:
		// 这个成员在复制树结构的时候用到。
		const xNodeImpl *xm_pSource;

	public:
		template<typename ...ParamsT>
		explicit xNodeImpl(ParamsT &&...vParams)
			: ElementT(std::forward<ParamsT>(vParams)...)
		{
		}

	public:
		template<std::size_t INDEX_ID_T>
		const xNodeImpl *GetPrev() const noexcept {
			using IndexNode = xIndexNode<INDEX_ID_T>;

			return static_cast<const xNodeImpl *>(static_cast<const IndexNode *>(
				static_cast<const IndexNode *>(this)->GetPrev()));
		}
		template<std::size_t INDEX_ID_T>
		xNodeImpl *GetPrev() noexcept {
			using IndexNode = xIndexNode<INDEX_ID_T>;

			return static_cast<xNodeImpl *>(static_cast<IndexNode *>(
				static_cast<IndexNode *>(this)->GetPrev()));
		}
		template<std::size_t INDEX_ID_T>
		const xNodeImpl *GetNext() const noexcept {
			using IndexNode = xIndexNode<INDEX_ID_T>;

			return static_cast<const xNodeImpl *>(static_cast<const IndexNode *>(
				static_cast<const IndexNode *>(this)->GetNext()));
		}
		template<std::size_t INDEX_ID_T>
		xNodeImpl *GetNext() noexcept {
			using IndexNode = xIndexNode<INDEX_ID_T>;

			return static_cast<xNodeImpl *>(static_cast<IndexNode *>(
				static_cast<IndexNode *>(this)->GetNext()));
		}
	};

	template<std::size_t ...INDEX_IDS_T>
	static auto xMakeNode(std::index_sequence<INDEX_IDS_T...>) noexcept -> xNodeImpl<INDEX_IDS_T...>;

	template<class NodeT, std::size_t ...INDEX_IDS_T>
	static auto xMakeIndices(std::index_sequence<INDEX_IDS_T...>) noexcept ->
		std::tuple<typename IndicesT::template IndexType<NodeT, INDEX_IDS_T>...>;

public:
	using Node = decltype(xMakeNode(std::index_sequence_for<IndicesT...>()));
	using Hints = std::tuple<decltype(reinterpret_cast<Node *>(static_cast<IndicesT *>(nullptr)))...>;

private:
	using xIndexTuple = decltype(xMakeIndices<Node>(std::index_sequence_for<IndicesT...>()));

private:
	template<typename, typename FirstT, typename ...RemainingT>
	static void xUpdateHints(Hints &vHints, Node *pNode) noexcept {
		constexpr std::size_t INDEX_ID = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;

		auto &pHint = std::get<INDEX_ID>(vHints);
		if(pHint == pNode){
			pHint = pNode->template GetNext<INDEX_ID>();
		}

		xUpdateHints<int, RemainingT...>(vHints, pNode);
	}
	template<typename>
	static void xUpdateHints(Hints &, Node *) noexcept {
	}

private:
	xIndexTuple xm_vIndices;
	std::size_t xm_uSize;

public:
	constexpr MultiIndexMap() noexcept
		: xm_vIndices(), xm_uSize(0)
	{
	}
	MultiIndexMap(const MultiIndexMap &rhs)
		: MultiIndexMap()
	{
		// 稳定地复制树结构。

		// 第一步，分配全部新节点。
		::MCF_AvlRoot avlAddressMap = nullptr;
		try {
			auto pNode = rhs.GetFirst<0>();
			while(pNode){
				const auto pNewNode = new Node(*pNode);
				pNewNode->xm_pSource = pNode;
				::MCF_AvlAttach(&avlAddressMap, static_cast<xAddressNode *>(pNewNode),
					[](const ::MCF_AvlNodeHeader *lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
						return std::less<void>()(
							static_cast<const Node *>(
								static_cast<const xAddressNode *>(lhs))->xm_pSource,
							static_cast<const Node *>(
								static_cast<const xAddressNode *>(rhs))->xm_pSource);
					}
				);
				pNode = pNode->template GetNext<0>();
			}
		} catch(...){
			if(avlAddressMap){
				auto pCur = ::MCF_AvlPrev(avlAddressMap);
				while(pCur){
					const auto pPrev = ::MCF_AvlPrev(pCur);
					delete static_cast<Node *>(static_cast<xAddressNode *>(pCur));
					pCur = pPrev;
				}
				pCur = ::MCF_AvlNext(avlAddressMap);
				while(pCur){
					const auto pNext = ::MCF_AvlNext(pCur);
					delete static_cast<Node *>(static_cast<xAddressNode *>(pCur));
					pCur = pNext;
				}
				delete static_cast<Node *>(static_cast<xAddressNode *>(avlAddressMap));
			}
			throw;
		}

		// 所有节点都分配完成，现在进行第二步，重建每个二叉树，保证无异常抛出。
		xCloneAll<int, IndicesT...>(rhs.xm_vIndices, avlAddressMap);
		xm_uSize = rhs.xm_uSize;
	}
	MultiIndexMap(MultiIndexMap &&rhs) noexcept
		: MultiIndexMap()
	{
		Swap(rhs);
	}
	MultiIndexMap &operator=(const MultiIndexMap &rhs){
		if(&rhs != this){
			MultiIndexMap(rhs).Swap(*this);
		}
		return *this;
	}
	MultiIndexMap &operator=(MultiIndexMap &&rhs) noexcept {
		Swap(rhs);
		return *this;
	}
	~MultiIndexMap(){
		Clear();
	}

private:
	template<typename, typename FirstT, typename ...RemainingT>
	Node *xAttachAll(const Hints &vHints, Node *pNode) noexcept {
		constexpr std::size_t INDEX_ID = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;
		using IndexNode = xIndexNode<INDEX_ID>;

		const auto pHintIndexNode = static_cast<IndexNode *>(std::get<INDEX_ID>(vHints));
		const auto pIndexNode = static_cast<IndexNode *>(pNode);
		const auto pExistentIndexNode = std::get<INDEX_ID>(xm_vIndices).Attach(pHintIndexNode, pIndexNode);
		if(pExistentIndexNode){
			// 当前的插入操作失败了，返回现存的节点。
			return static_cast<Node *>(static_cast<IndexNode *>(pExistentIndexNode));
		}
		const auto pResult = xAttachAll<int, RemainingT...>(vHints, pNode);
		if(pResult){
			// 刚刚的插入操作失败了，撤销刚才的操作。
			std::get<INDEX_ID>(xm_vIndices).Detach(pIndexNode);
		}
		return pResult;
	}
	template<typename>
	Node *xAttachAll(const Hints &, Node *) noexcept {
		return nullptr;
	}

	template<std::size_t EXCEPT_ID_T, typename FirstT, typename ...RemainingT>
	void xDetachAll(Node *pNode) noexcept {
		constexpr std::size_t INDEX_ID = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;
		using IndexNode = xIndexNode<INDEX_ID>;

		if(INDEX_ID != EXCEPT_ID_T){
			const auto pIndexNode = static_cast<IndexNode *>(pNode);
			std::get<INDEX_ID>(xm_vIndices).Detach(pIndexNode);
		}
		xDetachAll<EXCEPT_ID_T, RemainingT...>(pNode);
	}
	template<std::size_t EXCEPT_ID_T>
	void xDetachAll(Node *) noexcept {
	}

	template<typename, typename FirstT, typename ...RemainingT>
	void xClearAll() noexcept {
		constexpr std::size_t INDEX_ID = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;

		std::get<INDEX_ID>(xm_vIndices).Clear();
		xClearAll<int, RemainingT...>();
	}
	template<typename>
	void xClearAll() noexcept {
	}

	template<typename, typename FirstT, typename ...RemainingT>
	void xCloneAll(const xIndexTuple &vStructure, ::MCF_AvlRoot avlNewNodes) noexcept {
		constexpr std::size_t INDEX_ID = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;
		using IndexNode = xIndexNode<INDEX_ID>;

		IndexNode *pHintIndexNode = nullptr;

		// 逆序复制，这样我们可以保证除了第一次以外每次拿到的 pHintIndexNode 都是非空的。
		auto pSourceIndexNode = std::get<INDEX_ID>(vStructure).GetLast();
		while(pSourceIndexNode){
			const auto pSourceNode = static_cast<const Node *>(pSourceIndexNode);
			const auto pNode = static_cast<Node *>(static_cast<xAddressNode *>(::MCF_AvlFind(
				&avlNewNodes, reinterpret_cast<std::intptr_t>(pSourceNode),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs) noexcept -> bool {
					return std::less<void>()(
						static_cast<const Node *>(
							static_cast<const xAddressNode *>(lhs))->xm_pSource,
						reinterpret_cast<const Node *>(rhs));
				},
				[](std::intptr_t lhs, const ::MCF_AvlNodeHeader *rhs) noexcept -> bool {
					return std::less<void>()(
						reinterpret_cast<const Node *>(lhs),
						static_cast<const Node *>(
							static_cast<const xAddressNode *>(rhs))->xm_pSource);
				}
			)));
			ASSERT(pNode);

			const auto pIndexNode = static_cast<IndexNode *>(pNode);
#ifdef NDEBUG
			std::get<INDEX_ID>(xm_vIndices).Attach(pHintIndexNode, pIndexNode);
#else
			const auto pExistentIndexNode = std::get<INDEX_ID>(xm_vIndices).Attach(pHintIndexNode, pIndexNode);
			ASSERT(!pExistentIndexNode);
#endif

			pHintIndexNode = pIndexNode;
			pSourceIndexNode = pSourceIndexNode->GetPrev();
		}

		xCloneAll<int, RemainingT...>(vStructure, avlNewNodes);
	}
	template<typename>
	void xCloneAll(const xIndexTuple &, ::MCF_AvlRoot) noexcept {
	}

public:
	bool IsEmpty() const noexcept {
		return xm_uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return xm_uSize;
	}
	void Clear() noexcept {
		auto pNode = GetFirst<0>();
		while(pNode){
			const auto pNext = pNode->template GetNext<0>();
			delete pNode;
			pNode = pNext;
		}
		xClearAll<int, IndicesT...>();
		xm_uSize = 0;
	}

	template<typename ...ParamsT>
	std::pair<Node *, bool> Insert(bool bOverwrites, ParamsT &&...vParams){
		return InsertWithHints(bOverwrites, Hints(), std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	std::pair<Node *, bool> InsertWithHints(bool bOverwrites, const Hints &vHints, ParamsT &&...vParams){
		const auto pNode = new Node(std::forward<ParamsT>(vParams)...);
		Node *pExistent = xAttachAll<int, IndicesT...>(vHints, pNode);
		if(pExistent){
			if(!bOverwrites){
				delete pNode;
				return std::make_pair(pExistent, false);
			}
			Hints vNewHints(vHints);
			do {
				xUpdateHints<int, IndicesT...>(vNewHints, pExistent);
				Erase(pExistent);
				pExistent = xAttachAll<int, IndicesT...>(vNewHints, pNode);
			} while(pExistent);
		}
		++xm_uSize;
		return std::make_pair(pNode, true);
	}
	void Erase(Node *pNode) noexcept {
		xDetachAll<(std::size_t)-1, IndicesT...>(pNode);
		delete pNode;
		--xm_uSize;
	}

	template<typename ModifierT>
	Node *Modify(bool bOverwrites, Node *pNode, ModifierT &&vModifier){
		return ModifyWithHints(bOverwrites, Hints(), pNode, std::forward<ModifierT>(vModifier));
	}
	template<typename ModifierT>
	Node *ModifyWithHints(bool bOverwrites, const Hints &vHints, Node *pNode, ModifierT &&vModifier){
		xDetachAll<(std::size_t)-1, IndicesT...>(pNode);
		try {
			std::forward<ModifierT>(vModifier)(*pNode);
		} catch(...){
			delete pNode;
			--xm_uSize;
			throw;
		}

		Node *pExistent = xAttachAll<int, IndicesT...>(vHints, pNode);
		if(pExistent){
			if(!bOverwrites){
				delete pNode;
				--xm_uSize;
				return nullptr;
			}
			Hints vNewHints(vHints);
			do {
				xUpdateHints<int, IndicesT...>(vNewHints, pExistent);
				Erase(pExistent);
				pExistent = xAttachAll<int, IndicesT...>(vNewHints, pNode);
			} while(pExistent);
		}
		return pNode;
	}

	template<typename ...ParamsT>
	Node *Replace(bool bOverwrites, Node *pNode, ParamsT &&...vParams){
		return ReplaceWithHints(bOverwrites, Hints(), pNode, std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	Node *ReplaceWithHints(bool bOverwrites, const Hints &vHints, Node *pNode, ParamsT &&...vParams){
		return ModifyWithHints(bOverwrites, vHints, pNode,
			[&](auto &vNode){ vNode = Node(std::forward<ParamsT>(vParams)...); });
	}

	template<std::size_t INDEX_ID_T, typename ...ParamsT>
	bool SetKey(bool bOverwrites, Node *pNode, ParamsT &&...vParams){
		return SetKeyWithHint<INDEX_ID_T>(bOverwrites, nullptr, pNode, std::forward<ParamsT>(vParams)...);
	}
	template<std::size_t INDEX_ID_T, typename ...ParamsT>
	bool SetKeyWithHint(bool bOverwrites, Node *pHint, Node *pNode, ParamsT &&...vParams){
		using Index = std::tuple_element_t<INDEX_ID_T, std::tuple<IndicesT...>>;
		using IndexNode = typename Index::template IndexNode<INDEX_ID_T>;

		Index::GetKey(*static_cast<ElementT *>(pNode)) =
			std::remove_reference_t<decltype(Index::GetKey(std::declval<ElementT &>()))>(
				std::forward<ParamsT>(vParams)...);

		const auto pIndexNode = static_cast<IndexNode *>(pNode);
		std::get<INDEX_ID_T>(xm_vIndices).Detach(pIndexNode);
		auto pHintIndexNode = static_cast<IndexNode *>(pHint);
		auto pExistentIndexNode = std::get<INDEX_ID_T>(xm_vIndices).Attach(pHintIndexNode, pIndexNode);
		if(pExistentIndexNode){
			if(!bOverwrites){
				xDetachAll<INDEX_ID_T, IndicesT...>(pNode);
				delete pNode;
				--xm_uSize;
				return false;
			}
			pHintIndexNode = static_cast<IndexNode *>(pExistentIndexNode->GetNext());
			Erase(static_cast<Node *>(static_cast<IndexNode *>(pExistentIndexNode)));
			pExistentIndexNode = std::get<INDEX_ID_T>(xm_vIndices).Attach(pHintIndexNode, pIndexNode);
			ASSERT(!pExistentIndexNode);
		}
		return true;
	}

	template<std::size_t INDEX_ID_T>
	const Node *GetFirst() const noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<const Node *>(static_cast<const IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetFirst()));
	}
	template<std::size_t INDEX_ID_T>
	Node *GetFirst() noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<Node *>(static_cast<IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetFirst()));
	}
	template<std::size_t INDEX_ID_T>
	const Node *GetLast() const noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<const Node *>(static_cast<const IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetLast()));
	}
	template<std::size_t INDEX_ID_T>
	Node *GetLast() noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<Node *>(static_cast<IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetLast()));
	}

	void Swap(MultiIndexMap &rhs) noexcept {
		std::swap(xm_vIndices, rhs.xm_vIndices);
		std::swap(xm_uSize, rhs.xm_uSize);
	}

	template<std::size_t INDEX_ID_T, typename ComparandT>
	const Node *GetLowerBound(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<const Node *>(static_cast<const IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetLowerBound(&vComparandT)));
	}
	template<std::size_t INDEX_ID_T, typename ComparandT>
	Node *GetLowerBound(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<Node *>(static_cast<IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetLowerBound(&vComparandT)));
	}

	template<std::size_t INDEX_ID_T, typename ComparandT>
	const Node *GetUpperBound(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<const Node *>(static_cast<const IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetUpperBound(&vComparandT)));
	}
	template<std::size_t INDEX_ID_T, typename ComparandT>
	Node *GetUpperBound(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<Node *>(static_cast<IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).GetUpperBound(&vComparandT)));
	}

	template<std::size_t INDEX_ID_T, typename ComparandT>
	const Node *Find(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<const Node *>(static_cast<const IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).Find(&vComparandT)));
	}
	template<std::size_t INDEX_ID_T, typename ComparandT>
	Node *Find(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		return static_cast<Node *>(static_cast<IndexNode *>(
			std::get<INDEX_ID_T>(xm_vIndices).Find(&vComparandT)));
	}

	template<std::size_t INDEX_ID_T, typename ComparandT>
	std::pair<const Node *, const Node *> GetEqualRange(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		const auto vRange = std::get<INDEX_ID_T>(xm_vIndices).GetEqualRange(&vComparandT);
		return std::make_pair(static_cast<const Node *>(static_cast<const IndexNode *>(vRange.first)),
			static_cast<const Node *>(static_cast<const IndexNode *>(vRange.second)));
	}
	template<std::size_t INDEX_ID_T, typename ComparandT>
	std::pair<Node *, Node *> GetEqualRange(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<INDEX_ID_T>;

		const auto vRange = std::get<INDEX_ID_T>(xm_vIndices).GetEqualRange(&vComparandT);
		return std::make_pair(static_cast<Node *>(static_cast<IndexNode *>(vRange.first)),
			static_cast<Node *>(static_cast<IndexNode *>(vRange.second)));
	}

public:
	using value_type = Node;

	// std::insert_iterator
	template<typename ParamT>
	void insert(Node *pHint, ParamT &&vParams){
		InsertWithHints(
			Hints(reinterpret_cast<Node *>(reinterpret_cast<IndicesT *>(pHint))...),
			std::forward<ParamT>(vParams));
	}
};

template<class ElementT, typename ...IndicesT>
void swap(MultiIndexMap<ElementT, IndicesT...> &lhs,
	MultiIndexMap<ElementT, IndicesT...> &rhs) noexcept
{
	lhs.Swap(rhs);
}

}

#endif
