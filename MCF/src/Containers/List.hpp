// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

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
		AlignedStorage<Element> vElement;
		X_Node *pPrev;
		X_Node *pNext;
	};

public:
	enum : std::size_t {
		kNodeSize = sizeof(X_Node),
	};

private:
	X_Node *x_pHead;
	X_Node *x_pTail;

public:
	constexpr List() noexcept
		: x_pHead(nullptr), x_pTail(nullptr)
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
		auto pNode = rhs.x_pHead;
		while(pNode){
			const auto pNext = pNode->pNext;
			const void *const pElementRaw = pNext;
			const auto pElement = static_cast<const Element *>(pElementRaw);
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
		return !x_pHead;
	}
	void Clear() noexcept {
		auto pNode = x_pTail;
		while(pNode){
			const auto pPrev = pNode->pPrev;
			void *const pElementRaw = pNode;
			const auto pElement = static_cast<Element *>(pElementRaw);
			Destruct(pElement);
			Allocator()(pElementRaw);
			pNode = pPrev;
		}
		x_pHead = nullptr;
		x_pTail  = nullptr;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			auto pNode = x_pHead;
			while(pNode){
				const auto pNext = pNode->pNext;
				void *const pElementRaw = pNode;
				const auto pElement = static_cast<Element *>(pElementRaw);
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
		const auto pNode = x_pHead;
		if(!pNode){
			return nullptr;
		}
		const void *const pElementRaw = pNode;
		return static_cast<const Element *>(pElementRaw);
	}
	Element *GetFirst() noexcept {
		const auto pNode = x_pHead;
		if(!pNode){
			return nullptr;
		}
		void *const pElementRaw = pNode;
		return static_cast<Element *>(pElementRaw);
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		const auto pNode = x_pTail;
		if(!pNode){
			return nullptr;
		}
		const void *const pElementRaw = pNode;
		return static_cast<const Element *>(pElementRaw);
	}
	Element *GetLast() noexcept {
		const auto pNode = x_pTail;
		if(!pNode){
			return nullptr;
		}
		void *const pElementRaw = pNode;
		return static_cast<Element *>(pElementRaw);
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	static const Element *GetPrev(const Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const void *pElementRaw = pPos;
		auto pNode = static_cast<const X_Node *>(pElementRaw);
		pNode = pNode->pPrev;
		if(!pNode){
			return nullptr;
		}
		pElementRaw = pNode;
		return static_cast<const Element *>(pElementRaw);
	}
	static Element *GetPrev(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		void *pElementRaw = pPos;
		auto pNode = static_cast<X_Node *>(pElementRaw);
		pNode = pNode->pPrev;
		if(!pNode){
			return nullptr;
		}
		pElementRaw = pNode;
		return static_cast<Element *>(pElementRaw);
	}
	static const Element *GetNext(const Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const void *pElementRaw = pPos;
		auto pNode = static_cast<const X_Node *>(pElementRaw);
		pNode = pNode->pNext;
		if(!pNode){
			return nullptr;
		}
		pElementRaw = pNode;
		return static_cast<const Element *>(pElementRaw);
	}
	static Element *GetNext(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		void *pElementRaw = pPos;
		auto pNode = static_cast<X_Node *>(pElementRaw);
		pNode = pNode->pNext;
		if(!pNode){
			return nullptr;
		}
		pElementRaw = pNode;
		return static_cast<Element *>(pElementRaw);
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
		swap(x_pHead, rhs.x_pHead);
		swap(x_pTail, rhs.x_pTail);
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
		const auto pNewFirstRaw = Allocator()(kNodeSize);
		const auto pElement = static_cast<Element *>(pNewFirstRaw);
		try {
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		} catch(...){
			Allocator()(pNewFirstRaw);
			throw;
		}
		const auto pNewFirst = static_cast<X_Node *>(pNewFirstRaw);
		pNewFirst->pPrev = nullptr;
		pNewFirst->pNext = x_pHead;

		const auto pOldFirst = x_pHead;
		(pOldFirst ? pOldFirst->pPrev : x_pTail) = pNewFirst;
		x_pHead = pNewFirst;

		return *pElement;
	}
	void Shift(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= CountElements());

		auto pNewFirst = x_pHead;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pNext = pNewFirst->pNext;
			void *const pElementRaw = pNewFirst;
			const auto pElement = static_cast<Element *>(pElementRaw);
			Destruct(pElement);
			Allocator()(pElementRaw);
			pNewFirst = pNext;
		}
		(pNewFirst ? pNewFirst->pPrev : x_pTail) = nullptr;
		x_pHead = pNewFirst;
	}

	template<typename ...ParamsT>
	Element &Push(ParamsT &&...vParams){
		const auto pNewLastRaw = Allocator()(kNodeSize);
		const auto pElement = static_cast<Element *>(pNewLastRaw);
		try {
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
		} catch(...){
			Allocator()(pNewLastRaw);
			throw;
		}
		const auto pNewLast = static_cast<X_Node *>(pNewLastRaw);
		pNewLast->pPrev = x_pTail;
		pNewLast->pNext = nullptr;

		const auto pOldLast = x_pTail;
		(pOldLast ? pOldLast->pNext : x_pHead) = pNewLast;
		x_pTail = pNewLast;

		return *pElement;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		MCF_DEBUG_CHECK(uCount <= CountElements());

		auto pNewLast = x_pTail;
		for(std::size_t i = 0; i < uCount; ++i){
			const auto pPrev = pNewLast->pPrev;
			void *const pElementRaw = pNewLast;
			const auto pElement = static_cast<Element *>(pElementRaw);
			Destruct(pElement);
			Allocator()(pElementRaw);
			pNewLast = pPrev;
		}
		(pNewLast ? pNewLast->pNext : x_pHead) = nullptr;
		x_pTail = pNewLast;
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

		void *const pInsertRaw = const_cast<void *>(static_cast<const void *>(pInsert));
		void *const pBeginRaw  = const_cast<void *>(static_cast<const void *>(pBegin ));
		void *const pEndRaw    = const_cast<void *>(static_cast<const void *>(pEnd   ));
		const auto pInsertNode = static_cast<X_Node *>(pInsertRaw);
		const auto pBeginNode  = static_cast<X_Node *>(pBeginRaw );
		const auto pEndNode    = static_cast<X_Node *>(pEndRaw   );

		if(pBeginNode != pEndNode){
			MCF_DEBUG_CHECK(pBeginNode);

			const auto pNodeBeforeBegin = pBeginNode->pPrev;
			const auto pNodeBeforeEnd = std::exchange(pEndNode ? pEndNode->pPrev : lstSrc.x_pTail, pNodeBeforeBegin);
			const auto pNodeBeforeInsert = std::exchange(pInsertNode ? pInsertNode->pPrev : x_pTail, pNodeBeforeEnd);

			(pNodeBeforeInsert ? pNodeBeforeInsert->pNext : x_pHead) = pBeginNode;
			(pNodeBeforeBegin ? pNodeBeforeBegin->pNext: lstSrc.x_pHead) = pEndNode;
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
		auto ppNext = &x_pHead, ppPrev = &x_pTail;
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
