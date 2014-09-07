// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_V_VECTOR_HPP_
#define MCF_V_VECTOR_HPP_

#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/MinMax.hpp"
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <memory>
#include <cstddef>

namespace MCF {

template<class Element, std::size_t ALT_STOR_THRESHOLD = 256u / sizeof(Element)>
class VVector {
	template<class, std::size_t>
	friend class VVector;

private:
	Element *xm_pBegin;
	Element *xm_pEnd;

	union {
		alignas(std::max_align_t)
			unsigned char xm_aSmall[sizeof(Element) * ALT_STOR_THRESHOLD];
		Element *xm_pEndOfStor;
	};

public:
	constexpr VVector() noexcept
		: xm_pBegin	((Element *)std::begin(xm_aSmall))
		, xm_pEnd	(xm_pBegin)
	{
	}
	template<typename ...Params>
	explicit VVector(std::size_t uCount, const Params &...vParams)
		: VVector()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class Iterator>
	VVector(Iterator itBegin, std::common_type_t<Iterator> itEnd)
		: VVector()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class Iterator>
	VVector(Iterator itBegin, std::size_t uCount)
		: VVector()
	{
		CopyToEnd(itBegin, uCount);
	}
	VVector(std::initializer_list<Element> rhs)
		: VVector()
	{
		CopyToEnd(rhs.begin(), rhs.size());
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector(const VVector<Element, OTHER_THRESHOLD> &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetFirst(), rhs.GetEnd());
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector(VVector<Element, OTHER_THRESHOLD> &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element>::value &&
			(ALT_STOR_THRESHOLD >= OTHER_THRESHOLD))
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector(const VVector &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetFirst(), rhs.GetEnd());
	}
	VVector(VVector &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element>::value)
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector &operator=(std::initializer_list<Element> rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector &operator=(const VVector<Element, OTHER_THRESHOLD> &rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector &operator=(VVector<Element, OTHER_THRESHOLD> &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element>::value &&
			(ALT_STOR_THRESHOLD >= OTHER_THRESHOLD))
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
		noexcept(std::is_nothrow_move_constructible<Element>::value)
	{
		rhs.Swap(*this);
		return *this;
	}
	~VVector() noexcept {
		Clear(true);
	}

private:
	template<std::size_t OTHER_THRESHOLD>
	void xMoveFrom(VVector<Element, OTHER_THRESHOLD> &rhs)
		noexcept(std::is_nothrow_move_constructible<Element>::value &&
			(ALT_STOR_THRESHOLD >= OTHER_THRESHOLD))
	{
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		if(rhs.xm_pBegin != (Element *)std::begin(rhs.xm_aSmall)){
			if(xm_pBegin != (Element *)std::begin(xm_aSmall)){
				::operator delete[](xm_pBegin);
			}
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= (Element *)std::begin(rhs.xm_aSmall);
			rhs.xm_pEnd		= rhs.xm_pBegin;
		} else {
			auto pRead = rhs.GetFirst();
			const auto uSize = rhs.GetSize();
			if(ALT_STOR_THRESHOLD < OTHER_THRESHOLD){
				Reserve(uSize);
			}
			for(auto i = uSize; i; --i){
				PushNoCheck(std::move_if_noexcept(*pRead));
				++pRead;
			}
		}
	}
	void xMoveFrom(VVector<Element, 0> &rhs) noexcept {
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		if(xm_pBegin != (Element *)std::begin(xm_aSmall)){
			::operator delete[](xm_pBegin);
		}
		xm_pBegin		= rhs.xm_pBegin;
		xm_pEnd			= rhs.xm_pEnd;
		xm_pEndOfStor	= rhs.xm_pEndOfStor;

		rhs.xm_pBegin	= nullptr;
		rhs.xm_pEnd		= nullptr;
	}

public:
	const Element *GetFirst() const noexcept {
		return xm_pBegin;
	}
	Element *GetFirst() noexcept {
		return xm_pBegin;
	}
	const Element *GetCBegin() const noexcept {
		return GetFirst();
	}
	const Element *GetEnd() const noexcept {
		return xm_pEnd;
	}
	Element *GetEnd() noexcept {
		return xm_pEnd;
	}
	const Element *GetCEnd() const noexcept {
		return GetEnd();
	}

	const Element *GetData() const noexcept {
		return GetFirst();
	}
	Element *GetData() noexcept {
		return GetFirst();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetFirst());
	}
	template<typename ...Params>
	Element *Resize(std::size_t uNewSize, const Params &...vParams){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			FillAtEnd(uNewSize - uOldSize, vParams...);
		} else if(uNewSize < uOldSize){
			TruncateFromEnd(uOldSize - uNewSize);
		}
		return GetData();
	}
	template<typename ...Params>
	Element *ResizeMore(std::size_t uDeltaSize, const Params &...vParams){
		const auto uOldSize = GetSize();
		FillAtEnd(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetFirst();
	}
	void Clear(bool bDeallocateBuffer = false) noexcept {
		TruncateFromEnd(GetSize());

		if(bDeallocateBuffer && (xm_pBegin != (Element *)std::begin(xm_aSmall))){
			::operator delete[](xm_pBegin);
			xm_pBegin = (Element *)std::begin(xm_aSmall);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(xm_pBegin == (Element *)std::begin(xm_aSmall)){
			return sizeof(xm_aSmall) / sizeof(Element);
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

			const auto pOldBegin = GetFirst();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (Element *)::operator new[](sizeof(Element) * uSizeToAlloc);
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

			if(xm_pBegin != (Element *)std::begin(xm_aSmall)){
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

	Element *PushNoCheck()
		noexcept(std::is_nothrow_constructible<Element>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		if(std::is_pod<Element>::value){
#ifndef NDEBUG
			__builtin_memset(xm_pEnd, 0xCC, sizeof(Element));
#endif
		} else {
			Construct(xm_pEnd);
		}
		return xm_pEnd++;
	}
	template<typename ...Params>
	Element *PushNoCheck(Params &&...vParams)
		noexcept(std::is_nothrow_constructible<Element, Params &&...>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		Construct(xm_pEnd, std::forward<Params>(vParams)...);
		return xm_pEnd++;
	}
	template<typename ...Params>
	Element *Push(Params &&...vParams){
		Reserve(GetSize() + 1);
		return PushNoCheck(std::forward<Params>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		Destruct(--xm_pEnd);
	}

	template<typename ...Params>
	void FillAtEndNoCheck(std::size_t uCount, const Params &...vParams)
		noexcept(std::is_nothrow_constructible<Element, const Params &...>::value)
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(vParams...);
		}
	}
	template<class Iterator>
	void CopyToEndNoCheck(Iterator itBegin, std::common_type_t<Iterator> itEnd)
		noexcept(noexcept(PushNoCheck(*std::declval<Iterator>())))
	{
		while(itBegin != itEnd){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<class Iterator>
	void CopyToEndNoCheck(Iterator itBegin, std::size_t uCount)
		noexcept(noexcept(PushNoCheck(*std::declval<Iterator>())))
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<typename ...Params>
	void FillAtEnd(std::size_t uCount, const Params &...vParams){
		Reserve(GetSize() + uCount);
		FillAtEndNoCheck(uCount, vParams...);
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
		noexcept(std::is_nothrow_move_constructible<Element>::value)
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
	explicit operator const Element *() const noexcept {
		return GetData();
	}
	explicit operator Element *() noexcept {
		return GetData();
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
	Element &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
};

template<class Element>
class VVector<Element, 0> {
	template<class, std::size_t>
	friend class VVector;

private:
	Element *xm_pBegin;
	Element *xm_pEnd;
	Element *xm_pEndOfStor;

public:
	constexpr VVector() noexcept
		: xm_pBegin		(nullptr)
		, xm_pEnd		(xm_pBegin)
		, xm_pEndOfStor	(xm_pBegin)
	{
	}
	template<typename ...Params>
	explicit VVector(std::size_t uCount, const Params &...vParams)
		: VVector()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class Iterator>
	VVector(Iterator itBegin, std::common_type_t<Iterator> itEnd)
		: VVector()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class Iterator>
	VVector(Iterator itBegin, std::size_t uCount)
		: VVector()
	{
		CopyToEnd(itBegin, uCount);
	}
	VVector(std::initializer_list<Element> rhs)
		: VVector()
	{
		CopyToEnd(rhs.begin(), rhs.size());
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector(const VVector<Element, OTHER_THRESHOLD> &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetFirst(), rhs.GetEnd());
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector(VVector<Element, OTHER_THRESHOLD> &&rhs)
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector(const VVector &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetFirst(), rhs.GetEnd());
	}
	VVector(VVector &&rhs) noexcept
		: VVector()
	{
		xMoveFrom(rhs);
	}
	VVector &operator=(std::initializer_list<Element> rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector &operator=(const VVector<Element, OTHER_THRESHOLD> &rhs){
		VVector(rhs).Swap(*this);
		return *this;
	}
	template<std::size_t OTHER_THRESHOLD>
	VVector &operator=(VVector<Element, OTHER_THRESHOLD> &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element>::value &&
			(0 >= OTHER_THRESHOLD))
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
	~VVector() noexcept {
		Clear(true);
	}

private:
	template<std::size_t OTHER_THRESHOLD>
	void xMoveFrom(VVector<Element, OTHER_THRESHOLD> &rhs)
		noexcept(std::is_nothrow_move_constructible<Element>::value &&
			(0 >= OTHER_THRESHOLD))
	{
		ASSERT(IsEmpty());
		ASSERT((void *)this != (void *)&rhs);

		if(rhs.xm_pBegin != (Element *)std::begin(rhs.xm_aSmall)){
			::operator delete[](xm_pBegin);
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= (Element *)std::begin(rhs.xm_aSmall);
			rhs.xm_pEnd		= rhs.xm_pBegin;
		} else {
			auto pRead = rhs.GetFirst();
			const auto uSize = rhs.GetSize();
			if(0 < OTHER_THRESHOLD){
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
	const Element *GetFirst() const noexcept {
		return xm_pBegin;
	}
	Element *GetFirst() noexcept {
		return xm_pBegin;
	}
	const Element *GetCBegin() const noexcept {
		return GetFirst();
	}
	const Element *GetEnd() const noexcept {
		return xm_pEnd;
	}
	Element *GetEnd() noexcept {
		return xm_pEnd;
	}
	const Element *GetCEnd() const noexcept {
		return GetEnd();
	}

	const Element *GetData() const noexcept {
		return GetFirst();
	}
	Element *GetData() noexcept {
		return GetFirst();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetFirst());
	}
	template<typename ...Params>
	void Resize(std::size_t uNewSize, const Params &...vParams){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			FillAtEnd(uNewSize - uOldSize, vParams...);
		} else if(uNewSize < uOldSize){
			TruncateFromEnd(uOldSize - uNewSize);
		}
	}
	template<typename ...Params>
	Element *ResizeMore(std::size_t uDeltaSize, const Params &...vParams){
		const auto uOldSize = GetSize();
		FillAtEnd(uDeltaSize, vParams...);
		return GetData() + uOldSize;
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetFirst();
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

			const auto pOldBegin = GetFirst();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (Element *)::operator new[](sizeof(Element) * uSizeToAlloc);
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

	Element *PushNoCheck()
		noexcept(std::is_nothrow_constructible<Element>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		if(std::is_pod<Element>::value){
#ifndef NDEBUG
			__builtin_memset(xm_pEnd, 0xCC, sizeof(Element));
#endif
		} else {
			Construct(xm_pEnd);
		}
		return xm_pEnd++;
	}
	template<typename ...Params>
	Element *PushNoCheck(Params &&...vParams)
		noexcept(std::is_nothrow_constructible<Element, Params &&...>::value)
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		Construct(xm_pEnd, std::forward<Params>(vParams)...);
		return xm_pEnd++;
	}
	template<typename ...Params>
	Element *Push(Params &&...vParams){
		Reserve(GetSize() + 1);
		return PushNoCheck(std::forward<Params>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		Destruct(--xm_pEnd);
	}

	template<typename ...Params>
	void FillAtEndNoCheck(std::size_t uCount, const Params &...vParams)
		noexcept(std::is_nothrow_constructible<Element, const Params &...>::value)
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(vParams...);
		}
	}
	template<class Iterator>
	void CopyToEndNoCheck(Iterator itBegin, std::common_type_t<Iterator> itEnd)
		noexcept(std::is_nothrow_constructible<Element,
			decltype(*std::declval<Iterator>())>::value)
	{
		while(itBegin != itEnd){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<class Iterator>
	void CopyToEndNoCheck(Iterator itBegin, std::size_t uCount)
		noexcept(std::is_nothrow_constructible<Element,
			decltype(*std::declval<Iterator>())>::value)
	{
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*itBegin);
			++itBegin;
		}
	}
	template<typename ...Params>
	void FillAtEnd(std::size_t uCount, const Params &...vParams){
		Reserve(GetSize() + uCount);
		FillAtEndNoCheck(uCount, vParams...);
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
	typedef Element value_type;

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Push(std::forward<Param>(vParam));
	}

public:
	explicit operator const Element *() const noexcept {
		return GetData();
	}
	explicit operator Element *() noexcept {
		return GetData();
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
	Element &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
};

template<typename Element, std::size_t ALT_STOR_THRESHOLD>
const Element *begin(const VVector<Element, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetFirst();
}
template<typename Element, std::size_t ALT_STOR_THRESHOLD>
Element *begin(VVector<Element, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetFirst();
}
template<typename Element, std::size_t ALT_STOR_THRESHOLD>
const Element *cbegin(const VVector<Element, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetCBegin();
}

template<typename Element, std::size_t ALT_STOR_THRESHOLD>
const Element *end(const VVector<Element, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element, std::size_t ALT_STOR_THRESHOLD>
Element *end(VVector<Element, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element, std::size_t ALT_STOR_THRESHOLD>
const Element *cend(const VVector<Element, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetCEnd();
}

template<typename Element>
using Vector = VVector<Element, 0>;

template<class Element, std::size_t ALT_STOR_THRESHOLD>
void swap(VVector<Element, ALT_STOR_THRESHOLD> &lhs,
	VVector<Element, ALT_STOR_THRESHOLD> &rhs)
	noexcept(noexcept(lhs.Swap(rhs)))
{
	lhs.Swap(rhs);
}

template<class Element>
void swap(Vector<Element> &lhs, Vector<Element> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
