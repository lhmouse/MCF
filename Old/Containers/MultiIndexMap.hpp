// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_MULTI_INDEX_MAP_HPP_
#define MCF_CONTAINERS_MULTI_INDEX_MAP_HPP_

#include "../../MCFCRT/env/avl_tree.h"
#include "../Utilities/Assert.hpp"
#include <utility>
#include <tuple>
#include <type_traits>
#include <cstdlib>

namespace MCF {

template<typename DelegateComparatorT = std::less<void>>
struct TripleComparator {
	template<typename Tx, typename Ty>
	int operator()(const Tx &lhs, const Ty &rhs){
		if(DelegateComparatorT()(lhs, rhs)){
			return -1;
		} else if(DelegateComparatorT()(rhs, lhs)){
			return 1;
		} else {
			return 0;
		}
	}
};

template<typename KeyT>
TripleComparator<std::less<void>> GetDefaultComparator(const KeyT &) noexcept; // ADL

namespace Impl_MultiIndexMap {
	template<std::size_t kIndexT>
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

	template<class NodeT, class ElementT, std::size_t kIndexT, class IndexNodeT, class ComparatorT>
	struct OrderedMapIndexNodeComparator {
		using MapIndexNode = OrderedMapIndexNode<kIndexT>;

		static auto &Get(const MapIndexNode &vMapIndexNode) noexcept {
			return NodeT::template GetNodeFromIndexNode<kIndexT>(vMapIndexNode).Get();
		}

		int operator()(const MapIndexNode &lhs, const MapIndexNode &rhs) const noexcept {
			return ComparatorT()(Get(lhs), Get(rhs));
		}
		template<typename ComparandT>
		int operator()(const MapIndexNode &lhs, const ComparandT &rhs) const noexcept {
			return ComparatorT()(Get(lhs), rhs);
		}
	};

	template<class ElementT, class KeyTypeT, KeyTypeT ElementT::*kPtrToKeyT, class ComparatorT>
	struct MemberComparator {
		int operator()(const ElementT &lhs, const ElementT &rhs) const noexcept {
			return ComparatorT()(lhs.*kPtrToKeyT, rhs.*kPtrToKeyT);
		}
		template<typename ComparandT>
		int operator()(const ElementT &lhs, const ComparandT &rhs) const noexcept {
			return ComparatorT()(lhs.*kPtrToKeyT, rhs);
		}
	};

	template<std::size_t kIndexT, class NodeComparatorT>
	class MultiOrderedMapIndex {
	public:
		using IndexNode = OrderedMapIndexNode<kIndexT>;

	private:
		::MCF_AvlRoot x_avlRoot;
		IndexNode *x_pFirst;
		IndexNode *x_pLast;

	public:
		constexpr MultiOrderedMapIndex() noexcept
			: x_avlRoot(nullptr), x_pFirst(nullptr), x_pLast(nullptr)
		{
		}

	public:
		void Clear() noexcept {
			x_avlRoot = nullptr;
			x_pFirst = nullptr;
			x_pLast = nullptr;
		}

		const IndexNode *GetFirst() const noexcept {
			return x_pFirst;
		}
		IndexNode *GetFirst() noexcept {
			return x_pFirst;
		}
		const IndexNode *GetLast() const noexcept {
			return x_pLast;
		}
		IndexNode *GetLast() noexcept {
			return x_pLast;
		}

		IndexNode *Attach(IndexNode *pPos, IndexNode *pIndexNode) noexcept {
			::MCF_AvlAttachWithHint(&x_avlRoot, pPos, pIndexNode,
				[](const ::MCF_AvlNodeHeader *lhs, const ::MCF_AvlNodeHeader *rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), static_cast<const IndexNode &>(*rhs));
				}
			);
			if(!pIndexNode->GetPrev()){
				x_pFirst = pIndexNode;
			}
			if(!pIndexNode->GetNext()){
				x_pLast = pIndexNode;
			}
			return nullptr;
		}
		void Detach(IndexNode *pIndexNode) noexcept {
			if(x_pFirst == pIndexNode){
				x_pFirst = pIndexNode->GetNext();
			}
			if(x_pLast == pIndexNode){
				x_pLast = pIndexNode->GetPrev();
			}
			::MCF_AvlDetach(pIndexNode);
		}

