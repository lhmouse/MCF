// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

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
		ElementT x_vElement;
		xNode *x_pPrev;
		xNode *x_pNext;

	private:
		xNode() = default;

		template<typename ...ParamsT>
		explicit xNode(ParamsT &&...vParams)
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

		const xNode *GetPrev() const noexcept {
			return x_pPrev;
		}
		xNode *GetPrev() noexcept {
			return x_pPrev;
		}
		const xNode *GetNext() const noexcept {
			return x_pNext;
		}
		xNode *GetNext() noexcept {
			return x_pNext;
		}
	};

	template<typename CursorT, typename RealElementT, typename RealNodeT>
	class xCursorTemplate
		: public std::iterator<std::forward_iterator_tag, RealElementT>
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
			_MCFCRT_ASSERT_MSG(x_pNode, L"游标指向链表两端或者为空。");
			return x_pNode->Get();
		}
		RealElementT *operator->() const noexcept {
			_MCFCRT_ASSERT_MSG(x_pNode, L"游标指向链表两端或者为空。");
			return std::addressof(x_pNode->Get());
		}

		CursorT &operator++() noexcept {
			_MCFCRT_ASSERT_MSG(x_pNode, L"空游标不能移动。");

			x_pNode = x_pNode->GetNext();
			return static_cast<CursorT &>(*this);
		}
		CursorT &operator--() noexcept {
			_MCFCRT_ASSERT_MSG(x_pNode, L"空游标不能移动。");

			x_pNode = x_pNode->GetPrev();
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
		explicit constexpr Cursor(Node *pNode) noexcept
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
		explicit constexpr ConstCursor(const Node *pNode) noexcept
			: xBase(pNode)
		{
		}

	public:
		constexpr ConstCursor() noexcept = default;

		constexpr ConstCursor(const Cursor &rhs) noexcept
			: xBase(rhs.x_pNode)
		{
		}
	};

private:
	Node *x_pFirst;
	Node *x_pLast;

