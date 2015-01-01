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
		friend Deque;

	public:
		enum : std::size_t {
			MAX_ELEMENT_COUNT = 255 / sizeof(ElementT) + 1
		};

	private:
		alignas(ElementT) char xm_aaStorage[MAX_ELEMENT_COUNT][sizeof(ElementT)];
		ElementT *xm_pBegin;
		ElementT *xm_pEnd;

	public:
		explicit xChunk(bool bSeekToBegin) noexcept
			: xm_pBegin(reinterpret_cast<ElementT *>(bSeekToBegin ? std::begin(xm_aaStorage) : std::end(xm_aaStorage)))
			, xm_pEnd(xm_pBegin)
		{
		}
		xChunk(const xChunk &rhs)
			: xChunk(true)
		{
			for(auto pElement = rhs.xm_pBegin; pElement != rhs.xm_pEnd; ++pElement){
				UncheckedPush(*pElement);
			}
		}
		xChunk(xChunk &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value)
			: xChunk(true)
		{
			for(auto pElement = rhs.xm_pBegin; pElement != rhs.xm_pEnd; ++pElement){
				UncheckedPush(std::move(*pElement));
			}
		}
		xChunk &operator=(const xChunk &rhs){
			if(this != &rhs){
				xClear(true);
				for(auto pElement = rhs.xm_pBegin; pElement != rhs.xm_pEnd; ++pElement){
					UncheckedPush(*pElement);
				}
			}
			return *this;
		}
		xChunk &operator=(xChunk &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
			ASSERT(this != &rhs);

			xClear(true);
			for(auto pElement = rhs.xm_pBegin; pElement != rhs.xm_pEnd; ++pElement){
				UncheckedPush(std::move(*pElement));
			}
			return *this;
		}
		~xChunk(){
			xClear(true);
		}

	private:
		void xClear(bool bSeekToBegin) noexcept {
			while(xm_pBegin != xm_pEnd){
				xm_pBegin->~ElementT();
				++xm_pBegin;
			}
			xm_pBegin = reinterpret_cast<ElementT *>(bSeekToBegin ? std::begin(xm_aaStorage) : std::end(xm_aaStorage));
			xm_pEnd = xm_pBegin;
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
			return (std::size_t)(GetEnd() - GetBegin());
		}

		bool IsEmpty() const noexcept {
			return GetEnd() == GetBegin();
		}
		bool IsPushable() const noexcept {
			return GetEnd() != reinterpret_cast<const ElementT *>(std::end(xm_aaStorage));
		}
		bool IsUnshiftable() const noexcept {
			return GetBegin() != reinterpret_cast<const ElementT *>(std::begin(xm_aaStorage));
		}

		ElementT *UncheckedPush()
			noexcept(std::is_nothrow_constructible<ElementT>::value)
		{
			ASSERT_MSG(IsPushable(), L"容器已满。");

			if(std::is_trivial<ElementT>::value){
#ifndef NDEBUG
				__builtin_memset(xm_pEnd, 0xCC, sizeof(ElementT));
#endif
			} else {
				Construct(xm_pEnd);
			}
			return xm_pEnd++;
		}
		template<typename ...ParamsT>
		ElementT *UncheckedPush(ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		{
			ASSERT_MSG(IsPushable(), L"容器已满。");

			Construct(xm_pEnd, std::forward<ParamsT>(vParams)...);
			return xm_pEnd++;
		}
		void UncheckedPop() noexcept {
			ASSERT(!IsEmpty());

			Destruct(--xm_pEnd);
		}

		ElementT *UncheckedUnshift()
			noexcept(std::is_nothrow_constructible<ElementT>::value)
		{
			ASSERT_MSG(IsUnshiftable(), L"容器已满。");

			if(std::is_trivial<ElementT>::value){
#ifndef NDEBUG
				__builtin_memset(xm_pBegin - 1, 0xCC, sizeof(ElementT));
#endif
			} else {
				Construct(xm_pBegin - 1);
			}
			return --xm_pBegin;
		}
		template<typename ...ParamsT>
		ElementT *UncheckedUnshift(ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
		{
			ASSERT_MSG(IsUnshiftable(), L"容器已满。");

			Construct(xm_pBegin - 1, std::forward<ParamsT>(vParams)...);
			return --xm_pBegin;
		}
		void UncheckedShift() noexcept {
			ASSERT(!IsEmpty());

			Destruct(xm_pBegin++);
		}
	};

private:
	List<xChunk> xm_vList;

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
		rhs.Traverse([this](auto pBegin, auto uCount){ AppendCopy(pBegin, uCount); });
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
	const ElementT &GetFront() const noexcept {
		const auto pNode = xm_vList.GetFirst();
		ASSERT(pNode && !pNode->Get().IsEmpty());
		return pNode->Get().GetBegin()[0];
	}
	ElementT &GetFront() noexcept {
		const auto pNode = xm_vList.GetFirst();
		ASSERT(pNode && !pNode->Get().IsEmpty());
		return pNode->Get().GetBegin()[0];
	}
	const ElementT &GetBack() const noexcept {
		const auto pNode = xm_vList.GetLast();
		ASSERT(pNode && !pNode->Get().IsEmpty());
		return pNode->Get().GetEnd()[-1];
	}
	ElementT &GetBack() noexcept {
		const auto pNode = xm_vList.GetLast();
		ASSERT(pNode && !pNode->Get().IsEmpty());
		return pNode->Get().GetEnd()[-1];
	}

	template<typename CallbackT>
	void Traverse(CallbackT &&vCallback) const {
		for(auto pNode = xm_vList.GetFirst(); pNode; pNode = pNode->GetNext()){
			std::forward<CallbackT>(vCallback)(pNode->Get().GetBegin(), pNode->Get().GetSize());
		}
	}
	template<typename CallbackT>
	void Traverse(CallbackT &&vCallback){
		for(auto pNode = xm_vList.GetFirst(); pNode; pNode = pNode->GetNext()){
			std::forward<CallbackT>(vCallback)(pNode->Get().GetBegin(), pNode->Get().GetSize());
		}
	}

	bool IsEmpty() const noexcept {
		return xm_vList.IsEmpty();
	}
	void Clear() noexcept {
		xm_vList.Clear();
	}

	void Splice(Deque &clsSource) noexcept {
		xm_vList.Splice(nullptr, clsSource.xm_vList);
		ASSERT(clsSource.IsEmpty());
	}
	void Splice(Deque &&clsSource) noexcept {
		xm_vList.Splice(nullptr, std::move(clsSource.xm_vList));
		ASSERT(clsSource.IsEmpty());
	}

	template<typename ...ParamsT>
	ElementT *Push(ParamsT &&...vParams){
		ElementT *pRet;
		auto pNode = xm_vList.GetLast();
		if(pNode && pNode->Get().IsPushable()){
			pRet = pNode->Get().UncheckedPush(std::forward<ParamsT>(vParams)...);
		}
		pNode = xm_vList.Push(true);
		try {
			pRet = pNode->Get().UncheckedPush(std::forward<ParamsT>(vParams)...);
		} catch(...){
			xm_vList.Pop();
			throw;
		}
		return pRet;
	}
	void Pop() noexcept {
		const auto pNode = xm_vList.GetFirst();
		ASSERT(pNode && !pNode->Get().IsEmpty());
		pNode->Get().UncheckedPop();
		if(pNode->Get().IsEmpty()){
			xm_vList.Pop();
		}
	}

	template<typename ...ParamsT>
	ElementT *Unshift(ParamsT &&...vParams){
		ElementT *pRet;
		auto pNode = xm_vList.GetFirst();
		if(pNode && pNode->Get().IsUnshiftable()){
			pRet = pNode->Get().UncheckedUnshift(std::forward<ParamsT>(vParams)...);
		} else {
			pNode = xm_vList.Unshift(false);
			try {
				pRet = pNode->Get().UncheckedUnshift(std::forward<ParamsT>(vParams)...);
			} catch(...){
				xm_vList.Shift();
				throw;
			}
		}
		return pRet;
	}
	void Shift() noexcept {
		const auto pNode = xm_vList.GetLast();
		ASSERT(pNode && !pNode->Get().IsEmpty());
		pNode->Get().UncheckedShift();
		if(pNode->Get().IsEmpty()){
			xm_vList.Shift();
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
		xm_vList.Swap(rhs.xm_vList);
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

}

#endif
