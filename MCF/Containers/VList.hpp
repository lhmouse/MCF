// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_V_LIST_HPP_
#define MCF_V_LIST_HPP_

#include "../Core/Utilities.hpp"
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
		union alignas(std::max_align_t) xRawElement {
			unsigned char aby[];
			Element elem;

			constexpr xRawElement() noexcept {
			}
			~xRawElement() noexcept {
			}
		} xm_vElement;

		Node *xm_pPrev;
		Node *xm_pNext;

	private:
		constexpr Node() noexcept = default;

	public:
		const Element &GetElement() const noexcept {
			return xm_vElement.elem;
		}
		Element &GetElement() noexcept {
			return xm_vElement.elem;
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
	Node *xm_pPool;

public:
	constexpr VList() noexcept
		: xm_pFirst	(nullptr)
		, xm_pLast	(nullptr)
		, xm_pPool	(nullptr)
	{
	}
	template<typename... Params>
	explicit VList(std::size_t uCount, const Params &...vParams)
		: VList()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class Iterator>
	VList(Iterator itBegin, typename std::common_type<Iterator>::type itEnd)
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

		for(;;){
			const auto pCur = rhs.xDepool();
			if(!pCur){
				break;
			}
			xEnpool(pCur);
		}
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
		VList(std::move(rhs)).Swap(*this);
		return *this;
	}
	~VList() noexcept {
		Clear(true);
	}

private:
	void xEnpool(Node *pNode) noexcept {
		pNode->xm_pPrev = std::exchange(xm_pPool, pNode);
	}
	Node *xDepool() noexcept {
		if(!xm_pPool){
			return nullptr;
		}
		return std::exchange(xm_pPool, xm_pPool->xm_pPrev);
	}

public:
	const Node *GetBegin() const noexcept {
		return xm_pFirst;
	}
	Node *GetBegin() noexcept {
		return xm_pFirst;
	}
	const Node *GetCBegin() const noexcept {
		return GetBegin();
	}
	const Node *GetRBegin() const noexcept {
		return xm_pLast;
	}
	Node *GetRBegin() noexcept {
		return xm_pLast;
	}
	const Node *GetCRBegin() const noexcept {
		return GetRBegin();
	}

	bool IsEmpty() const noexcept {
		return GetBegin() == nullptr;
	}
	void Clear(bool bDeallocatePool = false) noexcept {
		if(bDeallocatePool){
			while(xm_pFirst){
				const auto pCur = std::exchange(xm_pFirst, xm_pFirst->xm_pNext);
				Destruct(&(pCur->GetElement()));
				delete pCur;
			}
			xm_pLast = nullptr;

			for(;;){
				const auto pCur = xDepool();
				if(!pCur){
					break;
				}
				delete pCur;
			}
		} else {
			while(xm_pFirst){
				const auto pCur = std::exchange(xm_pFirst, xm_pFirst->xm_pNext);
				Destruct(&(pCur->GetElement()));
				xEnpool(pCur);
			}
		}
	}

	template<typename... Params>
	Node *Insert(Node *pPos, Params &&...vParams){
		return InsertWithPool(pPos, *this, std::forward<Params>(vParams)...);
	}
	template<typename... Params>
	Node *InsertWithPool(Node *pPos, VList &lstPool, Params &&...vParams){
		auto pNode = lstPool.xDepool();
		if(!pNode){
			pNode = new Node;
		}
		try {
			Construct(&(pNode->GetElement()), std::forward<Params>(vParams)...);
		} catch(...){
			delete pNode;
			throw;
		}

		const auto pPrev = std::exchange((pPos ? pPos->xm_pPrev : xm_pLast), pNode);
		(pPrev ? pPrev->xm_pNext : xm_pFirst) = pNode;
		pNode->xm_pPrev = pPrev;
		pNode->xm_pNext = pPos;

		return pNode;
	}
	Node *Erase(Node *pPos) noexcept {
		return Erase(pPos, pPos->xm_pNext);
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

	template<typename... Params>
	Node *Push(Params &&...vParams){
		return Insert(nullptr, std::forward<Params>(vParams)...);
	}
	void Pop() noexcept {
		Erase(xm_pLast);
	}

	template<typename... Params>
	Node *Unshift(Params &&...vParams){
		return Insert(xm_pFirst, std::forward<Params>(vParams)...);
	}
	void Shift() noexcept {
		Erase(xm_pFirst);
	}

	template<typename... Params>
	void FillAtEnd(std::size_t uCount, const Params &...vParams){
		for(std::size_t i = 0; i < uCount; ++i){
			Push(vParams...);
		}
	}
	template<class Iterator>
	void CopyToEnd(Iterator itBegin, typename std::common_type<Iterator>::type itEnd){
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

	template<typename... Params>
	void FillAtBegin(std::size_t uCount, const Params &...vParams){
		for(std::size_t i = 0; i < uCount; ++i){
			Unshift(vParams...);
		}
	}
	template<class Iterator>
	void CopyToBegin(Iterator itBegin, typename std::common_type<Iterator>::type itEnd){
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
		std::swap(xm_pPool, rhs.xm_pPool);
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

}

#endif
