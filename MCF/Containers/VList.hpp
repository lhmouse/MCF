// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_V_LIST_HPP_
#define MCF_V_LIST_HPP_

#include <initializer_list>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace MCF {

template<class Element>
class VList {
public:
	class Node {
		friend class VList;

	private:
		Element xm_vElement;
		Node *xm_pPrev;
		Node *xm_pNext;

	private:
#ifdef NDEBUG
		constexpr Node()
			noexcept(std::is_nothrow_constructible<Element>::value)
		{
		}
#else
		Node()
			noexcept(std::is_nothrow_constructible<Element>::value)
		{
			if(std::is_pod<Element>::value){
				__builtin_memset(&xm_vElement, 0xCC, sizeof(xm_vElement));
			}
		}
#endif
		template<typename ...Params>
		explicit constexpr Node(Params &&...vParams)
			: xm_vElement(std::forward<Params>(vParams)...)
		{
		}

	public:
		const Element &GetElement() const noexcept {
			return xm_vElement;
		}
		Element &GetElement() noexcept {
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
	constexpr VList() noexcept
		: xm_pFirst	(nullptr)
		, xm_pLast	(nullptr)
	{
	}
	template<typename ...Params>
	explicit VList(std::size_t uCount, const Params &...vParams)
		: VList()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class Iterator>
	VList(Iterator itBegin, std::common_type_t<Iterator> itEnd)
		: VList()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class Iterator>
	VList(Iterator itBegin, std::size_t uCount)
		: VList()
	{
		CopyToEnd(itBegin, uCount);
	}
	VList(std::initializer_list<Element> rhs)
		: VList()
	{
		CopyToEnd(rhs.begin(), rhs.size());
	}
	VList(const VList &rhs)
		: VList()
	{
		for(auto pCur = rhs.xm_pFirst; pCur; pCur = pCur->xm_pNext){
			Push(pCur->GetElement());
		}
	}
	VList(VList &&rhs) noexcept
		: VList()
	{
		std::swap(xm_pFirst, rhs.xm_pFirst);
		std::swap(xm_pLast, rhs.xm_pLast);
	}
	VList &operator=(std::initializer_list<Element> rhs){
		VList(rhs).Swap(*this);
		return *this;
	}
	VList &operator=(const VList &rhs){
		if(&rhs != this){
			VList(rhs).Swap(*this);
		}
		return *this;
	}
	VList &operator=(VList &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~VList() noexcept {
		Clear();
	}

public:
	const Node *GetFirst() const noexcept {
		return xm_pFirst;
	}
	Node *GetFirst() noexcept {
		return xm_pFirst;
	}
	const Node *GetCBegin() const noexcept {
		return GetFirst();
	}
	const Node *GetLast() const noexcept {
		return xm_pLast;
	}
	Node *GetLast() noexcept {
		return xm_pLast;
	}
	const Node *GetCRBegin() const noexcept {
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

	template<typename ...Params>
	Node *Insert(Node *pPos, Params &&...vParams){
		auto pNode = new Node(std::forward<Params>(vParams)...);

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

	Node *Splice(Node *pPos, VList &lstSource) noexcept {
		const auto pRet = Splice(pPos, lstSource, lstSource.xm_pFirst, nullptr);
		ASSERT(lstSource.IsEmpty());
		return pRet;
	}
	Node *Splice(Node *pPos, VList &lstSource, Node *pSingle) noexcept {
		ASSERT(pSingle);
		return Splice(pPos, lstSource, pSingle, pSingle->xm_pNext);
	}
	Node *Splice(Node *pPos, VList &lstSource, Node *pBegin, Node *pEnd) noexcept {
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

	Node *Splice(Node *pPos, VList &&lstSource) noexcept {
		return Splice(pPos, lstSource);
	}
	Node *Splice(Node *pPos, VList &&lstSource, Node *pSingle) noexcept {
		return Splice(pPos, lstSource, pSingle);
	}
	Node *Splice(Node *pPos, VList &&lstSource, Node *pBegin, Node *pEnd) noexcept {
		return Splice(pPos, lstSource, pBegin, pEnd);
	}

	template<typename ...Params>
	Node *Push(Params &&...vParams){
		return Insert(nullptr, std::forward<Params>(vParams)...);
	}
	void Pop() noexcept {
		Erase(xm_pLast);
	}

	template<typename ...Params>
	Node *Unshift(Params &&...vParams){
		return Insert(xm_pFirst, std::forward<Params>(vParams)...);
	}
	void Shift() noexcept {
		Erase(xm_pFirst);
	}

	template<typename ...Params>
	void FillAtEnd(std::size_t uCount, const Params &...vParams){
		for(std::size_t i = 0; i < uCount; ++i){
			Push(vParams...);
		}
	}
	template<class Iterator>
	void CopyToEnd(Iterator itBegin, std::common_type_t<Iterator> itEnd){
		while(itBegin != itEnd){
			Push(*itBegin);
			++itBegin;
		}
	}
	template<class Iterator>
	void CopyToEnd(Iterator itBegin, std::size_t uCount){
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

	template<typename ...Params>
	void FillAtBegin(std::size_t uCount, const Params &...vParams){
		for(std::size_t i = 0; i < uCount; ++i){
			Unshift(vParams...);
		}
	}
	template<class Iterator>
	void CopyToBegin(Iterator itBegin, std::common_type_t<Iterator> itEnd){
		while(itBegin != itEnd){
			Unshift(*itBegin);
			++itBegin;
		}
	}
	template<class Iterator>
	void CopyToBegin(Iterator itBegin, std::size_t uCount){
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

	void Swap(VList &rhs) noexcept {
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
	typedef Element value_type;

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Push(std::forward<Param>(vParam));
	}
	// std::front_insert_iterator
	template<typename Param>
	void push_front(Param &&vParam){
		Unshift(std::forward<Param>(vParam));
	}
};

template<class Element>
void swap(VList<Element> &lhs, VList<Element> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
