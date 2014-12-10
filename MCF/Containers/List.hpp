// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_LIST_HPP_
#define MCF_CONTAINERS_LIST_HPP_

#include <initializer_list>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace MCF {

template<class ElementT>
class List {
public:
	class Node {
		friend class List;

	private:
		ElementT xm_vElement;
		Node *xm_pPrev;
		Node *xm_pNext;

	private:
		template<typename ...ParamsT>
		explicit constexpr Node(ParamsT &&...vParams)
			: xm_vElement(std::forward<ParamsT>(vParams)...)
		{
		}

	public:
		const ElementT &GetElement() const noexcept {
			return xm_vElement;
		}
		ElementT &GetElement() noexcept {
			return xm_vElement;
		}

		const Node *GetPrev() const noexcept {
			return xm_pPrev;
		}
		Node *GetPrev() noexcept {
			return xm_pPrev;
		}
		const Node *GetNext() const noexcept {
			return xm_pNext;
		}
		Node *GetNext() noexcept {
			return xm_pNext;
		}
	};

private:
	Node *xm_pFirst;
	Node *xm_pLast;

public:
	constexpr List() noexcept
		: xm_pFirst(nullptr), xm_pLast(nullptr)
	{
	}
	template<typename ...ParamsT>
	explicit List(std::size_t uCount, const ParamsT &...vParams)
		: List()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class IteratorT>
	List(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: List()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class IteratorT>
	List(IteratorT itBegin, std::size_t uCount)
		: List()
	{
		CopyToEnd(itBegin, uCount);
	}
	List(std::initializer_list<ElementT> rhs)
		: List()
	{
		CopyToEnd(rhs.begin(), rhs.size());
	}
	List(const List &rhs)
		: List()
	{
		for(auto pCur = rhs.xm_pFirst; pCur; pCur = pCur->xm_pNext){
			Push(pCur->GetElement());
		}
	}
	List(List &&rhs) noexcept
		: List()
	{
		Swap(rhs);
	}
	List &operator=(std::initializer_list<ElementT> rhs){
		List(rhs).Swap(*this);
		return *this;
	}
	List &operator=(const List &rhs){
		if(&rhs != this){
			List(rhs).Swap(*this);
		}
		return *this;
	}
	List &operator=(List &&rhs) noexcept {
		Swap(rhs);
		return *this;
	}
	~List(){
		Clear();
	}

public:
	const Node *GetFirst() const noexcept {
		return xm_pFirst;
	}
	Node *GetFirst() noexcept {
		return xm_pFirst;
	}
	const Node *GetCFirst() const noexcept {
		return GetFirst();
	}
	const Node *GetLast() const noexcept {
		return xm_pLast;
	}
	Node *GetLast() noexcept {
		return xm_pLast;
	}
	const Node *GetCLast() const noexcept {
		return GetLast();
	}

	bool IsEmpty() const noexcept {
		return GetFirst() == nullptr;
	}
	void Clear() noexcept {
		while(xm_pFirst){
			const auto pCur = std::exchange(xm_pFirst, xm_pFirst->xm_pNext);
			delete pCur;
		}
		xm_pLast = nullptr;
	}

	template<typename ...ParamsT>
	Node *Insert(Node *pPos, ParamsT &&...vParams){
		auto pNode = new Node(std::forward<ParamsT>(vParams)...);

		const auto pPrev = std::exchange((pPos ? pPos->xm_pPrev : xm_pLast), pNode);
		(pPrev ? pPrev->xm_pNext : xm_pFirst) = pNode;
		pNode->xm_pPrev = pPrev;
		pNode->xm_pNext = pPos;

		return pNode;
	}
	Node *Erase(Node *pNode) noexcept {
		const auto pNext = pNode->xm_pNext;

		const auto pOldPrev = pNode->xm_pPrev;
		(pOldPrev ? pOldPrev->xm_pNext : xm_pFirst) = pNext;
		(pNext ? pNext->xm_pPrev : xm_pLast) = pOldPrev;

		delete pNode;
		return pNext;
	}
	Node *Erase(Node *pBegin, Node *pEnd) noexcept {
		if(pBegin != pEnd){
			const auto pOldPrev = pBegin->xm_pPrev;
			(pOldPrev ? pOldPrev->xm_pNext : xm_pFirst) = pEnd;
			(pEnd ? pEnd->xm_pPrev : xm_pLast) = pOldPrev;

			do {
				const auto pNode = pBegin;
				pBegin = pBegin->xm_pNext;

				delete pNode;
			} while(pBegin != pEnd);
		}
		return pEnd;
	}

	Node *Splice(Node *pPos, List &lstSource) noexcept {
		const auto pRet = Splice(pPos, lstSource, lstSource.xm_pFirst, nullptr);
		ASSERT(lstSource.IsEmpty());
		return pRet;
	}
	Node *Splice(Node *pPos, List &lstSource, Node *pSingle) noexcept {
		ASSERT(pSingle);
		return Splice(pPos, lstSource, pSingle, pSingle->xm_pNext);
	}
	Node *Splice(Node *pPos, List &lstSource, Node *pBegin, Node *pEnd) noexcept {
		if(pBegin != pEnd){
			const auto pOldPrev = pBegin->xm_pPrev;
			(pOldPrev ? pOldPrev->xm_pNext : lstSource.xm_pFirst) = pEnd;
			const auto pOldLast = std::exchange((pEnd ? pEnd->xm_pPrev : lstSource.xm_pLast), pOldPrev);

			const auto pPrev = std::exchange((pPos ? pPos->xm_pPrev : xm_pLast), pOldLast);
			pOldLast->xm_pNext = pPos;

			(pPrev ? pPrev->xm_pNext : xm_pFirst) = pBegin;
			pBegin->xm_pPrev = pPrev;
		}
		return pPos;
	}

	Node *Splice(Node *pPos, List &&lstSource) noexcept {
		return Splice(pPos, lstSource);
	}
	Node *Splice(Node *pPos, List &&lstSource, Node *pSingle) noexcept {
		return Splice(pPos, lstSource, pSingle);
	}
	Node *Splice(Node *pPos, List &&lstSource, Node *pBegin, Node *pEnd) noexcept {
		return Splice(pPos, lstSource, pBegin, pEnd);
	}

	template<typename ...ParamsT>
	Node *Push(ParamsT &&...vParams){
		return Insert(nullptr, std::forward<ParamsT>(vParams)...);
	}
	void Pop() noexcept {
		Erase(xm_pLast);
	}

	template<typename ...ParamsT>
	Node *Unshift(ParamsT &&...vParams){
		return Insert(xm_pFirst, std::forward<ParamsT>(vParams)...);
	}
	void Shift() noexcept {
		Erase(xm_pFirst);
	}

	template<typename ...ParamsT>
	void FillAtEnd(std::size_t uCount, const ParamsT &...vParams){
		for(std::size_t i = 0; i < uCount; ++i){
			Push(vParams...);
		}
	}
	template<class IteratorT>
	void CopyToEnd(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		while(itBegin != itEnd){
			Push(*itBegin);
			++itBegin;
		}
	}
	template<class IteratorT>
	void CopyToEnd(IteratorT itBegin, std::size_t uCount){
		for(std::size_t i = 0; i < uCount; ++i){
			Push(*itBegin);
			++itBegin;
		}
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		for(std::size_t i = 0; i < uCount; ++i){
			Pop();
		}
	}

	template<typename ...ParamsT>
	void FillAtBegin(std::size_t uCount, const ParamsT &...vParams){
		for(std::size_t i = 0; i < uCount; ++i){
			Unshift(vParams...);
		}
	}
	template<class IteratorT>
	void CopyToBegin(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		while(itBegin != itEnd){
			Unshift(*itBegin);
			++itBegin;
		}
	}
	template<class IteratorT>
	void CopyToBegin(IteratorT itBegin, std::size_t uCount){
		for(std::size_t i = 0; i < uCount; ++i){
			Unshift(*itBegin);
			++itBegin;
		}
	}
	void TruncateFromBegin(std::size_t uCount) noexcept {
		for(std::size_t i = 0; i < uCount; ++i){
			Shift();
		}
	}

	void Swap(List &rhs) noexcept {
		std::swap(xm_pFirst, rhs.xm_pFirst);
		std::swap(xm_pLast, rhs.xm_pLast);
	}

	void Reverse() noexcept {
		for(auto pCur = xm_pFirst; pCur; pCur = pCur->xm_pPrev){
			std::swap(pCur->xm_pPrev, pCur->xm_pNext);
		}
		std::swap(xm_pFirst, xm_pLast);
	}

public:
	using value_type = ElementT;

	// std::back_insert_iterator
	template<typename ParamT>
	void push_back(ParamT &&vParam){
		Push(std::forward<ParamT>(vParam));
	}
	// std::front_insert_iterator
	template<typename ParamT>
	void push_front(ParamT &&vParam){
		Unshift(std::forward<ParamT>(vParam));
	}
};

template<class ElementT>
void swap(List<ElementT> &lhs, List<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
