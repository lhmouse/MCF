// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_VVECTOR_HPP_
#define MCF_CRT_VVECTOR_HPP_

#include "../../c/ext/assert.h"
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

template<class Element_t, std::size_t ALT_STOR_THLD = 0x400u / sizeof(Element_t)>
class VVector {
private:
	template<typename Test_t = int>
	static Element_t *xMoveArray(
		Element_t *pOut,
		Element_t *pBegin,
		Element_t *pEnd,
		typename std::enable_if<std::is_nothrow_move_constructible<Element_t>::value, Test_t>::type = 0
	) noexcept {
		const auto pNewEnd = std::move(pBegin, pEnd, pOut);
		auto pToDestroy = pEnd;
		while(pToDestroy != pBegin){
			(--pToDestroy)->~Element_t();
		}
		return pNewEnd;
	}
	template<typename Test_t = int>
	static Element_t *xMoveArray(
		Element_t *pOut,
		Element_t *pBegin,
		Element_t *pEnd,
		typename std::enable_if<!std::is_nothrow_move_constructible<Element_t>::value, Test_t>::type = 0
	){
		auto pWrite = pOut;
		auto pRead = pBegin;
		try {
			while(pRead != pEnd){
				new(pWrite) Element_t(*pRead);
				++pWrite;
				++pRead;
			}
		} catch(...){
			while(pWrite != pOut){
				(--pWrite)->~Element_t();
			}
			throw;
		}
		while(pRead != pBegin){
			(--pRead)->~Element_t();
		}
		return pWrite;
	}

private:
	unsigned char xm_aSmall[sizeof(Element_t) * ALT_STOR_THLD];
	std::unique_ptr<unsigned char []> xm_pLarge;

