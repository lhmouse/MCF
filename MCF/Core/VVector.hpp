// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_V_VECTOR_HPP_
#define MCF_V_VECTOR_HPP_

#include "../../MCFCRT/stdc/ext/assert.h"
#include "Utilities.hpp"
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

// C++11 workaround.
template<typename Element_t, bool ASSIGNABLE = std::is_trivial<Element_t>::value && std::is_copy_assignable<Element_t>::value>
struct Constructor_;

template<typename Element_t>
struct Constructor_<Element_t, true> {
	template<typename... Params_t>
	inline void operator()(Element_t *pRaw, Params_t &&... vParams) const noexcept {
		ASSERT_NOEXCEPT_BEGIN

		*pRaw = Element_t(std::forward<Params_t>(vParams)...);

		ASSERT_NOEXCEPT_END
	}
};

template<typename Element_t>
struct Constructor_<Element_t, false> {
	template<typename... Params_t>
	inline void operator()(Element_t *pRaw, Params_t &&... vParams) const
		noexcept(noexcept(Element_t(NullRef<Params_t>()...)))
	{
		ASSERT_NOEXCEPT_BEGIN

		new(pRaw) Element_t(std::forward<Params_t>(vParams)...);

		ASSERT_NOEXCEPT_END_COND(noexcept(Element_t(NullRef<Params_t>()...)))
	}
};
// - end workaround.
// Added on 2014-04-29.

template<class Element_t, std::size_t ALT_STOR_THRESHOLD = 0x400u / sizeof(Element_t)>
class VVector {
	template<class OtherElement_t, std::size_t OTHER_ALT_STOR_THRESHOLD>
	friend class VVector;

	static_assert(noexcept(NullRef<Element_t>().~Element_t()), "Element_t must be noexcept destructible.");

private:
	Element_t *xm_pBegin;
	Element_t *xm_pEnd;

