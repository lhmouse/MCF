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
			kChunkSize = 255 / sizeof(ElementT) + 1
		};

	private:
		alignas(ElementT) char x_aachStorage[kChunkSize][sizeof(ElementT)];
		ElementT *x_pBegin;
		ElementT *x_pEnd;

	public:
		explicit xChunk(bool bSeekToBegin) noexcept
			: x_pBegin(reinterpret_cast<ElementT *>(
				bSeekToBegin ? std::begin(x_aachStorage) : std::end(x_aachStorage)))
			, x_pEnd(x_pBegin)
		{
		}
		xChunk(const xChunk &rhs)
			noexcept(std::is_nothrow_copy_constructible<ElementT>::value)
			: xChunk(true)
		{
			for(auto pCur = rhs.x_pBegin; pCur != x_pEnd; ++pCur){
				UncheckedPush(*pCur);
			}
		}
		xChunk(xChunk &&rhs)
			noexcept(std::is_nothrow_move_constructible<ElementT>::value)
			: xChunk(true)
		{
			for(auto pCur = rhs.x_pBegin; pCur != x_pEnd; ++pCur){
				UncheckedPush(std::move(*pCur));
			}
		}
		xChunk &operator=(const xChunk &rhs)
			noexcept(std::is_nothrow_copy_constructible<ElementT>::value)
		{
			Clear(true);
			for(auto pCur = rhs.x_pBegin; pCur != x_pEnd; ++pCur){
				UncheckedPush(*pCur);
			}
			return *this;
		}
		xChunk &operator=(xChunk &&rhs)
			noexcept(std::is_nothrow_move_constructible<ElementT>::value)
		{
			Clear(true);
			for(auto pCur = rhs.x_pBegin; pCur != x_pEnd; ++pCur){
				UncheckedPush(std::move(*pCur));
			}
			return *this;
		}
		~xChunk(){
			Clear(true);
		}

	public:
		const ElementT *GetBegin() const noexcept {
			return x_pBegin;
		}
		ElementT *GetBegin() noexcept {
			return x_pBegin;
		}
		const ElementT *GetEnd() const noexcept {
			return x_pEnd;
		}
		ElementT *GetEnd() noexcept {
			return x_pEnd;
		}

		std::size_t GetSize() const noexcept {
			return (std::size_t)(x_pEnd - x_pBegin);
		}
		std::size_t GetPushableSize() const noexcept {
			return (std::size_t)(reinterpret_cast<const ElementT *>(std::end(x_aachStorage)) - x_pEnd);
		}
		std::size_t GetUnshiftableSize() const noexcept {
			return (std::size_t)(x_pBegin - reinterpret_cast<const ElementT *>(std::begin(x_aachStorage)));
		}
		void Clear(bool bSeekToBegin) noexcept {
			for(auto pCur = x_pBegin; pCur != x_pEnd; ++pCur){
				pCur->~ElementT();
			}
			x_pBegin = reinterpret_cast<ElementT *>(
				bSeekToBegin ? std::begin(x_aachStorage) : std::end(x_aachStorage));
			x_pEnd = x_pBegin;
		}

		template<typename ...ParamsT>
		ElementT *UncheckedPush(ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		{
			ASSERT(GetPushableSize() > 0);

			const auto pRet = x_pEnd;
			DefaultConstruct(pRet, std::forward<ParamsT>(vParams)...);
			++x_pEnd;
			return pRet;
		}
		void UncheckedPop() noexcept {
			ASSERT(GetSize() > 0);

			--x_pEnd;
			Destruct(x_pEnd);
		}

		template<typename ...ParamsT>
		ElementT *UncheckedUnshift(ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		{
			ASSERT(GetUnshiftableSize() > 0);

			const auto pRet = x_pBegin - 1;
			DefaultConstruct(pRet, std::forward<ParamsT>(vParams)...);
			--x_pBegin;
			return pRet;
		}
		void UncheckedShift() noexcept {
			ASSERT(GetSize() > 0);

			Destruct(x_pBegin);
			++x_pBegin;
		}
	};

	template<typename CursorT, typename RealElementT, typename RealNodeT>
	class xCursorTemplate
		: public std::iterator<std::bidirectional_iterator_tag, RealElementT>
	{
	protected:
		RealElementT *x_pElement;
		RealNodeT *x_pNode;

	protected:
		constexpr xCursorTemplate(RealElementT *pElement, RealNodeT *pNode) noexcept
			: x_pElement(pElement), x_pNode(pNode)
		{
		}

	public:
		constexpr xCursorTemplate() noexcept
			: xCursorTemplate(nullptr, nullptr)
		{
		}

	public:
		bool operator==(const xCursorTemplate &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const xCursorTemplate &rhs) const noexcept {
			return x_pElement != rhs.x_pElement;
		}

		RealElementT &operator*() const noexcept {
			ASSERT_MSG(x_pElement, L"游标指向队列两端或者为空。");
			return *x_pElement;
		}
		RealElementT *operator->() const noexcept {
			ASSERT_MSG(x_pElement, L"游标指向队列两端或者为空。");
			return x_pElement;
		}

		CursorT &operator++() noexcept {
			ASSERT_MSG(x_pNode, L"空游标不能移动。");

			if(x_pElement != (x_pNode->Get().GetEnd() - 1)){
				++x_pElement;
			} else {
				x_pNode = x_pNode->GetNext();
				x_pElement = x_pNode ? x_pNode->Get().GetBegin() : nullptr;
			}
			return static_cast<CursorT &>(*this);
		}
		CursorT &operator--() noexcept {
			ASSERT_MSG(x_pNode, L"空游标不能移动。");

			if(x_pElement != x_pNode->Get().GetBegin()){
				--x_pElement;
			} else {
				x_pNode = x_pNode->GetPrev();
				x_pElement = x_pNode ? (x_pNode->Get().GetEnd() - 1) : nullptr;
			}
			return static_cast<CursorT &>(*this);
		}

		CursorT operator++(int) noexcept {
			CursorT ret(x_pElement, x_pNode);
			++*this;
			return ret;
		}
		CursorT operator--(int) noexcept {
			CursorT ret(x_pElement, x_pNode);
			--*this;
			return ret;
		}

		explicit operator bool() const noexcept {
			return x_pElement != nullptr;
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
			: xBase(rhs.x_pElement, rhs.x_pNode)
		{
		}
	};

private:
	List<xChunk> x_lstChunks;

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
		const auto pFirstNode = x_lstChunks.GetFirst();
		if(!pFirstNode){
			return ConstCursor();
		}
		ASSERT(pFirstNode->Get().GetSize() > 0);
		return ConstCursor(pFirstNode->Get().GetBegin(), pFirstNode);
	}
	Cursor GetFirstCursor() noexcept {
		const auto pFirstNode = x_lstChunks.GetFirst();
		if(!pFirstNode){
			return Cursor();
		}
		ASSERT(pFirstNode->Get().GetSize() > 0);
		return Cursor(pFirstNode->Get().GetBegin(), pFirstNode);
	}
	ConstCursor GetLastCursor() const noexcept {
		const auto pLastNode = x_lstChunks.GetLast();
		if(!pLastNode){
			return ConstCursor();
		}
		ASSERT(pLastNode->Get().GetSize() > 0);
		return ConstCursor(pLastNode->Get().GetEnd() - 1, pLastNode);
	}
	Cursor GetLastCursor() noexcept {
		const auto pLastNode = x_lstChunks.GetLast();
		if(!pLastNode){
			return Cursor();
		}
		ASSERT(pLastNode->Get().GetSize() > 0);
		return Cursor(pLastNode->Get().GetEnd() - 1, pLastNode);
	}

	const ElementT &GetFront() const noexcept {
		const auto pFirstNode = x_lstChunks.GetFirst();
		ASSERT(pFirstNode && (pFirstNode->Get().GetSize() > 0));
		return pFirstNode->Get().GetBegin()[0];
	}
	ElementT &GetFront() noexcept {
		const auto pFirstNode = x_lstChunks.GetFirst();
		ASSERT(pFirstNode && (pFirstNode->Get().GetSize() > 0));
		return pFirstNode->Get().GetBegin()[0];
	}
	const ElementT &GetBack() const noexcept {
		const auto pLastNode = x_lstChunks.GetLast();
		ASSERT(pLastNode && (pLastNode->Get().GetSize() > 0));
		return pLastNode->Get().GetEnd()[-1];
	}
	ElementT &GetBack() noexcept {
		const auto pLastNode = x_lstChunks.GetLast();
		ASSERT(pLastNode && (pLastNode->Get().GetSize() > 0));
		return pLastNode->Get().GetEnd()[-1];
	}

	bool IsEmpty() const noexcept {
		return x_lstChunks.IsEmpty();
	}
	void Clear() noexcept {
		x_lstChunks.Clear();
	}

	void Splice(Deque &clsSource) noexcept {
		x_lstChunks.Splice(nullptr, clsSource.x_lstChunks);
		ASSERT(clsSource.IsEmpty());
	}
	void Splice(Deque &&clsSource) noexcept {
		x_lstChunks.Splice(nullptr, std::move(clsSource.x_lstChunks));
		ASSERT(clsSource.IsEmpty());
	}

	template<typename ...ParamsT>
	ElementT *Push(ParamsT &&...vParams){
		auto pLastNode = x_lstChunks.GetLast();
		if(pLastNode && (pLastNode->Get().GetPushableSize() > 0)){
			return pLastNode->Get().UncheckedPush(std::forward<ParamsT>(vParams)...);
		}

		List<xChunk> lstTemp;
		pLastNode = lstTemp.Push(true);
		const auto pRet = pLastNode->Get().UncheckedPush(std::forward<ParamsT>(vParams)...);
		x_lstChunks.Splice(nullptr, lstTemp);
		return pRet;
	}
	void Pop() noexcept {
		const auto pLastNode = x_lstChunks.GetLast();
		ASSERT(pLastNode && (pLastNode->Get().GetSize() > 0));
		pLastNode->Get().UncheckedPop();
		if(pLastNode->Get().GetSize() == 0){
			x_lstChunks.Pop();
		}
	}

	template<typename ...ParamsT>
	ElementT *Unshift(ParamsT &&...vParams){
		auto pFirstNode = x_lstChunks.GetFirst();
		if(pFirstNode && (pFirstNode->Get().GetUnshiftableSize() > 0)){
			return pFirstNode->Get().UncheckedUnshift(std::forward<ParamsT>(vParams)...);
		}

		List<xChunk> lstTemp;
		pFirstNode = lstTemp.Push(false);
		const auto pRet = pFirstNode->Get().UncheckedUnshift(std::forward<ParamsT>(vParams)...);
		x_lstChunks.Splice(nullptr, lstTemp);
		return pRet;
	}
	void Shift() noexcept {
		const auto pFirstNode = x_lstChunks.GetFirst();
		ASSERT(pFirstNode && (pFirstNode->Get().GetSize() > 0));
		pFirstNode->Get().UncheckedShift();
		if(pFirstNode->Get().GetSize() == 0){
			x_lstChunks.Shift();
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
		x_lstChunks.Swap(rhs.x_lstChunks);
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
