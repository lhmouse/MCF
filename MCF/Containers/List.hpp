// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_LIST_HPP_
#define MCF_CONTAINERS_LIST_HPP_

#include "../Utilities/Assert.hpp"
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <memory>
#include <iterator>
#include <cstddef>

namespace MCF {

template<class ElementT>
class List {
private:
	class xNode {
		friend List;

	private:
		ElementT xm_vElement;
		xNode *xm_pPrev;
		xNode *xm_pNext;

	private:
		xNode() = default;

		template<typename ...ParamsT>
		explicit xNode(ParamsT &&...vParams)
			: xm_vElement(std::forward<ParamsT>(vParams)...)
		{
		}

	public:
		const ElementT &Get() const noexcept {
			return xm_vElement;
		}
		ElementT &Get() noexcept {
			return xm_vElement;
		}

		const xNode *GetPrev() const noexcept {
			return xm_pPrev;
		}
		xNode *GetPrev() noexcept {
			return xm_pPrev;
		}
		const xNode *GetNext() const noexcept {
			return xm_pNext;
		}
		xNode *GetNext() noexcept {
			return xm_pNext;
		}
	};

	template<typename CursorT, typename RealElementT, typename RealNodeT>
	class xCursorTemplate
		: public std::iterator<std::bidirectional_iterator_tag, RealElementT>
	{
	protected:
		RealNodeT *xm_pNode;

	protected:
		explicit constexpr xCursorTemplate(RealNodeT *pNode) noexcept
			: xm_pNode(pNode)
		{
		}

	public:
		constexpr xCursorTemplate() noexcept
			: xCursorTemplate(nullptr)
		{
		}

	public:
		bool operator==(const xCursorTemplate &rhs) const noexcept {
			return xm_pNode == rhs.xm_pNode;
		}
		bool operator!=(const xCursorTemplate &rhs) const noexcept {
			return xm_pNode != rhs.xm_pNode;
		}

		RealElementT &operator*() const noexcept {
			ASSERT_MSG(xm_pNode, L"游标指向链表两端或者为空。");
			return xm_pNode->Get();
		}
		RealElementT *operator->() const noexcept {
			ASSERT_MSG(xm_pNode, L"游标指向链表两端或者为空。");
			return std::addressof(xm_pNode->Get());
		}

		CursorT &operator++() noexcept {
			ASSERT_MSG(xm_pNode, L"空游标不能移动。");

			xm_pNode = xm_pNode->GetNext();
			return static_cast<CursorT &>(*this);
		}
		CursorT &operator--() noexcept {
			ASSERT_MSG(xm_pNode, L"空游标不能移动。");

			xm_pNode = xm_pNode->GetPrev();
			return static_cast<CursorT &>(*this);
		}

		CursorT operator++(int) noexcept {
			CursorT ret(xm_pNode);
			++*this;
			return ret;
		}
		CursorT operator--(int) noexcept {
			CursorT ret(xm_pNode);
			--*this;
			return ret;
		}

		explicit operator bool() const noexcept {
			return xm_pNode != nullptr;
		}
	};

public:
	using Node = xNode;

	class ConstCursor;

	class Cursor
		: public xCursorTemplate<Cursor, ElementT, Node>
	{
		friend List;
		friend ConstCursor;

	private:
		using xBase = xCursorTemplate<Cursor, ElementT, Node>;

	private:
		constexpr explicit Cursor(Node *pNode) noexcept
			: xBase(pNode)
		{
		}

	public:
		constexpr Cursor() noexcept = default;
	};