	union {
		unsigned char xm_aSmall[sizeof(Element_t) * ALT_STOR_THRESHOLD];
		Element_t *xm_pEndOfStor;
	};

public:
	constexpr VVector() noexcept
		: xm_pBegin	((Element_t *)std::begin(xm_aSmall))
		, xm_pEnd	(xm_pBegin)
	{
	}
	template<typename... Params_t>
	explicit VVector(std::size_t uCount, const Params_t &...vParams)
		: VVector()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class Iterator_t>
	VVector(Iterator_t itBegin, Iterator_t itEnd)
		: VVector()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class Iterator_t>
	VVector(Iterator_t itBegin, std::size_t uCount)
		: VVector()
	{
		CopyToEnd(itBegin, uCount);
	}
	VVector(std::initializer_list<Element_t> vInitList)
		: VVector()
	{
		MoveToEnd(vInitList.begin(), vInitList.end());
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector(const VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector(VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element_t>::value && (ALT_STOR_THRESHOLD >= OTHER_ALT_STOR_THRESHOLD))
		: VVector()
	{
		operator=<OTHER_ALT_STOR_THRESHOLD>(std::move(rhs));
	}
	VVector(const VVector &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	VVector(VVector &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element_t>::value)
		: VVector()
	{
		operator=<ALT_STOR_THRESHOLD>(std::move(rhs));
	}
	VVector &operator=(std::initializer_list<Element_t> vInitList){
		Clear();
		MoveToEnd(vInitList.begin(), vInitList.end());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector &operator=(const VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &rhs){
		Clear();
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector &operator=(VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element_t>::value && (ALT_STOR_THRESHOLD >= OTHER_ALT_STOR_THRESHOLD))
	{
		Clear();

		if(rhs.xm_pBegin != (Element_t *)std::begin(rhs.xm_aSmall)){
			if(xm_pBegin != (Element_t *)std::begin(xm_aSmall)){
				::operator delete(xm_pBegin);
			}
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= (Element_t *)std::begin(rhs.xm_aSmall);
			rhs.xm_pEnd		= rhs.xm_pBegin;
		} else {
			auto pRead = rhs.GetBegin();
			const auto uSize = rhs.GetSize();
			Reserve(uSize);
			try {
				for(auto i = uSize; i; --i){
					PushNoCheck(std::move_if_noexcept(*pRead));
					++pRead;
				}
			} catch(...){
				Clear();
				throw;
			}
		}
		return *this;
	}
	VVector &operator=(VVector<Element_t, 0> &&rhs) noexcept {
		Clear();

		if(xm_pBegin != (Element_t *)std::begin(xm_aSmall)){
			::operator delete(xm_pBegin);
		}
		xm_pBegin		= rhs.xm_pBegin;
		xm_pEnd			= rhs.xm_pEnd;
		xm_pEndOfStor	= rhs.xm_pEndOfStor;

		rhs.xm_pBegin	= nullptr;
		rhs.xm_pEnd		= nullptr;

		return *this;
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			operator=<ALT_STOR_THRESHOLD>(rhs);
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs)
		noexcept(std::is_nothrow_move_constructible<Element_t>::value)
	{
		if(&rhs != this){
			operator=<ALT_STOR_THRESHOLD>(std::move(rhs));
		}
		return *this;
	}
	~VVector() noexcept {
		Clear();

		if(xm_pBegin != (Element_t *)std::begin(xm_aSmall)){
			::operator delete(xm_pBegin);
		}
	}

public:
	const Element_t *GetBegin() const noexcept {
		return xm_pBegin;
	}
	Element_t *GetBegin() noexcept {
		return xm_pBegin;
	}
	const Element_t *GetCBegin() const noexcept {
		return GetBegin();
	}
	const Element_t *GetEnd() const noexcept {
		return xm_pEnd;
	}
	Element_t *GetEnd() noexcept {
		return xm_pEnd;
	}
	const Element_t *GetCEnd() const noexcept {
		return GetEnd();
	}

	const Element_t *GetData() const noexcept {
		return GetBegin();
	}
	Element_t *GetData() noexcept {
		return GetBegin();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	template<typename... Params_t>
	void Resize(std::size_t uNewSize, const Params_t &... vParams){
		const std::size_t uCurrentSize = GetSize();
		if(uNewSize > uCurrentSize){
			FillAtEnd(uNewSize - uCurrentSize, vParams...);
		} else if(uNewSize < uCurrentSize){
			TruncateFromEnd(uCurrentSize - uNewSize);
		}
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetBegin();
	}
	void Clear() noexcept {
		TruncateFromEnd(GetSize());
	}

	std::size_t GetCapacity() const noexcept {
		if(xm_pBegin == (Element_t *)std::begin(xm_aSmall)){
			return sizeof(xm_aSmall) / sizeof(Element_t);
		}
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uCapacity){
		if(uCapacity > GetCapacity()){
			auto uSizeToAlloc = uCapacity;
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & (std::size_t)-0x10;

			const auto pOldBegin = GetBegin();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (Element_t *)::operator new(sizeof(Element_t) * uSizeToAlloc);
			auto pRead = pOldBegin;
			auto pWrite = pNewBegin;
			try {
				while(pRead != pOldEnd){
					Constructor_<Element_t>()(pWrite, std::move_if_noexcept(*pRead));
					++pWrite;
					++pRead;
				}
			} catch(...){
				while(pWrite != pNewBegin){
					--pWrite;
					pWrite->~Element_t();
				}
				::operator delete(pNewBegin);
				throw;
			}
			while(pRead != pOldBegin){
				--pRead;
				pRead->~Element_t();
			}

			if(xm_pBegin != (Element_t *)std::begin(xm_aSmall)){
				::operator delete(xm_pBegin);
			}
			xm_pBegin		= pNewBegin;
			xm_pEnd			= pWrite;
			xm_pEndOfStor	= xm_pBegin + uSizeToAlloc;
		}
	}

	template<typename... Params_t>
	Element_t &PushNoCheck(Params_t &&...vParams)
		noexcept(noexcept(Constructor_<Element_t>()(nullptr, NullRef<Params_t>()...)))
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		Constructor_<Element_t>()(xm_pEnd, std::forward<Params_t>(vParams)...);
		return *(xm_pEnd++);
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(!IsEmpty(), L"容器为空。");

		(--xm_pEnd)->~Element_t();
	}

	template<typename... Params_t>
	Element_t &Push(Params_t &&...vParams){
		Reserve(GetSize() + 1);
		return PushNoCheck(std::forward<Params_t>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		PopNoCheck();
	}

	template<typename... Params_t>
	void FillAtEnd(std::size_t uCount, const Params_t &...vParams){
		Reserve(GetSize() + uCount);
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(vParams...);
		}
	}
	template<class Iterator_t>
	void CopyToEnd(Iterator_t itBegin, Iterator_t itEnd){
		CopyToEnd(std::move(itBegin), (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator_t>
	void CopyToEnd(Iterator_t itBegin, std::size_t uCount){
		Reserve(GetSize() + uCount);
		auto itCur = itBegin;
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*itCur);
			++itCur;
		}
	}
	template<class Iterator_t>
	void MoveToEnd(Iterator_t itBegin, Iterator_t itEnd){
		MoveToEnd(std::move(itBegin), (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator_t>
	void MoveToEnd(Iterator_t itBegin, std::size_t uCount){
		CopyToEnd(std::make_move_iterator(itBegin), uCount);
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			PopNoCheck();
		}
	}

	void Swap(VVector &rhs)
		noexcept(std::is_nothrow_move_constructible<Element_t>::value)
	{
		VVector vecTemp(std::move_if_noexcept(*this));
		*this = std::move_if_noexcept(rhs);
		rhs = std::move_if_noexcept(vecTemp);
	}

public:
	explicit operator const Element_t *() const noexcept {
		return GetData();
	}
	explicit operator Element_t *() noexcept {
		return GetData();
	}

	const Element_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
	Element_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
};

template<class Element_t>
class VVector<Element_t, 0> {
	template<class OtherElement_t, std::size_t OTHER_ALT_STOR_THRESHOLD>
	friend class VVector;

	static_assert(noexcept(delete (Element_t *)nullptr), "Element_t must be noexcept destructible.");

private:
	Element_t *xm_pBegin;
	Element_t *xm_pEnd;
	Element_t *xm_pEndOfStor;

public:
	constexpr VVector() noexcept
		: xm_pBegin		(nullptr)
		, xm_pEnd		(xm_pBegin)
		, xm_pEndOfStor	(xm_pBegin)
	{
	}
	template<typename... Params_t>
	explicit VVector(std::size_t uCount, const Params_t &...vParams)
		: VVector()
	{
		FillAtEnd(uCount, vParams...);
	}
	template<class Iterator_t>
	VVector(Iterator_t itBegin, Iterator_t itEnd)
		: VVector()
	{
		CopyToEnd(itBegin, itEnd);
	}
	template<class Iterator_t>
	VVector(Iterator_t itBegin, std::size_t uCount)
		: VVector()
	{
		CopyToEnd(itBegin, uCount);
	}
	VVector(std::initializer_list<Element_t> vInitList)
		: VVector()
	{
		MoveToEnd(vInitList.begin(), vInitList.end());
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector(const VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector(VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &&rhs)
		noexcept(OTHER_ALT_STOR_THRESHOLD == 0)
		: VVector()
	{
		operator=<OTHER_ALT_STOR_THRESHOLD>(std::move(rhs));
	}
	VVector(const VVector &rhs)
		: VVector()
	{
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	VVector(VVector &&rhs) noexcept
		: VVector()
	{
		operator=(std::move(rhs));
	}
	VVector &operator=(std::initializer_list<Element_t> vInitList){
		Clear();
		MoveToEnd(vInitList.begin(), vInitList.end());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector &operator=(const VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &rhs){
		Clear();
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THRESHOLD>
	VVector &operator=(VVector<Element_t, OTHER_ALT_STOR_THRESHOLD> &&rhs)
		noexcept(OTHER_ALT_STOR_THRESHOLD == 0)
	{
		Clear();

		if(rhs.xm_pBegin != (Element_t *)std::begin(rhs.xm_aSmall)){
			::operator delete(xm_pBegin);
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= (Element_t *)std::begin(rhs.xm_aSmall);
			rhs.xm_pEnd		= rhs.xm_pBegin;
		} else {
			auto pRead = rhs.GetBegin();
			const auto uSize = rhs.GetSize();
			Reserve(uSize);
			try {
				for(auto i = uSize; i; --i){
					PushNoCheck(std::move_if_noexcept(*pRead));
					++pRead;
				}
			} catch(...){
				Clear();
				throw;
			}
		}
		return *this;
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			operator=<0>(rhs);
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs) noexcept {
		if(&rhs != this){
			Clear();

			::operator delete(xm_pBegin);
			xm_pBegin		= rhs.xm_pBegin;
			xm_pEnd			= rhs.xm_pEnd;
			xm_pEndOfStor	= rhs.xm_pEndOfStor;

			rhs.xm_pBegin	= nullptr;
			rhs.xm_pEnd		= nullptr;
		}
		return *this;
	}
	~VVector() noexcept {
		Clear();

		::operator delete(xm_pBegin);
	}

public:
	const Element_t *GetBegin() const noexcept {
		return xm_pBegin;
	}
	Element_t *GetBegin() noexcept {
		return xm_pBegin;
	}
	const Element_t *GetCBegin() const noexcept {
		return GetBegin();
	}
	const Element_t *GetEnd() const noexcept {
		return xm_pEnd;
	}
	Element_t *GetEnd() noexcept {
		return xm_pEnd;
	}
	const Element_t *GetCEnd() const noexcept {
		return GetEnd();
	}

	const Element_t *GetData() const noexcept {
		return GetBegin();
	}
	Element_t *GetData() noexcept {
		return GetBegin();
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	template<typename... Params_t>
	void Resize(std::size_t uNewSize, const Params_t &... vParams){
		const std::size_t uCurrentSize = GetSize();
		if(uNewSize > uCurrentSize){
			FillAtEnd(uNewSize - uCurrentSize, vParams...);
		} else if(uNewSize < uCurrentSize){
			TruncateFromEnd(uCurrentSize - uNewSize);
		}
	}

	bool IsEmpty() const noexcept {
		return GetEnd() == GetBegin();
	}
	void Clear() noexcept {
		TruncateFromEnd(GetSize());
	}

	std::size_t GetCapacity() const noexcept {
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uCapacity){
		if(uCapacity > GetCapacity()){
			auto uSizeToAlloc = uCapacity;
			uSizeToAlloc += (uSizeToAlloc >> 1);
			uSizeToAlloc = (uSizeToAlloc + 0xF) & (std::size_t)-0x10;

			const auto pOldBegin = GetBegin();
			const auto pOldEnd = GetEnd();
			const auto pNewBegin = (Element_t *)::operator new(sizeof(Element_t) * uSizeToAlloc);
			auto pRead = pOldBegin;
			auto pWrite = pNewBegin;
			try {
				while(pRead != pOldEnd){
					Constructor_<Element_t>()(pWrite, std::move_if_noexcept(*pRead));
					++pWrite;
					++pRead;
				}
			} catch(...){
				while(pWrite != pNewBegin){
					--pWrite;
					pWrite->~Element_t();
				}
				::operator delete(pNewBegin);
				throw;
			}
			while(pRead != pOldBegin){
				--pRead;
				pRead->~Element_t();
			}

			::operator delete(xm_pBegin);
			xm_pBegin		= pNewBegin;
			xm_pEnd			= pWrite;
			xm_pEndOfStor	= xm_pBegin + uSizeToAlloc;
		}
	}

	template<typename... Params_t>
	Element_t &PushNoCheck(Params_t &&...vParams)
		noexcept(noexcept(Constructor_<Element_t>()(nullptr, NullRef<Params_t>()...)))
	{
		ASSERT_MSG(GetSize() < GetCapacity(), L"容器已满。");

		Constructor_<Element_t>()(xm_pEnd, std::forward<Params_t>(vParams)...);
		return *(xm_pEnd++);
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(!IsEmpty(), L"容器为空。");

		(--xm_pEnd)->~Element_t();
	}

	template<typename... Params_t>
	Element_t &Push(Params_t &&...vParams){
		Reserve(GetSize() + 1);
		return PushNoCheck(std::forward<Params_t>(vParams)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		PopNoCheck();
	}

	template<typename... Params_t>
	void FillAtEnd(std::size_t uCount, const Params_t &...vParams){
		Reserve(GetSize() + uCount);
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(vParams...);
		}
	}
	template<class Iterator_t>
	void CopyToEnd(Iterator_t itBegin, Iterator_t itEnd){
		CopyToEnd(std::move(itBegin), (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator_t>
	void CopyToEnd(Iterator_t itBegin, std::size_t uCount){
		Reserve(GetSize() + uCount);
		auto itCur = itBegin;
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*itCur);
			++itCur;
		}
	}
	template<class Iterator_t>
	void MoveToEnd(Iterator_t itBegin, Iterator_t itEnd){
		MoveToEnd(std::move(itBegin), (std::size_t)std::distance(itBegin, itEnd));
	}
	template<class Iterator_t>
	void MoveToEnd(Iterator_t itBegin, std::size_t uCount){
		CopyToEnd(std::make_move_iterator(itBegin), uCount);
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			PopNoCheck();
		}
	}

	void Swap(VVector &rhs) noexcept {
		std::swap(xm_pBegin,		rhs.xm_pBegin);
		std::swap(xm_pEnd,			rhs.xm_pEnd);
		std::swap(xm_pEndOfStor,	rhs.rxm_pEndOfStor);
	}

public:
	explicit operator const Element_t *() const noexcept {
		return GetData();
	}
	explicit operator Element_t *() noexcept {
		return GetData();
	}

	const Element_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
	Element_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData()[uIndex];
	}
};

template<typename Element_t, std::size_t ALT_STOR_THRESHOLD>
const Element_t *begin(const VVector<Element_t, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename Element_t, std::size_t ALT_STOR_THRESHOLD>
Element_t *begin(VVector<Element_t, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename Element_t, std::size_t ALT_STOR_THRESHOLD>
const Element_t *cbegin(const VVector<Element_t, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetCBegin();
}

template<typename Element_t, std::size_t ALT_STOR_THRESHOLD>
const Element_t *end(const VVector<Element_t, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element_t, std::size_t ALT_STOR_THRESHOLD>
Element_t *end(VVector<Element_t, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element_t, std::size_t ALT_STOR_THRESHOLD>
const Element_t *cend(const VVector<Element_t, ALT_STOR_THRESHOLD> &vec) noexcept {
	return vec.GetCEnd();
}

template<class Element_t>
using Vector = VVector<Element_t, 0>;

}

#endif
