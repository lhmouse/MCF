// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_DEQUE_HPP_
#define MCF_CONTAINERS_DEQUE_HPP_

#include "../Utilities/ConstructDestruct.hpp"
#include "List.hpp"

namespace MCF {

template<class ElementT>
class Deque {
private:
	class xChunk {
	public:
		enum : std::size_t {
			CHUNK_SIZE = 255 / sizeof(ElementT) + 1
		};

	private:
		alignas(ElementT) char xm_aachStorage[CHUNK_SIZE][sizeof(ElementT)];
		ElementT *xm_pBegin;
		ElementT *xm_pEnd;

	public:
		explicit xChunk(bool bSeekToBegin) noexcept
			: xm_pBegin(reinterpret_cast<ElementT *>(
				bSeekToBegin ? std::begin(xm_aachStorage) : std::end(xm_aachStorage)))
			, xm_pEnd(xm_pBegin)
		{
		}
		xChunk(const xChunk &rhs)
			noexcept(std::is_nothrow_copy_constructible<ElementT>::value)
			: xChunk(true)
		{
			for(auto pCur = rhs.xm_pBegin; pCur != xm_pEnd; ++pCur){
				UncheckedPush(*pCur);
			}
		}
		xChunk(xChunk &&rhs)
			noexcept(std::is_nothrow_move_constructible<ElementT>::value)
			: xChunk(true)
		{
			for(auto pCur = rhs.xm_pBegin; pCur != xm_pEnd; ++pCur){
				UncheckedPush(std::move(*pCur));
			}
		}
		xChunk &operator=(const xChunk &rhs)
			noexcept(std::is_nothrow_copy_constructible<ElementT>::value)
		{
			Clear(true);
			for(auto pCur = rhs.xm_pBegin; pCur != xm_pEnd; ++pCur){
				UncheckedPush(*pCur);
			}
			return *this;
		}
		xChunk &operator=(xChunk &&rhs)
			noexcept(std::is_nothrow_move_constructible<ElementT>::value)
		{
			Clear(true);
			for(auto pCur = rhs.xm_pBegin; pCur != xm_pEnd; ++pCur){
				UncheckedPush(std::move(*pCur));
			}
			return *this;
		}
		~xChunk(){
			Clear(true);
		}

	public:
		const ElementT *GetBegin() const noexcept {
			return xm_pBegin;
		}
		ElementT *GetBegin() noexcept {
			return xm_pBegin;
		}
		const ElementT *GetEnd() const noexcept {
			return xm_pEnd;
		}
		ElementT *GetEnd() noexcept {
			return xm_pEnd;
		}

		std::size_t GetSize() const noexcept {
			return (std::size_t)(xm_pEnd - xm_pBegin);
		}
		std::size_t GetPushableSize() const noexcept {
			return (std::size_t)(reinterpret_cast<const ElementT *>(std::end(xm_aachStorage)) - xm_pEnd);
		}
		std::size_t GetUnshiftableSize() const noexcept {
			return (std::size_t)(xm_pBegin - reinterpret_cast<const ElementT *>(std::begin(xm_aachStorage)));
		}
		void Clear(bool bSeekToBegin) noexcept {
			for(auto pCur = xm_pBegin; pCur != xm_pEnd; ++pCur){
				pCur->~ElementT();
			}
			xm_pBegin = reinterpret_cast<ElementT *>(
				bSeekToBegin ? std::begin(xm_aachStorage) : std::end(xm_aachStorage));
			xm_pEnd = xm_pBegin;
		}