	Element_t *xm_pBegin;
	Element_t *xm_pEnd;
	const Element_t *xm_pEndOfStor;

public:
	constexpr VVector() noexcept
		: xm_pBegin		((Element_t *)std::begin(xm_aSmall))
		, xm_pEnd		((Element_t *)std::begin(xm_aSmall))
		, xm_pEndOfStor	((Element_t *)std::end(xm_aSmall))
	{
	}
	template<typename... Params_t>
	VVector(std::size_t uCount, const Params_t &...vParams) : VVector() {
		FillAtEnd(uCount, vParams...);
	}
	VVector(const Element_t *pFrom, const Element_t *pTo) : VVector() {
		CopyToEnd(pFrom, pTo);
	}
	VVector(const Element_t *pFrom, std::size_t uCount) : VVector() {
		CopyToEnd(pFrom, uCount);
	}
	VVector(std::initializer_list<Element_t> rhs) : VVector() {
		Reserve(rhs.size());
		for(auto it = rhs.begin(); it != rhs.end(); ++it){
			PushNoCheck(std::move(*it));
		}
	}
	VVector(const VVector &rhs) : VVector(rhs.GetBegin(), rhs.GetEnd()) {
	}
	VVector(VVector &&rhs) : VVector() {
		*this = std::move(rhs);
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector(const VVector<Element_t, OTHER_ALT_STOR_THLD> &rhs) : VVector(rhs.GetBegin(), rhs.GetEnd()) {
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector(VVector<Element_t, OTHER_ALT_STOR_THLD> &&rhs) : VVector() {
		MoveToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			Clear();
			CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs){
		if(&rhs != this){
			Clear();

			if(!xIsSmall()){
				xm_pLarge			= std::move(rhs.xm_pLarge);
				xm_pBegin			= rhs.xm_pBegin;
				xm_pEnd				= rhs.xm_pEnd;
				xm_pEndOfStor		= rhs.xm_pEndOfStor;

				rhs.xm_pBegin		= (Element_t *)std::begin(rhs.xm_aSmall);
				rhs.xm_pEnd			= (Element_t *)std::begin(rhs.xm_aSmall);
				rhs.xm_pEndOfStor	= (Element_t *)std::end(rhs.xm_aSmall);
			} else {
				Reserve((std::size_t)(rhs.xm_pEnd - rhs.xm_pBegin));
				for(auto pRead = rhs.xm_pBegin; pRead != rhs.xm_pEnd; ++pRead){
					PushNoCheck(std::move_if_noexcept(*pRead));
				}
			}
		}
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector &operator=(const VVector<Element_t, OTHER_ALT_STOR_THLD> &rhs){
		Clear();
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector &operator=(VVector<Element_t, OTHER_ALT_STOR_THLD> &&rhs){
		Clear();
		MoveToEnd(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	VVector &operator=(std::initializer_list<Element_t> rhs){
		Clear();
		Reserve(rhs.size());
		for(auto it = rhs.begin(); it != rhs.end(); ++it){
			PushNoCheck(std::move(*it));
		}
		return *this;
	}
	~VVector(){
		Clear();
	}

private:
	void xCheckCapacity(std::size_t uRequired){
		if(GetCapacity() < uRequired){
			auto uNewCapacity = uRequired;
			uNewCapacity += (uNewCapacity >> 1);
			uNewCapacity = (uNewCapacity + 0xF) & -0x10;

			std::unique_ptr<unsigned char []> pNewLarge(new unsigned char[sizeof(Element_t) * uNewCapacity]);
			const auto pWriteBegin = (Element_t *)pNewLarge.get();
			const auto pWriteEnd = xMoveArray(pWriteBegin, xm_pBegin, xm_pEnd);

			xm_pLarge.swap(pNewLarge);
			xm_pBegin		= pWriteBegin;
			xm_pEnd			= pWriteEnd;
			xm_pEndOfStor	= pWriteBegin + uNewCapacity;
		}
	}

	bool xIsSmall() const noexcept {
		return xm_pBegin == (Element_t *)std::begin(xm_aSmall);
	}

	void xSwapWithLarge(VVector &rhs) noexcept(std::is_nothrow_move_constructible<Element_t>::value) {
		ASSERT(xIsSmall() && !rhs.xIsSmall());

		const auto pWriteBegin = (Element_t *)std::begin(rhs.xm_aSmall);
		const auto pWriteEnd = xMoveArray(pWriteBegin, xm_pBegin, xm_pEnd);

		std::swap(xm_pLarge, rhs.xm_pLarge);
		xm_pBegin			= rhs.xm_pBegin;
		xm_pEnd				= rhs.xm_pEnd;
		xm_pEndOfStor		= rhs.xm_pEndOfStor;
		rhs.xm_pBegin		= pWriteBegin;
		rhs.xm_pEnd			= pWriteEnd;
		rhs.xm_pEndOfStor	= (Element_t *)std::end(rhs.xm_aSmall);
	}
	template<typename Test_t = int>
	void xSwapSmall(
		VVector &rhs,
		typename std::enable_if<std::is_nothrow_move_constructible<Element_t>::value, Test_t>::type = 0
	) noexcept(std::is_nothrow_move_constructible<Element_t>::value) {
		ASSERT(xIsSmall() && rhs.xIsSmall());

		unsigned char aTemp[sizeof(Element_t) * ALT_STOR_THLD];

		const auto pTempBegin = (Element_t *)std::begin(aTemp);
		const auto pTempEnd = xMoveArray(pTempBegin, xm_pBegin, xm_pEnd);
		xm_pEnd = xMoveArray(xm_pBegin, rhs.xm_pBegin, rhs.xm_pEnd);
		rhs.xm_pEnd = xMoveArray(rhs.xm_pBegin, pTempBegin, pTempEnd);
	}
	template<typename Test_t = int>
	void xSwapSmall(
		VVector &rhs,
		typename std::enable_if<!std::is_nothrow_move_constructible<Element_t>::value, Test_t>::type = 0
	){
		ASSERT(xIsSmall() && rhs.xIsSmall());

		rhs.Reserve(ALT_STOR_THLD + 1);
		xSwapWithLarge(rhs);
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
		return xm_pBegin;
	}
	Element_t *GetData() noexcept {
		return xm_pBegin;
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(xm_pEnd - xm_pBegin);
	}
	void Resize(std::size_t uSize){
		const std::size_t uCurrentSize = GetSize();
		if(uSize > uCurrentSize){
			FillAtEnd(uSize - uCurrentSize);
		} else if(uSize < uCurrentSize){
			TruncateFromEnd(uCurrentSize - uSize);
		}
	}
	bool IsEmpty() const noexcept {
		return xm_pEnd == xm_pBegin;
	}
	void Clear() noexcept {
		while(!IsEmpty()){
			PopNoCheck();
		}
	}

	std::size_t GetCapacity() const noexcept {
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uCapacity){
		xCheckCapacity(uCapacity);
	}

	template<typename... Params_t>
	Element_t &PushNoCheck(Params_t &&...vParams){
		ASSERT_MSG(xm_pEnd != xm_pEndOfStor, L"容器已满。");

		new(xm_pEnd) Element_t(std::forward<Params_t>(vParams)...);
		return *(xm_pEnd++);
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(xm_pEnd != xm_pBegin, L"容器为空。");

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
	void CopyToEnd(const Element_t *pFrom, const Element_t *pTo){
		Reserve(GetSize() + (pTo - pFrom));
		auto pRead = pFrom;
		while(pRead != pTo){
			PushNoCheck(*(pRead++));
		}
	}
	void MoveToEnd(const Element_t *pFrom, const Element_t *pTo){
		Reserve(GetSize() + (pTo - pFrom));
		auto pRead = pFrom;
		while(pRead != pTo){
			PushNoCheck(std::move(*(pRead++)));
		}
	}
	void CopyToEnd(const Element_t *pFrom, std::size_t uCount){
		CopyToEnd(pFrom, pFrom + uCount);
	}
	void MoveToEnd(Element_t *pFrom, std::size_t uCount){
		MoveToEnd(pFrom, pFrom + uCount);
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			PopNoCheck();
		}
	}

	void Swap(VVector &rhs) noexcept(std::is_nothrow_move_constructible<Element_t>::value) {
		if(&rhs == this){
			return;
		}

		if(xIsSmall()){
			if(!rhs.xIsSmall()){
				std::swap(xm_pLarge, rhs.xm_pLarge);
				std::swap(xm_pBegin, rhs.xm_pBegin);
				std::swap(xm_pEnd, rhs.xm_pEnd);
				std::swap(xm_pEndOfStor, rhs.xm_pEndOfStor);
			} else {
				xSwapWithLarge(rhs);
			}
		} else {
			if(!rhs.xIsSmall()){
				rhs.xSwapWithLarge(*this);
			} else {
				xSwapSmall(rhs);
			}
		}
	}

public:
	explicit operator const Element_t *() const noexcept {
		return xm_pBegin;
	}
	explicit operator Element_t *() noexcept {
		return xm_pBegin;
	}
	const Element_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return xm_pBegin[uIndex];
	}
	Element_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return xm_pBegin[uIndex];
	}
};

template<class Element_t>
class VVector<Element_t, 0u> {
private:
	template<typename Test_t = int>
	static Element_t *xMoveArray(
		Element_t *pOut,
		Element_t *pBegin,
		Element_t *pEnd,
		typename std::enable_if<std::is_nothrow_move_constructible<Element_t>::value, Test_t>::type = 0
	) noexcept {
		const auto pNewEnd = std::move(pBegin, pEnd, pOut);
		auto pToDestroy = pEnd;
		while(pToDestroy != pBegin){
			(--pToDestroy)->~Element_t();
		}
		return pNewEnd;
	}
	template<typename Test_t = int>
	static Element_t *xMoveArray(
		Element_t *pOut,
		Element_t *pBegin,
		Element_t *pEnd,
		typename std::enable_if<!std::is_nothrow_move_constructible<Element_t>::value, Test_t>::type = 0
	){
		auto pWrite = pOut;
		auto pRead = pBegin;
		try {
			while(pRead != pEnd){
				new(pWrite) Element_t(*pRead);
				++pWrite;
				++pRead;
			}
		} catch(...){
			while(pWrite != pOut){
				(--pWrite)->~Element_t();
			}
			throw;
		}
		while(pRead != pBegin){
			(--pRead)->~Element_t();
		}
		return pWrite;
	}

private:
	std::unique_ptr<unsigned char []> xm_pLarge;

	Element_t *xm_pBegin;
	Element_t *xm_pEnd;
	const Element_t *xm_pEndOfStor;

public:
	constexpr VVector() noexcept
		: xm_pBegin		()
		, xm_pEnd		()
		, xm_pEndOfStor	()
	{
	}
	template<typename... Params_t>
	VVector(std::size_t uCount, const Params_t &...vParams) : VVector() {
		FillAtEnd(uCount, vParams...);
	}
	VVector(const Element_t *pFrom, const Element_t *pTo) : VVector() {
		CopyToEnd(pFrom, pTo);
	}
	VVector(const Element_t *pFrom, std::size_t uCount) : VVector() {
		CopyToEnd(pFrom, uCount);
	}
	VVector(std::initializer_list<Element_t> rhs) : VVector() {
		Reserve(rhs.size());
		for(auto it = rhs.begin(); it != rhs.end(); ++it){
			PushNoCheck(std::move(*it));
		}
	}
	VVector(const VVector &rhs) : VVector(rhs.GetBegin(), rhs.GetEnd()) {
	}
	VVector(VVector &&rhs) : VVector() {
		*this = std::move(rhs);
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector(const VVector<Element_t, OTHER_ALT_STOR_THLD> &rhs) : VVector(rhs.GetBegin(), rhs.GetEnd()) {
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector(VVector<Element_t, OTHER_ALT_STOR_THLD> &&rhs) : VVector() {
		MoveToEnd(rhs.GetBegin(), rhs.GetEnd());
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			Clear();
			CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs){
		if(&rhs != this){
			Clear();

			xm_pLarge			= std::move(rhs.xm_pLarge);
			xm_pBegin			= rhs.xm_pBegin;
			xm_pEnd				= rhs.xm_pEnd;
			xm_pEndOfStor		= rhs.xm_pEndOfStor;

			rhs.xm_pBegin		= nullptr;
			rhs.xm_pEnd			= nullptr;
			rhs.xm_pEndOfStor	= nullptr;
		}
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector &operator=(const VVector<Element_t, OTHER_ALT_STOR_THLD> &rhs){
		Clear();
		CopyToEnd(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector &operator=(VVector<Element_t, OTHER_ALT_STOR_THLD> &&rhs){
		Clear();
		MoveToEnd(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	VVector &operator=(std::initializer_list<Element_t> rhs){
		Clear();
		Reserve(rhs.size());
		for(auto it = rhs.begin(); it != rhs.end(); ++it){
			PushNoCheck(std::move(*it));
		}
		return *this;
	}
	~VVector(){
		Clear();
	}

private:
	void xCheckCapacity(std::size_t uRequired){
		if(GetCapacity() < uRequired){
			auto uNewCapacity = uRequired;
			uNewCapacity += (uNewCapacity >> 1);
			uNewCapacity = (uNewCapacity + 0xF) & -0x10;

			std::unique_ptr<unsigned char []> pNewLarge(new unsigned char[sizeof(Element_t) * uNewCapacity]);
			const auto pWriteBegin = (Element_t *)pNewLarge.get();
			const auto pWriteEnd = xMoveArray(pWriteBegin, xm_pBegin, xm_pEnd);

			xm_pLarge.swap(pNewLarge);
			xm_pBegin		= pWriteBegin;
			xm_pEnd			= pWriteEnd;
			xm_pEndOfStor	= pWriteBegin + uNewCapacity;
		}
	}

public:
	const Element_t *GetBegin() const noexcept {
		return xm_pBegin;
	}
	Element_t *GetBegin() noexcept {
		return xm_pBegin;
	}
	const Element_t *GetEnd() const noexcept {
		return xm_pEnd;
	}
	Element_t *GetEnd() noexcept {
		return xm_pEnd;
	}

	const Element_t *GetData() const noexcept {
		return xm_pBegin;
	}
	Element_t *GetData() noexcept {
		return xm_pBegin;
	}

	std::size_t GetSize() const noexcept {
		return (std::size_t)(xm_pEnd - xm_pBegin);
	}
	void Resize(std::size_t uSize){
		const std::size_t uCurrentSize = GetSize();
		if(uSize > uCurrentSize){
			FillAtEnd(uSize - uCurrentSize);
		} else if(uSize < uCurrentSize){
			TruncateFromEnd(uCurrentSize - uSize);
		}
	}
	bool IsEmpty() const noexcept {
		return xm_pEnd == xm_pBegin;
	}
	void Clear() noexcept {
		while(!IsEmpty()){
			PopNoCheck();
		}
	}

	std::size_t GetCapacity() const noexcept {
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uCapacity){
		xCheckCapacity(uCapacity);
	}

	template<typename... Params_t>
	Element_t &PushNoCheck(Params_t &&...vParams){
		ASSERT_MSG(xm_pEnd != xm_pEndOfStor, L"容器已满。");

		new(xm_pEnd) Element_t(std::forward<Params_t>(vParams)...);
		return *(xm_pEnd++);
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(xm_pEnd != xm_pBegin, L"容器为空。");

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
	void CopyToEnd(const Element_t *pFrom, const Element_t *pTo){
		Reserve(GetSize() + (pTo - pFrom));
		auto pRead = pFrom;
		while(pRead != pTo){
			PushNoCheck(*(pRead++));
		}
	}
	void MoveToEnd(const Element_t *pFrom, const Element_t *pTo){
		Reserve(GetSize() + (pTo - pFrom));
		auto pRead = pFrom;
		while(pRead != pTo){
			PushNoCheck(std::move(*(pRead++)));
		}
	}
	void CopyToEnd(const Element_t *pFrom, std::size_t uCount){
		CopyToEnd(pFrom, pFrom + uCount);
	}
	void MoveToEnd(Element_t *pFrom, std::size_t uCount){
		MoveToEnd(pFrom, pFrom + uCount);
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			PopNoCheck();
		}
	}

	void Swap(VVector &rhs) noexcept {
		if(&rhs == this){
			return;
		}

		std::swap(xm_pLarge, xm_pLarge);
		std::swap(xm_pBegin, xm_pBegin);
		std::swap(xm_pEnd, xm_pEnd);
		std::swap(xm_pEndOfStor, xm_pEndOfStor);
	}

public:
	explicit operator const Element_t *() const noexcept {
		return xm_pBegin;
	}
	explicit operator Element_t *() noexcept {
		return xm_pBegin;
	}
	const Element_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return xm_pBegin[uIndex];
	}
	Element_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return xm_pBegin[uIndex];
	}
};

template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *begin(const VVector<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
Element_t *begin(VVector<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *cbegin(const VVector<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetCBegin();
}

template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *end(const VVector<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
Element_t *end(VVector<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *cend(const VVector<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetCEnd();
}

template<class Element_t>
using Vector = VVector<Element_t, 0>;

}

#endif