		template<typename ComparandT>
		const IndexNode *GetLowerBound(const ComparandT *pComparand) const noexcept {
			return static_cast<const IndexNode *>(::MCF_AvlLowerBound(&x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}
		template<typename ComparandT>
		IndexNode *GetLowerBound(const ComparandT *pComparand) noexcept {
			return static_cast<IndexNode *>(::MCF_AvlLowerBound(&x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}

		template<typename ComparandT>
		const IndexNode *GetUpperBound(const ComparandT *pComparand) const noexcept {
			return static_cast<const IndexNode *>(::MCF_AvlUpperBound(&x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}
		template<typename ComparandT>
		IndexNode *GetUpperBound(const ComparandT *pComparand) noexcept {
			return static_cast<IndexNode *>(::MCF_AvlUpperBound(&x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}

		template<typename ComparandT>
		const IndexNode *Find(const ComparandT *pComparand) const noexcept {
			return static_cast<const IndexNode *>(::MCF_AvlFind(&x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}
		template<typename ComparandT>
		IndexNode *Find(const ComparandT *pComparand) noexcept {
			return static_cast<IndexNode *>(::MCF_AvlFind(&x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			));
		}

		template<typename ComparandT>
		std::pair<const IndexNode *, const IndexNode *> GetEqualRange(const ComparandT *pComparand) const noexcept {
			::MCF_AvlNodeHeader *pBegin, *pEnd;
			::MCF_AvlEqualRange(&pBegin, &pEnd, &x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			);
			return std::make_pair(static_cast<const IndexNode *>(pBegin), static_cast<const IndexNode *>(pEnd));
		}
		template<typename ComparandT>
		std::pair<IndexNode *, IndexNode *> GetEqualRange(const ComparandT *pComparand) noexcept {
			::MCF_AvlNodeHeader *pBegin, *pEnd;
			::MCF_AvlEqualRange(&pBegin, &pEnd, &x_avlRoot, reinterpret_cast<std::intptr_t>(pComparand),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return NodeComparatorT()(static_cast<const IndexNode &>(*lhs), *reinterpret_cast<const ComparandT *>(rhs));
				}
			);
			return std::make_pair(static_cast<IndexNode *>(pBegin), static_cast<IndexNode *>(pEnd));
		}
	};

	template<std::size_t kIndexT, class NodeComparatorT>
	class UniqueOrderedMapIndex {
	public:
		using IndexNode = OrderedMapIndexNode<kIndexT>;

	private:
		MultiOrderedMapIndex<kIndexT, NodeComparatorT> x_vDelegate;

	public:
		void Clear() noexcept {
			x_vDelegate.Clear();
		}

		const IndexNode *GetFirst() const noexcept {
			return x_vDelegate.GetFirst();
		}
		IndexNode *GetFirst() noexcept {
			return x_vDelegate.GetFirst();
		}
		const IndexNode *GetLast() const noexcept {
			return x_vDelegate.GetLast();
		}
		IndexNode *GetLast() noexcept {
			return x_vDelegate.GetLast();
		}

		IndexNode *Attach(IndexNode *pPos, IndexNode *pIndexNode) noexcept {
			auto pHint = x_vDelegate.GetLowerBound(pIndexNode);
			if(pHint){	// *pHint >= *pIndexNode
				if(NodeComparatorT()(*pIndexNode, *pHint) >= 0){	// *pIndexNode >= *pHint
					return pHint;
				}
			} else {
				pHint = pPos;
			}
			return x_vDelegate.Attach(pHint, pIndexNode);
		}
		void Detach(IndexNode *pIndexNode) noexcept {
			x_vDelegate.Detach(pIndexNode);
		}

		template<typename ComparandT>
		const IndexNode *GetLowerBound(const ComparandT *pComparand) const noexcept {
			return x_vDelegate.GetLowerBound(pComparand);
		}
		template<typename ComparandT>
		IndexNode *GetLowerBound(const ComparandT *pComparand) noexcept {
			return x_vDelegate.GetLowerBound(pComparand);
		}

		template<typename ComparandT>
		const IndexNode *GetUpperBound(const ComparandT *pComparand) const noexcept {
			return x_vDelegate.GetUpperBound(pComparand);
		}
		template<typename ComparandT>
		IndexNode *GetUpperBound(const ComparandT *pComparand) noexcept {
			return x_vDelegate.GetUpperBound(pComparand);
		}

		template<typename ComparandT>
		const IndexNode *Find(const ComparandT *pComparand) const noexcept {
			return x_vDelegate.Find(pComparand);
		}
		template<typename ComparandT>
		IndexNode *Find(const ComparandT *pComparand) noexcept {
			return x_vDelegate.Find(pComparand);
		}

		template<typename ComparandT>
		std::pair<const IndexNode *, const IndexNode *> GetEqualRange(const ComparandT *pComparand) const noexcept {
			return x_vDelegate.GetEqualRange(pComparand);
		}
		template<typename ComparandT>
		std::pair<IndexNode *, IndexNode *> GetEqualRange(const ComparandT *pComparand) noexcept {
			return x_vDelegate.GetEqualRange(pComparand);
		}
	};

	template<std::size_t kIndexT>
	class SequencedMapIndexNode {
		template<std::size_t>
		friend class SequencedMapIndex;

	private:
		SequencedMapIndexNode *x_pPrev;
		SequencedMapIndexNode *x_pNext;

	public:
		const SequencedMapIndexNode *GetPrev() const noexcept {
			return x_pPrev;
		}
		SequencedMapIndexNode *GetPrev() noexcept {
			return x_pPrev;
		}
		const SequencedMapIndexNode *GetNext() const noexcept {
			return x_pNext;
		}
		SequencedMapIndexNode *GetNext() noexcept {
			return x_pNext;
		}
	};

	template<std::size_t kIndexT>
	class SequencedMapIndex {
	public:
		using IndexNode = SequencedMapIndexNode<kIndexT>;

	private:
		IndexNode *x_pFirst;
		IndexNode *x_pLast;

	public:
		constexpr SequencedMapIndex() noexcept
			: x_pFirst(nullptr), x_pLast(nullptr)
		{
		}

	public:
		void Clear() noexcept {
			x_pFirst = nullptr;
			x_pLast = nullptr;
		}

		const IndexNode *GetFirst() const noexcept {
			return x_pFirst;
		}
		IndexNode *GetFirst() noexcept {
			return x_pFirst;
		}
		const IndexNode *GetLast() const noexcept {
			return x_pLast;
		}
		IndexNode *GetLast() noexcept {
			return x_pLast;
		}

		IndexNode *Attach(IndexNode *pNext, IndexNode *pIndexNode) noexcept {
			const auto pPrev = std::exchange((pNext ? pNext->x_pPrev : x_pLast), pIndexNode);
			(pPrev ? pPrev->x_pNext : x_pFirst) = pIndexNode;
			pIndexNode->x_pPrev = pPrev;
			pIndexNode->x_pNext = pNext;
			return nullptr;
		}
		void Detach(IndexNode *pIndexNode) noexcept {
			const auto pNext = pIndexNode->x_pNext;
			const auto pPrev = pIndexNode->x_pPrev;
			(pPrev ? pPrev->x_pNext : x_pFirst) = pNext;
			(pNext ? pNext->x_pPrev : x_pLast) = pPrev;
		}
	};
}

template<class ElementT,
	class ComparatorT = decltype(GetDefaultComparator(std::declval<const ElementT &>()))>
struct MultiOrderedIndex {
	template<std::size_t kIndexT>
	using IndexNode = Impl_MultiIndexMap::OrderedMapIndexNode<kIndexT>;

	template<class NodeT, std::size_t kIndexT>
	using IndexType = Impl_MultiIndexMap::MultiOrderedMapIndex<kIndexT,
		Impl_MultiIndexMap::OrderedMapIndexNodeComparator<
			NodeT, ElementT, kIndexT, IndexNode<kIndexT>, ComparatorT>
		>;

	static ElementT &GetKey(ElementT &vElement) noexcept {
		return vElement;
	}
};

template<class ElementT,
	class ComparatorT = decltype(GetDefaultComparator(std::declval<const ElementT &>()))>
struct UniqueOrderedIndex {
	template<std::size_t kIndexT>
	using IndexNode = Impl_MultiIndexMap::OrderedMapIndexNode<kIndexT>;

	template<class NodeT, std::size_t kIndexT>
	using IndexType = Impl_MultiIndexMap::UniqueOrderedMapIndex<kIndexT,
		Impl_MultiIndexMap::OrderedMapIndexNodeComparator<
			NodeT, ElementT, kIndexT, IndexNode<kIndexT>, ComparatorT>
		>;

	static ElementT &GetKey(ElementT &vElement) noexcept {
		return vElement;
	}
};

template<class ElementT, class KeyTypeT, KeyTypeT ElementT::*kPtrToKeyT,
	class ComparatorT = decltype(GetDefaultComparator(std::declval<const KeyTypeT &>()))>
struct MultiOrderedMemberIndex {
	template<std::size_t kIndexT>
	using IndexNode = Impl_MultiIndexMap::OrderedMapIndexNode<kIndexT>;

	template<class NodeT, std::size_t kIndexT>
	using IndexType = Impl_MultiIndexMap::MultiOrderedMapIndex<kIndexT,
		Impl_MultiIndexMap::OrderedMapIndexNodeComparator<
			NodeT, ElementT, kIndexT, IndexNode<kIndexT>,
			Impl_MultiIndexMap::MemberComparator<ElementT, KeyTypeT, kPtrToKeyT, ComparatorT>>
		>;

	static KeyTypeT &GetKey(ElementT &vElement) noexcept {
		return vElement.*kPtrToKeyT;
	}
};

template<class ElementT, class KeyTypeT, KeyTypeT ElementT::*kPtrToKeyT,
	class ComparatorT = decltype(GetDefaultComparator(std::declval<const KeyTypeT &>()))>
struct UniqueOrderedMemberIndex {
	template<std::size_t kIndexT>
	using IndexNode = Impl_MultiIndexMap::OrderedMapIndexNode<kIndexT>;

	template<class NodeT, std::size_t kIndexT>
	using IndexType = Impl_MultiIndexMap::UniqueOrderedMapIndex<kIndexT,
		Impl_MultiIndexMap::OrderedMapIndexNodeComparator<
			NodeT, ElementT, kIndexT, IndexNode<kIndexT>,
			Impl_MultiIndexMap::MemberComparator<ElementT, KeyTypeT, kPtrToKeyT, ComparatorT>>
		>;

	static KeyTypeT &GetKey(ElementT &vElement) noexcept {
		return vElement.*kPtrToKeyT;
	}
};

template<class ElementT>
struct SequencedIndex {
	template<std::size_t kIndexT>
	using IndexNode = Impl_MultiIndexMap::SequencedMapIndexNode<kIndexT>;

	template<class NodeT, std::size_t kIndexT>
	using IndexType = Impl_MultiIndexMap::SequencedMapIndex<kIndexT>;

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

	template<std::size_t kIndexT>
		using xIndexNode = typename
			std::tuple_element_t<kIndexT, std::tuple<IndicesT...>>::template IndexNode<kIndexT>;

	template<std::size_t ...kIndicesT>
	class xNodeImpl
		: private IndicesT::template IndexNode<kIndicesT>...
		, private xAddressNode	// 复制树结构的时候用到。
	{
		friend MultiIndexMap;

	public:
		template<std::size_t kIndexT>
		static auto &GetNodeFromIndexNode(const xIndexNode<kIndexT> &vIndexNode) noexcept {
			return static_cast<const xNodeImpl &>(vIndexNode);
		}
		template<std::size_t kIndexT>
		static auto &GetNodeFromIndexNode(xIndexNode<kIndexT> &vIndexNode) noexcept {
			return static_cast<xNodeImpl &>(vIndexNode);
		}

	private:
		// 这个成员在复制树结构的时候用到。
		const xNodeImpl *x_pSource;

		ElementT x_vElement;

	public:
		xNodeImpl() = default;

		template<typename ...ParamsT>
		explicit xNodeImpl(ParamsT &&...vParams)
			: x_vElement(std::forward<ParamsT>(vParams)...)
		{
		}

	public:
		const ElementT &Get() const noexcept {
			return x_vElement;
		}
		ElementT &Get() noexcept {
			return x_vElement;
		}

		template<std::size_t kIndexT>
		const xNodeImpl *GetPrev() const noexcept {
			using IndexNode = xIndexNode<kIndexT>;

			// return static_cast<const xNodeImpl *>(static_cast<const IndexNode *>());
			const auto pIndexNode = static_cast<const IndexNode *>(this)->GetPrev();
			if(!pIndexNode){
				return nullptr;
			}
			return &static_cast<const xNodeImpl &>(static_cast<const IndexNode &>(*pIndexNode));
		}
		template<std::size_t kIndexT>
		xNodeImpl *GetPrev() noexcept {
			using IndexNode = xIndexNode<kIndexT>;

			// return static_cast<xNodeImpl *>(static_cast<IndexNode *>(static_cast<IndexNode *>(this)->GetPrev()));
			const auto pIndexNode = static_cast<IndexNode *>(this)->GetPrev();
			if(!pIndexNode){
				return nullptr;
			}
			return &static_cast<xNodeImpl &>(static_cast<IndexNode &>(*pIndexNode));
		}
		template<std::size_t kIndexT>
		const xNodeImpl *GetNext() const noexcept {
			using IndexNode = xIndexNode<kIndexT>;

			// return static_cast<const xNodeImpl *>(static_cast<const IndexNode *>(static_cast<const IndexNode *>(this)->GetNext()));
			const auto pIndexNode = static_cast<const IndexNode *>(this)->GetNext();
			if(!pIndexNode){
				return nullptr;
			}
			return &static_cast<const xNodeImpl &>(static_cast<const IndexNode &>(*pIndexNode));
		}
		template<std::size_t kIndexT>
		xNodeImpl *GetNext() noexcept {
			using IndexNode = xIndexNode<kIndexT>;

			// return static_cast<xNodeImpl *>(static_cast<IndexNode *>(static_cast<IndexNode *>(this)->GetNext()));
			const auto pIndexNode = static_cast<IndexNode *>(this)->GetNext();
			if(!pIndexNode){
				return nullptr;
			}
			return &static_cast<xNodeImpl &>(static_cast<IndexNode &>(*pIndexNode));
		}
	};

	template<std::size_t ...kIndicesT>
		static auto xMakeNode(std::index_sequence<kIndicesT...>) noexcept ->
			xNodeImpl<kIndicesT...>;
	template<class NodeT, std::size_t ...kIndicesT>
		static auto xMakeIndices(std::index_sequence<kIndicesT...>) noexcept ->
			std::tuple<typename IndicesT::template IndexType<NodeT, kIndicesT>...>;

	template<std::size_t kIndexT, typename CursorT, typename RealElementT, typename RealNodeT>
	class xCursorTemplate
		: public std::iterator<std::bidirectional_iterator_tag, RealElementT>
	{
	protected:
		RealNodeT *x_pNode;

	protected:
		explicit constexpr xCursorTemplate(RealNodeT *pNode) noexcept
			: x_pNode(pNode)
		{
		}

	public:
		constexpr xCursorTemplate() noexcept
			: xCursorTemplate(nullptr)
		{
		}

	public:
		RealNodeT *Get() const noexcept {
			return x_pNode;
		}

	public:
		bool operator==(const xCursorTemplate &rhs) const noexcept {
			return x_pNode == rhs.x_pNode;
		}
		bool operator!=(const xCursorTemplate &rhs) const noexcept {
			return x_pNode != rhs.x_pNode;
		}

		RealElementT &operator*() const noexcept {
			ASSERT_MSG(x_pNode, L"游标指向链表两端或者为空。");
			return x_pNode->Get();
		}
		RealElementT *operator->() const noexcept {
			ASSERT_MSG(x_pNode, L"游标指向链表两端或者为空。");
			return std::addressof(x_pNode->Get());
		}

		CursorT &operator++() noexcept {
			ASSERT_MSG(x_pNode, L"空游标不能移动。");

			x_pNode = x_pNode->template GetNext<kIndexT>();
			return static_cast<CursorT &>(*this);
		}
		CursorT &operator--() noexcept {
			ASSERT_MSG(x_pNode, L"空游标不能移动。");

			x_pNode = x_pNode->template GetPrev<kIndexT>();
			return static_cast<CursorT &>(*this);
		}

		CursorT operator++(int) noexcept {
			CursorT ret(x_pNode);
			++*this;
			return ret;
		}
		CursorT operator--(int) noexcept {
			CursorT ret(x_pNode);
			--*this;
			return ret;
		}

		explicit operator bool() const noexcept {
			return x_pNode != nullptr;
		}
	};

public:
	using Node = decltype(xMakeNode(std::index_sequence_for<IndicesT...>()));
	using Hints = std::tuple<decltype(reinterpret_cast<Node *>(static_cast<IndicesT *>(nullptr)))...>;

	template<std::size_t kIndexT>
	class ConstCursor;

	template<std::size_t kIndexT>
	class Cursor
		: public xCursorTemplate<kIndexT, Cursor<kIndexT>, ElementT, Node>
	{
		friend MultiIndexMap;
		friend ConstCursor<kIndexT>;

	private:
		using xBase = xCursorTemplate<kIndexT, Cursor<kIndexT>, ElementT, Node>;

	private:
		explicit constexpr Cursor(Node *pNode) noexcept
			: xBase(pNode)
		{
		}

	public:
		constexpr Cursor() noexcept = default;
	};

	template<std::size_t kIndexT>
	class ConstCursor
		: public xCursorTemplate<kIndexT, ConstCursor<kIndexT>, const ElementT, const Node>
	{
		friend MultiIndexMap;

	private:
		using xBase = xCursorTemplate<kIndexT, ConstCursor<kIndexT>, const ElementT, const Node>;

	private:
		explicit constexpr ConstCursor(const Node *pNode) noexcept
			: xBase(pNode)
		{
		}

	public:
		constexpr ConstCursor() noexcept = default;

		constexpr ConstCursor(const Cursor<kIndexT> &rhs) noexcept
			: xBase(rhs.x_pNode)
		{
		}
	};

private:
	using xIndexTuple = decltype(xMakeIndices<Node>(std::index_sequence_for<IndicesT...>()));

private:
	template<typename, typename FirstT, typename ...RemainingT>
	static void xUpdateHints(Hints &vHints, Node *pNode) noexcept {
		constexpr std::size_t kIndex = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;

		auto &pHint = std::get<kIndex>(vHints);
		if(pHint == pNode){
			pHint = pNode->template GetNext<kIndex>();
		}

		xUpdateHints<void, RemainingT...>(vHints, pNode);
	}
	template<typename>
	static void xUpdateHints(Hints &, Node *) noexcept {
	}

private:
	xIndexTuple x_vIndices;
	std::size_t x_uSize;

public:
	MultiIndexMap() noexcept
		: x_vIndices(), x_uSize(0)
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
				pNewNode->x_pSource = pNode;
				::MCF_AvlAttach(&avlAddressMap, &static_cast<xAddressNode &>(*pNewNode),
					[](const ::MCF_AvlNodeHeader *lhs, const ::MCF_AvlNodeHeader *rhs){
						return TripleComparator<>()(
							static_cast<const Node &>(static_cast<const xAddressNode &>(*lhs)).x_pSource,
							static_cast<const Node &>(static_cast<const xAddressNode &>(*rhs)).x_pSource);
					}
				);
				pNode = pNode->template GetNext<0>();
			}
		} catch(...){
			if(avlAddressMap){
				auto pCur = ::MCF_AvlPrev(avlAddressMap);
				while(pCur){
					const auto pPrev = ::MCF_AvlPrev(pCur);
					delete &static_cast<Node &>(static_cast<xAddressNode &>(*pCur));
					pCur = pPrev;
				}
				pCur = ::MCF_AvlNext(avlAddressMap);
				while(pCur){
					const auto pNext = ::MCF_AvlNext(pCur);
					delete &static_cast<Node &>(static_cast<xAddressNode &>(*pCur));
					pCur = pNext;
				}
				delete &static_cast<Node &>(static_cast<xAddressNode &>(*avlAddressMap));
			}
			throw;
		}

		// 所有节点都分配完成，现在进行第二步，重建每个二叉树，保证无异常抛出。
		xCloneAll<void, IndicesT...>(rhs.x_vIndices, avlAddressMap);
		x_uSize = rhs.x_uSize;
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
		constexpr std::size_t kIndex = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;

		using IndexNode = xIndexNode<kIndex>;

		const auto pHintIndexNode = static_cast<IndexNode *>(std::get<kIndex>(vHints));
		const auto pIndexNode = &static_cast<IndexNode &>(*pNode);
		const auto pExistentIndexNode = std::get<kIndex>(x_vIndices).Attach(pHintIndexNode, pIndexNode);
		if(pExistentIndexNode){
			// 当前的插入操作失败了，返回现存的节点。
			return &static_cast<Node &>(static_cast<IndexNode &>(*pExistentIndexNode));
		}
		const auto pResult = xAttachAll<void, RemainingT...>(vHints, pNode);
		if(pResult){
			// 刚刚的插入操作失败了，撤销刚才的操作。
			std::get<kIndex>(x_vIndices).Detach(pIndexNode);
		}
		return pResult;
	}
	template<typename>
	Node *xAttachAll(const Hints &, Node *) noexcept {
		return nullptr;
	}

	template<std::size_t kExceptIndexT, typename FirstT, typename ...RemainingT>
	void xDetachAll(Node *pNode) noexcept {
		constexpr std::size_t kIndex = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;

		using IndexNode = xIndexNode<kIndex>;

		if(kIndex != kExceptIndexT){
			const auto pIndexNode = &static_cast<IndexNode &>(*pNode);
			std::get<kIndex>(x_vIndices).Detach(pIndexNode);
		}
		xDetachAll<kExceptIndexT, RemainingT...>(pNode);
	}
	template<std::size_t kExceptIndexT>
	void xDetachAll(Node *) noexcept {
	}

	template<typename, typename FirstT, typename ...RemainingT>
	void xClearAll() noexcept {
		constexpr std::size_t kIndex = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;

		std::get<kIndex>(x_vIndices).Clear();
		xClearAll<void, RemainingT...>();
	}
	template<typename>
	void xClearAll() noexcept {
	}

	template<typename, typename FirstT, typename ...RemainingT>
	void xCloneAll(const xIndexTuple &vStructure, ::MCF_AvlRoot avlNewNodes) noexcept {
		constexpr std::size_t kIndex = sizeof...(IndicesT) - sizeof...(RemainingT) - 1;
		using IndexNode = xIndexNode<kIndex>;

		IndexNode *pHintIndexNode = nullptr;

		// 逆序复制，这样我们可以保证除了第一次以外每次拿到的 pHintIndexNode 都是非空的。
		auto pSourceIndexNode = std::get<kIndex>(vStructure).GetLast();
		while(pSourceIndexNode){
			const auto pSourceNode = &static_cast<const Node &>(*pSourceIndexNode);
			const auto pNode = &static_cast<Node &>(static_cast<xAddressNode &>(*::MCF_AvlFind(
				&avlNewNodes, reinterpret_cast<std::intptr_t>(pSourceNode),
				[](const ::MCF_AvlNodeHeader *lhs, std::intptr_t rhs){
					return TripleComparator<>()(
						static_cast<const Node &>(static_cast<const xAddressNode &>(*lhs)).x_pSource,
						reinterpret_cast<const Node *>(rhs));
				}
			)));
			const auto pIndexNode = &static_cast<IndexNode &>(*pNode);
			const auto pExistentIndexNode = std::get<kIndex>(x_vIndices).Attach(pHintIndexNode, pIndexNode);
			ASSERT(!pExistentIndexNode);

			pHintIndexNode = pIndexNode;
			pSourceIndexNode = pSourceIndexNode->GetPrev();
		}

		xCloneAll<void, RemainingT...>(vStructure, avlNewNodes);
	}
	template<typename>
	void xCloneAll(const xIndexTuple &, ::MCF_AvlRoot) noexcept {
	}

public:
	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	void Clear() noexcept {
		auto pNode = GetFirst<0>();
		while(pNode){
			const auto pNext = pNode->template GetNext<0>();
			delete pNode;
			pNode = pNext;
		}
		xClearAll<void, IndicesT...>();
		x_uSize = 0;
	}

	template<typename ...ParamsT>
	std::pair<Node *, bool> Insert(bool bOverwrites, ParamsT &&...vParams){
		return InsertWithHints(bOverwrites, Hints(), std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	std::pair<Node *, bool> InsertWithHints(bool bOverwrites, const Hints &vHints, ParamsT &&...vParams){
		const auto pNode = new Node(std::forward<ParamsT>(vParams)...);
		Node *pExistent = xAttachAll<void, IndicesT...>(vHints, pNode);
		if(pExistent){
			if(!bOverwrites){
				delete pNode;
				return std::make_pair(pExistent, false);
			}
			Hints vNewHints(vHints);
			do {
				xUpdateHints<void, IndicesT...>(vNewHints, pExistent);
				Erase(pExistent);
				pExistent = xAttachAll<void, IndicesT...>(vNewHints, pNode);
			} while(pExistent);
		}
		++x_uSize;
		return std::make_pair(pNode, true);
	}
	void Erase(Node *pNode) noexcept {
		xDetachAll<(std::size_t)-1, IndicesT...>(pNode);
		delete pNode;
		--x_uSize;
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
			--x_uSize;
			throw;
		}

		Node *pExistent = xAttachAll<void, IndicesT...>(vHints, pNode);
		if(pExistent){
			if(!bOverwrites){
				delete pNode;
				--x_uSize;
				return nullptr;
			}
			Hints vNewHints(vHints);
			do {
				xUpdateHints<void, IndicesT...>(vNewHints, pExistent);
				Erase(pExistent);
				pExistent = xAttachAll<void, IndicesT...>(vNewHints, pNode);
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

	template<std::size_t kIndexT, typename ...ParamsT>
	bool SetKey(bool bOverwrites, Node *pNode, ParamsT &&...vParams){
		return SetKeyWithHint<kIndexT>(bOverwrites, nullptr, pNode, std::forward<ParamsT>(vParams)...);
	}
	template<std::size_t kIndexT, typename ...ParamsT>
	bool SetKeyWithHint(bool bOverwrites, Node *pHint, Node *pNode, ParamsT &&...vParams){
		using Index = std::tuple_element_t<kIndexT, std::tuple<IndicesT...>>;
		using IndexNode = typename Index::template IndexNode<kIndexT>;

		Index::GetKey(static_cast<ElementT &>(*pNode)) =
			std::remove_reference_t<decltype(Index::GetKey(std::declval<ElementT &>()))>(
				std::forward<ParamsT>(vParams)...);

		const auto pIndexNode = &static_cast<IndexNode &>(*pNode);
		std::get<kIndexT>(x_vIndices).Detach(pIndexNode);
		auto pHintIndexNode = &static_cast<IndexNode &>(*pHint);
		auto pExistentIndexNode = std::get<kIndexT>(x_vIndices).Attach(pHintIndexNode, pIndexNode);
		if(pExistentIndexNode){
			if(!bOverwrites){
				xDetachAll<kIndexT, IndicesT...>(pNode);
				delete pNode;
				--x_uSize;
				return false;
			}
			pHintIndexNode = static_cast<IndexNode *>(pExistentIndexNode->GetNext());
			Erase(static_cast<Node &>(static_cast<IndexNode &>(*pExistentIndexNode)));
			pExistentIndexNode = std::get<kIndexT>(x_vIndices).Attach(pHintIndexNode, pIndexNode);
			ASSERT(!pExistentIndexNode);
		}
		return true;
	}

	template<std::size_t kIndexT>
	const Node *GetFirst() const noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<const Node *>(static_cast<const IndexNode *>(std::get<kIndexT>(x_vIndices).GetFirst()));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetFirst();
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<const Node &>(static_cast<const IndexNode &>(*pIndexNode));
	}
	template<std::size_t kIndexT>
	Node *GetFirst() noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<Node *>(static_cast<IndexNode *>(std::get<kIndexT>(x_vIndices).GetFirst()));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetFirst();
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<Node &>(static_cast<IndexNode &>(*pIndexNode));
	}
	template<std::size_t kIndexT>
	const Node *GetLast() const noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<const Node *>(static_cast<const IndexNode *>(std::get<kIndexT>(x_vIndices).GetLast()));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetLast();
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<const Node &>(static_cast<const IndexNode &>(*pIndexNode));
	}
	template<std::size_t kIndexT>
	Node *GetLast() noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<Node *>(static_cast<IndexNode *>(std::get<kIndexT>(x_vIndices).GetLast()));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetLast();
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<Node &>(static_cast<IndexNode &>(*pIndexNode));
	}

	template<std::size_t kIndexT>
	ConstCursor<kIndexT> GetFirstCursor() const noexcept {
		return ConstCursor<kIndexT>(GetFirst<kIndexT>());
	}
	template<std::size_t kIndexT>
	Cursor<kIndexT> GetFirstCursor() noexcept {
		return Cursor<kIndexT>(GetFirst<kIndexT>());
	}
	template<std::size_t kIndexT>
	ConstCursor<kIndexT> GetLastCursor() const noexcept {
		return ConstCursor<kIndexT>(GetLast<kIndexT>());
	}
	template<std::size_t kIndexT>
	Cursor<kIndexT> GetLastCursor() noexcept {
		return Cursor<kIndexT>(GetLast<kIndexT>());
	}

	void Swap(MultiIndexMap &rhs) noexcept {
		std::swap(x_vIndices, rhs.x_vIndices);
		std::swap(x_uSize, rhs.x_uSize);
	}

	template<std::size_t kIndexT, typename ComparandT>
	const Node *GetLowerBound(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<const Node *>(static_cast<const IndexNode *>(std::get<kIndexT>(x_vIndices).GetLowerBound(&vComparandT)));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetLowerBound(&vComparandT);
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<const Node &>(static_cast<const IndexNode &>(*pIndexNode));
	}
	template<std::size_t kIndexT, typename ComparandT>
	Node *GetLowerBound(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<Node *>(static_cast<IndexNode *>(std::get<kIndexT>(x_vIndices).GetLowerBound(&vComparandT)));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetLowerBound(&vComparandT);
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<Node &>(static_cast<IndexNode &>(*pIndexNode));
	}

	template<std::size_t kIndexT, typename ComparandT>
	const Node *GetUpperBound(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<const Node *>(static_cast<const IndexNode *>(std::get<kIndexT>(x_vIndices).GetUpperBound(&vComparandT)));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetUpperBound(&vComparandT);
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<const Node &>(static_cast<const IndexNode &>(*pIndexNode));
	}
	template<std::size_t kIndexT, typename ComparandT>
	Node *GetUpperBound(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<Node *>(static_cast<IndexNode *>(std::get<kIndexT>(x_vIndices).GetUpperBound(&vComparandT)));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).GetUpperBound(&vComparandT);
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<Node &>(static_cast<IndexNode &>(*pIndexNode));
	}

	template<std::size_t kIndexT, typename ComparandT>
	const Node *Find(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<const Node *>(static_cast<const IndexNode *>(std::get<kIndexT>(x_vIndices).Find(&vComparandT)));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).Find(&vComparandT);
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<const Node &>(static_cast<const IndexNode &>(*pIndexNode));
	}
	template<std::size_t kIndexT, typename ComparandT>
	Node *Find(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		// return static_cast<Node *>(static_cast<IndexNode *>(std::get<kIndexT>(x_vIndices).Find(&vComparandT)));
		const auto pIndexNode = std::get<kIndexT>(x_vIndices).Find(&vComparandT);
		if(!pIndexNode){
			return nullptr;
		}
		return &static_cast<Node &>(static_cast<IndexNode &>(*pIndexNode));
	}

	template<std::size_t kIndexT, typename ComparandT>
	std::pair<const Node *, const Node *> GetEqualRange(const ComparandT &vComparandT) const noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		const auto vRange = std::get<kIndexT>(x_vIndices).GetEqualRange(&vComparandT);
		// return std::make_pair(static_cast<const Node *>(static_cast<const IndexNode *>(vRange.first)),
		// 	static_cast<const Node *>(static_cast<const IndexNode *>(vRange.second)));
		std::pair<const Node *, const Node *> vRet;
		if(vRange.first){
			vRet.first = &static_cast<const Node &>(static_cast<const IndexNode &>(*vRange.first));
		}
		if(vRange.second){
			vRet.second = &static_cast<const Node &>(static_cast<const IndexNode &>(*vRange.second));
		}
		return vRet;
	}
	template<std::size_t kIndexT, typename ComparandT>
	std::pair<Node *, Node *> GetEqualRange(const ComparandT &vComparandT) noexcept {
		using IndexNode = xIndexNode<kIndexT>;

		const auto vRange = std::get<kIndexT>(x_vIndices).GetEqualRange(&vComparandT);
		// return std::make_pair(static_cast<Node *>(static_cast<IndexNode *>(vRange.first)),
		// 	static_cast<Node *>(static_cast<IndexNode *>(vRange.second)));
		std::pair<Node *, Node *> vRet;
		if(vRange.first){
			vRet.first = &static_cast<Node &>(static_cast<IndexNode &>(*vRange.first));
		}
		if(vRange.second){
			vRet.second = &static_cast<Node &>(static_cast<IndexNode &>(*vRange.second));
		}
		return vRet;
	}

public:
	using value_type = Node;

	// std::insert_iterator
	template<typename ParamT>
	Node *insert(Node *pHint, ParamT &&vParams){
		return InsertWithHints(Hints(reinterpret_cast<Node *>(reinterpret_cast<IndicesT *>(pHint))...), std::forward<ParamT>(vParams)).first;
	}
};

template<class ElementT, typename ...IndicesT>
void swap(MultiIndexMap<ElementT, IndicesT...> &lhs, MultiIndexMap<ElementT, IndicesT...> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
