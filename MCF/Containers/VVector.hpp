// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_V_VECTOR_HPP_
#define MCF_CONTAINERS_V_VECTOR_HPP_

#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/MinMax.hpp"
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <memory>
#include <cstddef>

namespace MCF {

template<class ElementT, std::size_t ALT_STOR_THRESHOLD_T = 256u / sizeof(ElementT)>
class VVector {
	template<class, std::size_t>
	friend class VVector;

private:
	ElementT *xm_pBegin;
	ElementT *xm_pEnd;

	union {
		alignas(std::max_align_t) unsigned char xm_aSmall[sizeof(ElementT) * ALT_STOR_THRESHOLD_T];
		ElementT *xm_pEndOfStor;
	};

public:
	constexpr VVector() noexcept
		: xm_pBegin	((ElementT *)std::begin(xm_aSmall))
		, xm_pEnd	(xm_pBegin)
	{
	}
	template<typename ...ParamsT>
	explicit VVector(std::size_t uCount, const ParamsT &...vParams)
		: VVector()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class IteratorT>
	VVector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: VVector()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class IteratorT>
	VVector(IteratorT itBegin, std::size_t uCount)
		: VVector()
	{
		CopyToEnd(itBegin, uCount);
	}
	VVector(std::initializer_list<ElementT> rhs)
		: VVector()
	{
		CopyToEnd(rhs.begin(), rhs.size());
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector(const VVector<ElementT, SRC_THRESHOLD_T> &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector(VVector<ElementT, SRC_THRESHOLD_T> &&rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value &&
			(ALT_STOR_THRESHOLD_T >= SRC_THRESHOLD_T))
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector(const VVector &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	VVector(VVector &&rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value)
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector &operator=(std::initializer_list<ElementT> rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector &operator=(const VVector<ElementT, SRC_THRESHOLD_T> &rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector &operator=(VVector<ElementT, SRC_THRESHOLD_T> &&rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value &&
			(ALT_STOR_THRESHOLD_T >= SRC_THRESHOLD_T))
	{
		VVector(std::move(rhs)).Swap(*this);
		return *this;
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			VVector(rhs).Swap(*this);
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value)
	{
		rhs.Swap(*this);
		return *this;
	}
	~VVector(){
		Clear(true);
	}

private:
	template<std::size_t SRC_THRESHOLD_T>
	void xMoveFrom(VVector<ElementT, SRC_THRESHOLD_T> &rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value &&
			(ALT_STOR_THRESHOLD_T >= SRC_THRESHOLD_T))
	{
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		if(rhs.xm_pBegin != (ElementT *)std::begin(rhs.xm_aSmall)){
			if(xm_pBegin != (ElementT *)std::begin(xm_aSmall)){
				::operator delete[](xm_pBegin);
			}
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= (ElementT *)std::begin(rhs.xm_aSmall);
			rhs.xm_pEnd		= rhs.xm_pBegin;
		} else {
			auto pRead = rhs.GetBegin();
			const auto uSize = rhs.GetSize();
			if(ALT_STOR_THRESHOLD_T < SRC_THRESHOLD_T){
				Reserve(uSize);
			}
			for(auto i = uSize; i; --i){
				PushNoCheck(std::move_if_noexcept(*pRead));
				++pRead;
			}
		}
	}
	void xMoveFrom(VVector<ElementT, 0> &rhs) noexcept {
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		if(xm_pBegin != (ElementT *)std::begin(xm_aSmall)){
			::operator delete[](xm_pBegin);
		}
		xm_pBegin		= rhs.xm_pBegin;
		xm_pEnd			= rhs.xm_pEnd;
		xm_pEndOfStor	= rhs.xm_pEndOfStor;

		rhs.xm_pBegin	= nullptr;
		rhs.xm_pEnd		= nullptr;
	}

public:
	const ElementT *GetBegin() const noexcept {
		return xm_pBegin;
	}
	ElementT *GetBegin() noexcept {
		return xm_pBegin;
	}
	const ElementT *GetCBegin() const noexcept {
		return GetBegin();
	}
	const ElementT *GetEnd() const noexcept {
		return xm_pEnd;
	}
	ElementT *GetEnd() noexcept {
		return xm_pEnd;
	}
	const ElementT *GetCEnd() const noexcept {
		return GetEnd();
	}

	const ElementT *GetData() const noexcept {
		return GetBegin();
	}
	ElementT *GetData() noexcept {
		return GetBegin();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	template<typename ...ParamsT>
	ElementT *Resize(std::size_t uNewSize, const ParamsT &...vParams){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			FillAtEnd(uNewSize - uOldSize, vParams...);
		} else if(uNewSize < uOldSize){
			TruncateFromEnd(uOldSize - uNewSize);
		}
		return GetData();
	}
	template<typename ...ParamsT>
	ElementT *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		FillAtEnd(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetBegin();
	}
	void Clear(bool bDeallocateBuffer = false) noexcept {
		TruncateFromEnd(GetSize());

		if(bDeallocateBuffer && (xm_pBegin != (ElementT *)std::begin(xm_aSmall))){
			::operator delete[](xm_pBegin);
			xm_pBegin = (ElementT *)std::begin(xm_aSmall);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(xm_pBegin == (ElementT *)std::begin(xm_aSmall)){
			return sizeof(xm_aSmall) / sizeof(ElementT);
		}
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity > uOldCapacity){
			auto uSizeToAlloc = uOldCapacity + 1;
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & (std::size_t)-0x10;
			uSizeToAlloc = Max(uSizeToAlloc, uNewCapacity);

			const auto pOldBegin = GetBegin();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (ElementT *)::operator new[](sizeof(ElementT) * uSizeToAlloc);
			auto pRead = pOldBegin;
			auto pWrite = pNewBegin;
			try {
				while(pRead != pOldEnd){
					Construct(pWrite, std::move_if_noexcept(*pRead));
					++pWrite;
					++pRead;
				}
			} catch(...){
				while(pWrite != pNewBegin){
					--pWrite;
					Destruct(pWrite);
				}
				::operator delete[](pNewBegin);
				throw;
			}
			while(pRead != pOldBegin){
				--pRead;
				Destruct(pRead);
			}

			if(xm_pBegin != (ElementT *)std::begin(xm_aSmall)){
				::operator delete[](xm_pBegin);
			}
			xm_pBegin		= pNewBegin;
			xm_pEnd			= pWrite;
			xm_pEndOfStor	= xm_pBegin + uSizeToAlloc;
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		Reserve(GetSize() + uDeltaCapacity);
	}

	ElementT *PushNoCheck()
		noexcept(std::is_nothrow_constructible<ElementT>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		if(std::is_pod<ElementT>::value){
#ifndef NDEBUG
			__builtin_memset(xm_pEnd, 0xCC, sizeof(ElementT));
#endif
		} else {
			Construct(xm_pEnd);
		}
		return xm_pEnd++;
	}
	template<typename ...ParamsT>
	ElementT *PushNoCheck(ParamsT &&...vParams)
		noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		Construct(xm_pEnd, std::forward<ParamsT>(vParams)...);
		return xm_pEnd++;
	}
	template<typename ...ParamsT>
	ElementT *Push(ParamsT &&...vParams){
		Reserve(GetSize() + 1);
		return PushNoCheck(std::forward<ParamsT>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		Destruct(--xm_pEnd);
	}

	template<typename ...ParamsT>
	void FillAtEndNoCheck(std::size_t uCount, const ParamsT &...vParams)
		noexcept(std::is_nothrow_constructible<ElementT, const ParamsT &...>::value)
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(vParams...);
		}
	}
	template<class IteratorT>
	void CopyToEndNoCheck(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		noexcept(noexcept(PushNoCheck(*std::declval<IteratorT>())))
	{
		while(itBegin != itEnd){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<class IteratorT>
	void CopyToEndNoCheck(IteratorT itBegin, std::size_t uCount)
		noexcept(noexcept(PushNoCheck(*std::declval<IteratorT>())))
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<typename ...ParamsT>
	void FillAtEnd(std::size_t uCount, const ParamsT &...vParams){
		Reserve(GetSize() + uCount);
		FillAtEndNoCheck(uCount, vParams...);
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
		Reserve(GetSize() + uCount);
		CopyToEndNoCheck(std::move(itBegin), uCount);
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			Pop();
		}
	}

	void Swap(VVector &rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value)
	{
		if(this != &rhs){
			VVector vecTemp;
			vecTemp.xMoveFrom(*this);
			Clear();
			xMoveFrom(rhs);
			rhs.Clear();
			rhs.xMoveFrom(vecTemp);
		}
	}

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Push(std::forward<Param>(vParam));
	}

public:
	explicit operator const ElementT *() const noexcept {
		return GetData();
	}
	explicit operator ElementT *() noexcept {
		return GetData();
	}

	const ElementT &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
	ElementT &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
};

template<class ElementT>
class VVector<ElementT, 0> {
	template<class, std::size_t>
	friend class VVector;

private:
	ElementT *xm_pBegin;
	ElementT *xm_pEnd;
	ElementT *xm_pEndOfStor;

public:
	constexpr VVector() noexcept
		: xm_pBegin		(nullptr)
		, xm_pEnd		(xm_pBegin)
		, xm_pEndOfStor	(xm_pBegin)
	{
	}
	template<typename ...ParamsT>
	explicit VVector(std::size_t uCount, const ParamsT &...vParams)
		: VVector()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class IteratorT>
	VVector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: VVector()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class IteratorT>
	VVector(IteratorT itBegin, std::size_t uCount)
		: VVector()
	{
		CopyToEnd(itBegin, uCount);
	}
	VVector(std::initializer_list<ElementT> rhs)
		: VVector()
	{
		CopyToEnd(rhs.begin(), rhs.size());
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector(const VVector<ElementT, SRC_THRESHOLD_T> &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector(VVector<ElementT, SRC_THRESHOLD_T> &&rhs)
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector(const VVector &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	VVector(VVector &&rhs) noexcept
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector &operator=(std::initializer_list<ElementT> rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector &operator=(const VVector<ElementT, SRC_THRESHOLD_T> &rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t SRC_THRESHOLD_T>
	VVector &operator=(VVector<ElementT, SRC_THRESHOLD_T> &&rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value &&
			(0 >= SRC_THRESHOLD_T))
	{
		VVector(std::move(rhs)).Swap(*this);
		return *this;
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			VVector(rhs).Swap(*this);
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs) noexcept {
		if(&rhs != this){
			rhs.Swap(*this);
		}
		return *this;
	}
	~VVector(){
		Clear(true);
	}

private:
	template<std::size_t SRC_THRESHOLD_T>
	void xMoveFrom(VVector<ElementT, SRC_THRESHOLD_T> &rhs)
		noexcept(std::is_nothrow_move_constructible<ElementT>::value &&
			(0 >= SRC_THRESHOLD_T))
	{
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		if(rhs.xm_pBegin != (ElementT *)std::begin(rhs.xm_aSmall)){
			::operator delete[](xm_pBegin);
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= (ElementT *)std::begin(rhs.xm_aSmall);
			rhs.xm_pEnd		= rhs.xm_pBegin;
		} else {
			auto pRead = rhs.GetBegin();
			const auto uSize = rhs.GetSize();
			if(0 < SRC_THRESHOLD_T){
				Reserve(uSize);
			}
			for(auto i = uSize; i; --i){
				PushNoCheck(std::move_if_noexcept(*pRead));
				++pRead;
			}
		}
	}
	void xMoveFrom(VVector &rhs) noexcept {
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		::operator delete[](xm_pBegin);
		xm_pBegin		= rhs.xm_pBegin;
		xm_pEnd			= rhs.xm_pEnd;
		xm_pEndOfStor	= rhs.xm_pEndOfStor;

		rhs.xm_pBegin	= nullptr;
		rhs.xm_pEnd		= nullptr;
	}

public:
	const ElementT *GetBegin() const noexcept {
		return xm_pBegin;
	}
	ElementT *GetBegin() noexcept {
		return xm_pBegin;
	}
	const ElementT *GetCBegin() const noexcept {
		return GetBegin();
	}
	const ElementT *GetEnd() const noexcept {
		return xm_pEnd;
	}
	ElementT *GetEnd() noexcept {
		return xm_pEnd;
	}
	const ElementT *GetCEnd() const noexcept {
		return GetEnd();
	}

	const ElementT *GetData() const noexcept {
		return GetBegin();
	}
	ElementT *GetData() noexcept {
		return GetBegin();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	template<typename ...ParamsT>
	void Resize(std::size_t uNewSize, const ParamsT &...vParams){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			FillAtEnd(uNewSize - uOldSize, vParams...);
		} else if(uNewSize < uOldSize){
			TruncateFromEnd(uOldSize - uNewSize);
		}
	}
	template<typename ...ParamsT>
	ElementT *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = GetSize();
		FillAtEnd(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetBegin();
	}
	void Clear(bool bDeallocateBuffer = false) noexcept {
		TruncateFromEnd(GetSize());

		if(bDeallocateBuffer){
			::operator delete[](xm_pBegin);
		}
	}

	std::size_t GetCapacity() const noexcept {
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity > uOldCapacity){
			auto uSizeToAlloc = uOldCapacity + 1;
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & (std::size_t)-0x10;
			uSizeToAlloc = Max(uSizeToAlloc, uNewCapacity);

			const auto pOldBegin = GetBegin();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (ElementT *)::operator new[](sizeof(ElementT) * uSizeToAlloc);
			auto pRead = pOldBegin;
			auto pWrite = pNewBegin;
			try {
				while(pRead != pOldEnd){
					Construct(pWrite, std::move_if_noexcept(*pRead));
					++pWrite;
					++pRead;
				}
			} catch(...){
				while(pWrite != pNewBegin){
					--pWrite;
					Destruct(pWrite);
				}
				::operator delete[](pNewBegin);
				throw;
			}
			while(pRead != pOldBegin){
				--pRead;
				Destruct(pRead);
			}

			::operator delete[](xm_pBegin);
			xm_pBegin		= pNewBegin;
			xm_pEnd			= pWrite;
			xm_pEndOfStor	= xm_pBegin + uSizeToAlloc;
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		Reserve(GetSize() + uDeltaCapacity);
	}

	ElementT *PushNoCheck()
		noexcept(std::is_nothrow_constructible<ElementT>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		if(std::is_pod<ElementT>::value){
#ifndef NDEBUG
			__builtin_memset(xm_pEnd, 0xCC, sizeof(ElementT));
#endif
		} else {
			Construct(xm_pEnd);
		}
		return xm_pEnd++;
	}
	template<typename ...ParamsT>
	ElementT *PushNoCheck(ParamsT &&...vParams)
		noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		Construct(xm_pEnd, std::forward<ParamsT>(vParams)...);
		return xm_pEnd++;
	}
	template<typename ...ParamsT>
	ElementT *Push(ParamsT &&...vParams){
		Reserve(GetSize() + 1);
		return PushNoCheck(std::forward<ParamsT>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		Destruct(--xm_pEnd);
	}

	template<typename ...ParamsT>
	void FillAtEndNoCheck(std::size_t uCount, const ParamsT &...vParams)
		noexcept(std::is_nothrow_constructible<ElementT, const ParamsT &...>::value)
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(vParams...);
		}
	}
	template<class IteratorT>
	void CopyToEndNoCheck(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		noexcept(std::is_nothrow_constructible<ElementT,
			decltype(*std::declval<IteratorT>())>::value)
	{
		while(itBegin != itEnd){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<class IteratorT>
	void CopyToEndNoCheck(IteratorT itBegin, std::size_t uCount)
		noexcept(std::is_nothrow_constructible<ElementT,
			decltype(*std::declval<IteratorT>())>::value)
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<typename ...ParamsT>
	void FillAtEnd(std::size_t uCount, const ParamsT &...vParams){
		Reserve(GetSize() + uCount);
		FillAtEndNoCheck(uCount, vParams...);
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
		Reserve(GetSize() + uCount);
		CopyToEndNoCheck(std::move(itBegin), uCount);
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			Pop();
		}
	}

	void Swap(VVector &rhs) noexcept {
		if(this != &rhs){
			std::swap(xm_pBegin,		rhs.xm_pBegin);
			std::swap(xm_pEnd,			rhs.xm_pEnd);
			std::swap(xm_pEndOfStor,	rhs.xm_pEndOfStor);
		}
	}

public:
	using value_type = ElementT;

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Push(std::forward<Param>(vParam));
	}

public:
	explicit operator const ElementT *() const noexcept {
		return GetData();
	}
	explicit operator ElementT *() noexcept {
		return GetData();
	}

	const ElementT &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
	ElementT &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
};

template<typename ElementT, std::size_t ALT_STOR_THRESHOLD_T>
const ElementT *begin(const VVector<ElementT, ALT_STOR_THRESHOLD_T> &vec) noexcept {
	return vec.GetBegin();
}
template<typename ElementT, std::size_t ALT_STOR_THRESHOLD_T>
ElementT *begin(VVector<ElementT, ALT_STOR_THRESHOLD_T> &vec) noexcept {
	return vec.GetBegin();
}
template<typename ElementT, std::size_t ALT_STOR_THRESHOLD_T>
const ElementT *cbegin(const VVector<ElementT, ALT_STOR_THRESHOLD_T> &vec) noexcept {
	return vec.GetCBegin();
}

template<typename ElementT, std::size_t ALT_STOR_THRESHOLD_T>
const ElementT *end(const VVector<ElementT, ALT_STOR_THRESHOLD_T> &vec) noexcept {
	return vec.GetEnd();
}
template<typename ElementT, std::size_t ALT_STOR_THRESHOLD_T>
ElementT *end(VVector<ElementT, ALT_STOR_THRESHOLD_T> &vec) noexcept {
	return vec.GetEnd();
}
template<typename ElementT, std::size_t ALT_STOR_THRESHOLD_T>
const ElementT *cend(const VVector<ElementT, ALT_STOR_THRESHOLD_T> &vec) noexcept {
	return vec.GetCEnd();
}

template<typename ElementT>
using Vector = VVector<ElementT, 0>;

template<class ElementT, std::size_t ALT_STOR_THRESHOLD_T>
void swap(VVector<ElementT, ALT_STOR_THRESHOLD_T> &lhs,
	VVector<ElementT, ALT_STOR_THRESHOLD_T> &rhs)
	noexcept(noexcept(lhs.Swap(rhs)))
{
	lhs.Swap(rhs);
}

template<class ElementT>
void swap(Vector<ElementT> &lhs, Vector<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