public:
	constexpr List() noexcept
		: x_pFirst(nullptr), x_pLast(nullptr)
	{
	}
	template<typename ...ParamsT>
	explicit List(std::size_t uCount, const ParamsT &...vParams)
		: List()
	{
		AppendFill(uCount, vParams...);
	}
	template<class IteratorT,
		typename std::iterator_traits<IteratorT>::iterator_category * = nullptr>
	List(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: List()
	{
		AppendCopy(itBegin, itEnd);
	}
	template<class IteratorT,
		typename std::iterator_traits<IteratorT>::iterator_category * = nullptr>
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
		for(auto pCur = rhs.x_pFirst; pCur; pCur = pCur->x_pNext){
			Push(pCur->Get());
		}
	}
	List(List &&rhs) noexcept
		: List()
	{
		Swap(rhs);
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
		return x_pFirst;
	}
	Node *GetFirst() noexcept {
		return x_pFirst;
	}
	const Node *GetLast() const noexcept {
		return x_pLast;
	}
	Node *GetLast() noexcept {
		return x_pLast;
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
		_MCFCRT_ASSERT(!IsEmpty());
		return x_pFirst->Get();
	}
	ElementT &GetFront() noexcept {
		_MCFCRT_ASSERT(!IsEmpty());
		return x_pFirst->Get();
	}
	const ElementT &GetBack() const noexcept {
		_MCFCRT_ASSERT(!IsEmpty());
		return x_pLast->Get();
	}
	ElementT &GetBack() noexcept {
		_MCFCRT_ASSERT(!IsEmpty());
		return x_pLast->Get();
	}

	bool IsEmpty() const noexcept {
		return x_pFirst == nullptr;
	}
	void Clear() noexcept {
		while(x_pFirst){
			delete std::exchange(x_pFirst, x_pFirst->x_pNext);
		}
		x_pLast = nullptr;
	}

	template<typename ...ParamsT>
	Node *Insert(Node *pPos, ParamsT &&...vParams){
		auto pNode = new Node(std::forward<ParamsT>(vParams)...);

		const auto pPrev = std::exchange((pPos ? pPos->x_pPrev : x_pLast), pNode);
		(pPrev ? pPrev->x_pNext : x_pFirst) = pNode;
		pNode->x_pPrev = pPrev;
		pNode->x_pNext = pPos;

		return pNode;
	}
	Node *Erase(Node *pNode) noexcept {
		const auto pNext = pNode->x_pNext;

		const auto pOldPrev = pNode->x_pPrev;
		(pOldPrev ? pOldPrev->x_pNext : x_pFirst) = pNext;
		(pNext ? pNext->x_pPrev : x_pLast) = pOldPrev;

		delete pNode;
		return pNext;
	}
	Node *Erase(Node *pBegin, Node *pEnd) noexcept {
		if(pBegin != pEnd){
			const auto pOldPrev = pBegin->x_pPrev;
			(pOldPrev ? pOldPrev->x_pNext : x_pFirst) = pEnd;
			(pEnd ? pEnd->x_pPrev : x_pLast) = pOldPrev;

			do {
				const auto pNode = pBegin;
				pBegin = pBegin->x_pNext;

				delete pNode;
			} while(pBegin != pEnd);
		}
		return pEnd;
	}

	Node *Splice(Node *pPos, List &lstSource) noexcept {
		const auto pRet = Splice(pPos, lstSource, lstSource.x_pFirst, nullptr);
		_MCFCRT_ASSERT(lstSource.IsEmpty());
		return pRet;
	}
	Node *Splice(Node *pPos, List &lstSource, Node *pSingle) noexcept {
		_MCFCRT_ASSERT(pSingle);
		return Splice(pPos, lstSource, pSingle, pSingle->x_pNext);
	}
	Node *Splice(Node *pPos, List &lstSource, Node *pBegin, Node *pEnd) noexcept {
#ifndef NDEBUG
		{
			auto p = pBegin;
			while(p != pEnd){
				if(p == pPos){
					_MCFCRT_ASSERT_MSG(false, L"不能将链表的某个区间拼接到该区间内部。");
				}
				p = p->GetNext();
			}
		}
#endif

		if(pBegin != pEnd){
			const auto pOldPrev = pBegin->x_pPrev;
			(pOldPrev ? pOldPrev->x_pNext : lstSource.x_pFirst) = pEnd;
			const auto pOldLast = std::exchange((pEnd ? pEnd->x_pPrev : lstSource.x_pLast), pOldPrev);

			const auto pPrev = std::exchange((pPos ? pPos->x_pPrev : x_pLast), pOldLast);
			pOldLast->x_pNext = pPos;

			(pPrev ? pPrev->x_pNext : x_pFirst) = pBegin;
			pBegin->x_pPrev = pPrev;
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
		Erase(x_pLast);
	}

	template<typename ...ParamsT>
	Node *Unshift(ParamsT &&...vParams){
		return Insert(x_pFirst, std::forward<ParamsT>(vParams)...);
	}
	void Shift() noexcept {
		Erase(x_pFirst);
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
		std::swap(x_pFirst, rhs.x_pFirst);
		std::swap(x_pLast, rhs.x_pLast);
	}

	void Reverse() noexcept {
		for(auto pCur = x_pFirst; pCur; pCur = pCur->x_pPrev){
			std::swap(pCur->x_pPrev, pCur->x_pNext);
		}
		std::swap(x_pFirst, x_pLast);
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
	Node *insert(Node *pPos, ParamT &&vParams){
		return Insert(pPos, std::forward<ParamT>(vParams));
	}
};

template<class ElementT>
void swap(List<ElementT> &lhs, List<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<class ElementT>
auto begin(const List<ElementT> &rhs) noexcept {
	return rhs.GetFirstCursor();
}
template<class ElementT>
auto begin(List<ElementT> &rhs) noexcept {
	return rhs.GetFirstCursor();
}
template<class ElementT>
auto cbegin(const List<ElementT> &rhs) noexcept {
	return rhs.GetFirstCursor();
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