		template<typename ...ParamsT>
		ElementT *UncheckedPush(ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		{
			ASSERT(GetPushableSize() > 0);

			const auto pRet = xm_pEnd;
			DefaultConstruct(pRet, std::forward<ParamsT>(vParams)...);
			++xm_pEnd;
			return pRet;
		}
		void UncheckedPop() noexcept {
			ASSERT(GetSize() > 0);

			--xm_pEnd;
			Destruct(xm_pEnd);
		}

		template<typename ...ParamsT>
		ElementT *UncheckedUnshift(ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		{
			ASSERT(GetUnshiftableSize() > 0);

			const auto pRet = xm_pBegin - 1;
			DefaultConstruct(pRet, std::forward<ParamsT>(vParams)...);
			--xm_pBegin;
			return pRet;
		}
		void UncheckedShift() noexcept {
			ASSERT(GetSize() > 0);

			Destruct(xm_pBegin);
			++xm_pBegin;
		}
	};

	template<typename CursorT, typename RealElementT, typename RealNodeT>
	class xCursorTemplate
		: public std::iterator<std::bidirectional_iterator_tag, RealElementT>
	{
	protected:
		RealElementT *xm_pElement;
		RealNodeT *xm_pNode;

	protected:
		constexpr xCursorTemplate(RealElementT *pElement, RealNodeT *pNode) noexcept
			: xm_pElement(pElement), xm_pNode(pNode)
		{
		}

	public:
		constexpr xCursorTemplate() noexcept
			: xCursorTemplate(nullptr, nullptr)
		{
		}

	public:
		bool operator==(const xCursorTemplate &rhs) const noexcept {
			return xm_pElement == rhs.xm_pElement;
		}
		bool operator!=(const xCursorTemplate &rhs) const noexcept {
			return xm_pElement != rhs.xm_pElement;
		}

		RealElementT &operator*() const noexcept {
			ASSERT_MSG(xm_pElement, L"游标指向队列两端或者为空。");
			return *xm_pElement;
		}
		RealElementT *operator->() const noexcept {
			ASSERT_MSG(xm_pElement, L"游标指向队列两端或者为空。");
			return xm_pElement;
		}

		CursorT &operator++() noexcept {
			ASSERT_MSG(xm_pNode, L"空游标不能移动。");

			if(xm_pElement != (xm_pNode->Get().GetEnd() - 1)){
				++xm_pElement;
			} else {
				xm_pNode = xm_pNode->GetNext();
				xm_pElement = xm_pNode ? xm_pNode->Get().GetBegin() : nullptr;
			}
			return static_cast<CursorT &>(*this);
		}
		CursorT &operator--() noexcept {
			ASSERT_MSG(xm_pNode, L"空游标不能移动。");

			if(xm_pElement != xm_pNode->Get().GetBegin()){
				--xm_pElement;
			} else {
				xm_pNode = xm_pNode->GetPrev();
				xm_pElement = xm_pNode ? (xm_pNode->Get().GetEnd() - 1) : nullptr;
			}
			return static_cast<CursorT &>(*this);
		}

		CursorT operator++(int) noexcept {
			CursorT ret(xm_pElement, xm_pNode);
			++*this;
			return ret;
		}
		CursorT operator--(int) noexcept {
			CursorT ret(xm_pElement, xm_pNode);
			--*this;
			return ret;
		}

		explicit operator bool() const noexcept {
			return xm_pElement != nullptr;
		}
	};

	using xChunkNode = typename List<xChunk>::Node;

public:
	class ConstCursor;

	class Cursor
		: public xCursorTemplate<Cursor, ElementT, xChunkNode>
	{
		friend Deque;
		friend ConstCursor;

	private:
		using xBase = xCursorTemplate<Cursor, ElementT, xChunkNode>;

	private:
		constexpr Cursor(ElementT *pElement, xChunkNode *pNode) noexcept
			: xBase(pElement, pNode)
		{
		}

	public:
		constexpr Cursor() noexcept = default;
	};

	class ConstCursor
		: public xCursorTemplate<ConstCursor, const ElementT, const xChunkNode>
	{
		friend Deque;

	private:
		using xBase = xCursorTemplate<ConstCursor, const ElementT, const xChunkNode>;

	private:
		constexpr ConstCursor(const ElementT *pElement, const xChunkNode *pNode) noexcept
			: xBase(pElement, pNode)
		{
		}

	public:
		constexpr ConstCursor() noexcept = default;

		constexpr ConstCursor(const Cursor &rhs) noexcept
			: xBase(rhs.xm_pElement, rhs.xm_pNode)
		{
		}
	};

private:
	List<xChunk> xm_lstChunks;

public:
	constexpr Deque() noexcept = default;
	Deque(Deque &&) noexcept = default;
	Deque &operator=(Deque &&) noexcept = default;

	template<typename ...ParamsT>
	explicit Deque(std::size_t uCount, const ParamsT &...vParams)
		: Deque()
	{
		AppendFill(uCount, vParams...);
	}
	template<class IteratorT>
	Deque(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: Deque()
	{
		AppendCopy(itBegin, itEnd);
	}
	template<class IteratorT>
	Deque(IteratorT itBegin, std::size_t uCount)
		: Deque()
	{
		AppendCopy(itBegin, uCount);
	}
	Deque(std::initializer_list<ElementT> rhs)
		: Deque()
	{
		AppendCopy(rhs.begin(), rhs.size());
	}
	Deque(const Deque &rhs)
		: Deque()
	{
		AppendCopy(rhs.GetFirstCursor(), ConstCursor());
	}
	Deque &operator=(std::initializer_list<ElementT> rhs){
		Deque(rhs).Swap(*this);
		return *this;
	}
	Deque &operator=(const Deque &rhs){
		if(&rhs != this){
			Deque(rhs).Swap(*this);
		}
		return *this;
	}

public:
	ConstCursor GetFirstCursor() const noexcept {
		const auto pFirstNode = xm_lstChunks.GetFirst();
		if(!pFirstNode){
			return ConstCursor();
		}
		ASSERT(pFirstNode->Get().GetSize() > 0);
		return ConstCursor(pFirstNode->Get().GetBegin(), pFirstNode);
	}
	Cursor GetFirstCursor() noexcept {
		const auto pFirstNode = xm_lstChunks.GetFirst();
		if(!pFirstNode){
			return Cursor();
		}
		ASSERT(pFirstNode->Get().GetSize() > 0);
		return Cursor(pFirstNode->Get().GetBegin(), pFirstNode);
	}
	ConstCursor GetLastCursor() const noexcept {
		const auto pLastNode = xm_lstChunks.GetLast();
		if(!pLastNode){
			return ConstCursor();
		}
		ASSERT(pLastNode->Get().GetSize() > 0);
		return ConstCursor(pLastNode->Get().GetEnd() - 1, pLastNode);
	}
	Cursor GetLastCursor() noexcept {
		const auto pLastNode = xm_lstChunks.GetLast();
		if(!pLastNode){
			return Cursor();
		}
		ASSERT(pLastNode->Get().GetSize() > 0);
		return Cursor(pLastNode->Get().GetEnd() - 1, pLastNode);
	}

	const ElementT &GetFront() const noexcept {
		const auto pFirstNode = xm_lstChunks.GetFirst();
		ASSERT(pFirstNode && (pFirstNode->Get().GetSize() > 0));
		return pFirstNode->Get().GetBegin()[0];
	}
	ElementT &GetFront() noexcept {
		const auto pFirstNode = xm_lstChunks.GetFirst();
		ASSERT(pFirstNode && (pFirstNode->Get().GetSize() > 0));
		return pFirstNode->Get().GetBegin()[0];
	}
	const ElementT &GetBack() const noexcept {
		const auto pLastNode = xm_lstChunks.GetLast();
		ASSERT(pLastNode && (pLastNode->Get().GetSize() > 0));
		return pLastNode->Get().GetEnd()[-1];
	}
	ElementT &GetBack() noexcept {
		const auto pLastNode = xm_lstChunks.GetLast();
		ASSERT(pLastNode && (pLastNode->Get().GetSize() > 0));
		return pLastNode->Get().GetEnd()[-1];
	}

	bool IsEmpty() const noexcept {
		return xm_lstChunks.IsEmpty();
	}
	void Clear() noexcept {
		xm_lstChunks.Clear();
	}

	void Splice(Deque &clsSource) noexcept {
		xm_lstChunks.Splice(nullptr, clsSource.xm_lstChunks);
		ASSERT(clsSource.IsEmpty());
	}
	void Splice(Deque &&clsSource) noexcept {
		xm_lstChunks.Splice(nullptr, std::move(clsSource.xm_lstChunks));
		ASSERT(clsSource.IsEmpty());
	}

	template<typename ...ParamsT>
	ElementT *Push(ParamsT &&...vParams){
		auto pLastNode = xm_lstChunks.GetLast();
		if(pLastNode && (pLastNode->Get().GetPushableSize() > 0)){
			return pLastNode->Get().UncheckedPush(std::forward<ParamsT>(vParams)...);
		}

		List<xChunk> lstTemp;
		pLastNode = lstTemp.Push(true);
		const auto pRet = pLastNode->Get().UncheckedPush(std::forward<ParamsT>(vParams)...);
		xm_lstChunks.Splice(nullptr, lstTemp);
		return pRet;
	}
	void Pop() noexcept {
		const auto pLastNode = xm_lstChunks.GetLast();
		ASSERT(pLastNode && (pLastNode->Get().GetSize() > 0));
		pLastNode->Get().UncheckedPop();
		if(pLastNode->Get().GetSize() == 0){
			xm_lstChunks.Pop();
		}
	}

	template<typename ...ParamsT>
	ElementT *Unshift(ParamsT &&...vParams){
		auto pFirstNode = xm_lstChunks.GetFirst();
		if(pFirstNode && (pFirstNode->Get().GetUnshiftableSize() > 0)){
			return pFirstNode->Get().UncheckedUnshift(std::forward<ParamsT>(vParams)...);
		}

		List<xChunk> lstTemp;
		pFirstNode = lstTemp.Push(false);
		const auto pRet = pFirstNode->Get().UncheckedUnshift(std::forward<ParamsT>(vParams)...);
		xm_lstChunks.Splice(nullptr, lstTemp);
		return pRet;
	}
	void Shift() noexcept {
		const auto pFirstNode = xm_lstChunks.GetFirst();
		ASSERT(pFirstNode && (pFirstNode->Get().GetSize() > 0));
		pFirstNode->Get().UncheckedShift();
		if(pFirstNode->Get().GetSize() == 0){
			xm_lstChunks.Shift();
		}
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

	void Swap(Deque &rhs) noexcept {
		xm_lstChunks.Swap(rhs.xm_lstChunks);
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
void swap(Deque<ElementT> &lhs, Deque<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<class ElementT>
auto begin(const Deque<ElementT> &lhs) noexcept {
	return lhs.GetFirstCursor();
}
template<class ElementT>
auto begin(Deque<ElementT> &lhs) noexcept {
	return lhs.GetFirstCursor();
}
template<class ElementT>
auto cbegin(const Deque<ElementT> &lhs) noexcept {
	return lhs.GetFirstCursor();
}
template<class ElementT>
auto end(const Deque<ElementT> &) noexcept {
	return typename Deque<ElementT>::ConstCursor();
}
template<class ElementT>
auto end(Deque<ElementT> &) noexcept {
	return typename Deque<ElementT>::Cursor();
}
template<class ElementT>
auto cend(const Deque<ElementT> &) noexcept {
	return typename Deque<ElementT>::ConstCursor();
}

}

#endif