	class ConstCursor
		: public xCursorTemplate<ConstCursor, const ElementT, const Node>
	{
		friend List;

	private:
		using xBase = xCursorTemplate<ConstCursor, const ElementT, const Node>;

	private:
		constexpr explicit ConstCursor(const Node *pNode) noexcept
			: xBase(pNode)
		{
		}

	public:
		constexpr ConstCursor() noexcept = default;

		constexpr ConstCursor(const Cursor &rhs) noexcept
			: xBase(rhs.xm_pNode)
		{
		}
	};

private:
	xNode *xm_pFirst;
	xNode *xm_pLast;

public:
	constexpr List() noexcept
		: xm_pFirst(nullptr), xm_pLast(nullptr)
	{
	}
	template<typename ...ParamsT>
	explicit List(std::size_t uCount, const ParamsT &...vParams)
		: List()
	{
		AppendFill(uCount, vParams...);
	}
	template<class IteratorT>
	List(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: List()
	{
		AppendCopy(itBegin, itEnd);
	}
	template<class IteratorT>
	List(IteratorT itBegin, std::size_t uCount)
		: List()
	{
		AppendCopy(itBegin, uCount);
	}
	explicit List(std::initializer_list<ElementT> rhs)
		: List()
	{
		AppendCopy(rhs.begin(), rhs.size());
	}
	List(const List &rhs)
		: List()
	{
		for(auto pCur = rhs.xm_pFirst; pCur; pCur = pCur->xm_pNext){
			Push(pCur->Get());
		}
	}
	List(List &&rhs) noexcept
		: List()
	{
		Swap(rhs);
	}
	List &operator=(std::initializer_list<ElementT> rhs){
		if(&rhs != this){
			List(rhs).Swap(*this);
		}
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
	const xNode *GetFirst() const noexcept {
		return xm_pFirst;
	}
	xNode *GetFirst() noexcept {
		return xm_pFirst;
	}
	const xNode *GetLast() const noexcept {
		return xm_pLast;
	}
	xNode *GetLast() noexcept {
		return xm_pLast;
	}

	ConstCursor GetFirstCursor() const noexcept {
		return ConstCursor(GetFirst());
	}
	Cursor GetFirstCursor() noexcept {
		return Cursor(GetFirst());
	}
	ConstCursor GetLastCursor() const noexcept {
		return ConstCursor(GetLast());
	}
	Cursor GetLastCursor() noexcept {
		return Cursor(GetLast());
	}

	const ElementT &GetFront() const noexcept {
		ASSERT(!IsEmpty());
		return xm_pFirst->Get();
	}
	ElementT &GetFront() noexcept {
		ASSERT(!IsEmpty());
		return xm_pFirst->Get();
	}
	const ElementT &GetBack() const noexcept {
		ASSERT(!IsEmpty());
		return xm_pLast->Get();
	}
	ElementT &GetBack() noexcept {
		ASSERT(!IsEmpty());
		return xm_pLast->Get();
	}

	bool IsEmpty() const noexcept {
		return xm_pFirst == nullptr;
	}
	void Clear() noexcept {
		while(xm_pFirst){
			delete std::exchange(xm_pFirst, xm_pFirst->xm_pNext);
		}
		xm_pLast = nullptr;
	}

	template<typename ...ParamsT>
	xNode *Insert(xNode *pPos, ParamsT &&...vParams){
		auto pNode = new xNode(std::forward<ParamsT>(vParams)...);

		const auto pPrev = std::exchange((pPos ? pPos->xm_pPrev : xm_pLast), pNode);
		(pPrev ? pPrev->xm_pNext : xm_pFirst) = pNode;
		pNode->xm_pPrev = pPrev;
		pNode->xm_pNext = pPos;

		return pNode;
	}
	xNode *Erase(xNode *pNode) noexcept {
		const auto pNext = pNode->xm_pNext;

		const auto pOldPrev = pNode->xm_pPrev;
		(pOldPrev ? pOldPrev->xm_pNext : xm_pFirst) = pNext;
		(pNext ? pNext->xm_pPrev : xm_pLast) = pOldPrev;

		delete pNode;
		return pNext;
	}
	xNode *Erase(xNode *pBegin, xNode *pEnd) noexcept {
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

	xNode *Splice(xNode *pPos, List &lstSource) noexcept {
		const auto pRet = Splice(pPos, lstSource, lstSource.xm_pFirst, nullptr);
		ASSERT(lstSource.IsEmpty());
		return pRet;
	}
	xNode *Splice(xNode *pPos, List &lstSource, xNode *pSingle) noexcept {
		ASSERT(pSingle);
		return Splice(pPos, lstSource, pSingle, pSingle->xm_pNext);
	}
	xNode *Splice(xNode *pPos, List &lstSource, xNode *pBegin, xNode *pEnd) noexcept {
#ifndef NDEBUG
		{
			auto p = pBegin;
			while(p != pEnd){
				if(p == pPos){
					ASSERT_MSG(false, L"不能将链表的某个区间拼接到该区间内部。");
				}
				p = p->GetNext();
			}
		}
#endif

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

	xNode *Splice(xNode *pPos, List &&lstSource) noexcept {
		return Splice(pPos, lstSource);
	}
	xNode *Splice(xNode *pPos, List &&lstSource, xNode *pSingle) noexcept {
		return Splice(pPos, lstSource, pSingle);
	}
	xNode *Splice(xNode *pPos, List &&lstSource, xNode *pBegin, xNode *pEnd) noexcept {
		return Splice(pPos, lstSource, pBegin, pEnd);
	}

	template<typename ...ParamsT>
	xNode *Push(ParamsT &&...vParams){
		return Insert(nullptr, std::forward<ParamsT>(vParams)...);
	}
	void Pop() noexcept {
		Erase(xm_pLast);
	}

	template<typename ...ParamsT>
	xNode *Unshift(ParamsT &&...vParams){
		return Insert(xm_pFirst, std::forward<ParamsT>(vParams)...);
	}
	void Shift() noexcept {
		Erase(xm_pFirst);
	}

	template<typename ...ParamsT>
	void AppendFill(std::size_t uCount, const ParamsT &...vParams){
		std::size_t i = 0;
		try {
			while(i < uCount){
				Push(vParams...);
				++i;
			}
		} catch(...){
			while(i > 0){
				Pop();
				--i;
			}
			throw;
		}
	}
	template<class IteratorT>
	void AppendCopy(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		std::size_t i = 0;
		try {
			while(itBegin != itEnd){
				Push(*itBegin);
				++itBegin;
				++i;
			}
		} catch(...){
			while(i > 0){
				Pop();
				--i;
			}
			throw;
		}
	}
	template<class IteratorT>
	void AppendCopy(IteratorT itBegin, std::size_t uCount){
		std::size_t i = 0;
		try {
			while(i < uCount){
				Push(*itBegin);
				++itBegin;
				++i;
			}
		} catch(...){
			while(i > 0){
				Pop();
				--i;
			}
			throw;
		}
	}
	void Truncate(std::size_t uCount) noexcept {
		for(std::size_t i = 0; i < uCount; ++i){
			Pop();
		}
	}

	template<typename ...ParamsT>
	void PrependFill(std::size_t uCount, const ParamsT &...vParams){
		std::size_t i = 0;
		try {
			while(i < uCount){
				Unshift(vParams...);
				++i;
			}
		} catch(...){
			while(i > 0){
				Shift();
				--i;
			}
			throw;
		}
	}
	template<class IteratorT>
	void PrependCopy(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		std::size_t i = 0;
		try {
			while(itBegin != itEnd){
				Unshift(*itBegin);
				++itBegin;
				++i;
			}
		} catch(...){
			while(i > 0){
				Shift();
				--i;
			}
			throw;
		}
	}
	template<class IteratorT>
	void PrependCopy(IteratorT itBegin, std::size_t uCount){
		std::size_t i = 0;
		try {
			while(i < uCount){
				Unshift(*itBegin);
				++itBegin;
				++i;
			}
		} catch(...){
			while(i > 0){
				Shift();
				--i;
			}
			throw;
		}
	}
	void CutOff(std::size_t uCount) noexcept {
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

	// std::insert_iterator
	template<typename ParamT>
	xNode *insert(xNode *pPos, ParamT &&vParams){
		return Insert(pPos, std::forward<ParamT>(vParams));
	}
};

template<class ElementT>
void swap(List<ElementT> &lhs, List<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<class ElementT>
auto begin(const List<ElementT> &lhs) noexcept {
	return lhs.GetFirstCursor();
}
template<class ElementT>
auto begin(List<ElementT> &lhs) noexcept {
	return lhs.GetFirstCursor();
}
template<class ElementT>
auto cbegin(const List<ElementT> &lhs) noexcept {
	return lhs.GetFirstCursor();
}
template<class ElementT>
auto end(const List<ElementT> &) noexcept {
	return typename List<ElementT>::ConstCursor();
}
template<class ElementT>
auto end(List<ElementT> &) noexcept {
	return typename List<ElementT>::Cursor();
}
template<class ElementT>
auto cend(const List<ElementT> &) noexcept {
	return typename List<ElementT>::ConstCursor();
}

}

#endif
