// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_VVECTOR_HPP__
#define __MCF_CRT_VVECTOR_HPP__

#include "../../c/ext/assert.h"
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD = 0x400ul / sizeof(ELEMENT_T)>
class VVector {
private:
	template<typename TEST_T = int>
	static ELEMENT_T *xMoveArray(
		ELEMENT_T *pOut,
		ELEMENT_T *pBegin,
		ELEMENT_T *pEnd,
		typename std::enable_if<std::is_nothrow_move_constructible<ELEMENT_T>::value, TEST_T>::type = 0
	) noexcept {
		const auto pNewEnd = std::move(pBegin, pEnd, pOut);
		auto pToDestroy = pEnd;
		while(pToDestroy != pBegin){
			(--pToDestroy)->~ELEMENT_T();
		}
		return pNewEnd;
	}
	template<typename TEST_T = int>
	static ELEMENT_T *xMoveArray(
		ELEMENT_T *pOut,
		ELEMENT_T *pBegin,
		ELEMENT_T *pEnd,
		typename std::enable_if<!std::is_nothrow_move_constructible<ELEMENT_T>::value, TEST_T>::type = 0
	){
		auto pWrite = pOut;
		auto pRead = pBegin;
		try {
			while(pRead != pEnd){
				new(pWrite) ELEMENT_T(*pRead);
				++pWrite;
				++pRead;
			}
		} catch(...){
			while(pWrite != pOut){
				(--pWrite)->~ELEMENT_T();
			}
			throw;
		}
		while(pRead != pBegin){
			(--pRead)->~ELEMENT_T();
		}
		return pWrite;
	}
private:
	unsigned char xm_aSmall[sizeof(ELEMENT_T) * ALT_STOR_THLD];
	std::unique_ptr<unsigned char[]> xm_pLarge;

	ELEMENT_T *xm_pBegin;
	ELEMENT_T *xm_pEnd;
	const ELEMENT_T *xm_pEndOfStor;
public:
	constexpr VVector() noexcept
		: xm_pBegin((ELEMENT_T *)std::begin(xm_aSmall))
		, xm_pEnd((ELEMENT_T *)std::begin(xm_aSmall))
		, xm_pEndOfStor((ELEMENT_T *)std::end(xm_aSmall))
	{
	}
	template<typename... PARAM_T>
	VVector(std::size_t uCount, const PARAM_T &...Params) : VVector() {
		FillAtEnd(uCount, Params...);
	}
	VVector(const ELEMENT_T *pFrom, std::size_t uCount) : VVector() {
		CopyToEnd(pFrom, uCount);
	}
	VVector(std::initializer_list<ELEMENT_T> rhs) : VVector() {
		Reserve(rhs.size());
		for(auto it = rhs.begin(); it != rhs.end(); ++it){
			PushNoCheck(std::move(*it));
		}
	}
	VVector(const VVector &rhs) : VVector() {
		CopyToEnd(rhs.GetData(), rhs.GetSize());
	}
	VVector(VVector &&rhs) : VVector() {
		*this = std::move(rhs);
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector(const VVector<ELEMENT_T, OTHER_ALT_STOR_THLD> &rhs) : VVector() {
		CopyToEnd(rhs.GetData(), rhs.GetSize());
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector(VVector<ELEMENT_T, OTHER_ALT_STOR_THLD> &&rhs) : VVector() {
		*this = std::move(rhs);
	}
	VVector &operator=(const VVector &rhs){
		if(&rhs != this){
			Clear();
			CopyToEnd(rhs.GetData(), rhs.GetSize());
		}
		return *this;
	}
	VVector &operator=(VVector &&rhs){
		if(&rhs != this){
			Clear();
			if(rhs.xm_pBegin != (ELEMENT_T *)std::begin(rhs.xm_aSmall)){
				xm_pLarge = std::move(rhs.xm_pLarge);
				xm_pBegin = rhs.xm_pBegin;
				xm_pEnd = rhs.xm_pEnd;
				xm_pEndOfStor = rhs.xm_pEndOfStor;

				rhs.xm_pBegin = (ELEMENT_T *)std::begin(rhs.xm_aSmall);
				rhs.xm_pEnd = (ELEMENT_T *)std::begin(rhs.xm_aSmall);
				rhs.xm_pEndOfStor = (ELEMENT_T *)std::end(rhs.xm_aSmall);
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
	VVector &operator=(const VVector<ELEMENT_T, OTHER_ALT_STOR_THLD> &rhs){
		Clear();
		CopyToEnd(rhs.GetData(), rhs.GetSize());
		return *this;
	}
	template<std::size_t OTHER_ALT_STOR_THLD>
	VVector &operator=(VVector<ELEMENT_T, OTHER_ALT_STOR_THLD> &&rhs){
		Clear();
		MoveToEnd(rhs.GetData(), rhs.GetSize());
		return *this;
	}
	VVector &operator=(std::initializer_list<ELEMENT_T> rhs){
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
			// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
			std::size_t uNewCapacity = uRequired;
			--uNewCapacity;
			uNewCapacity |= uNewCapacity >> 1;
			uNewCapacity |= uNewCapacity >> 2;
			uNewCapacity |= uNewCapacity >> 4;
			uNewCapacity |= uNewCapacity >> 8;
			uNewCapacity |= uNewCapacity >> 16;
#ifdef _WIN64
			uNewCapacity |= uNewCapacity >> 32;
#endif
			++uNewCapacity;

			std::unique_ptr<unsigned char[]> pNewLarge(new unsigned char[sizeof(ELEMENT_T) * uNewCapacity]);
			const auto pWriteBegin = (ELEMENT_T *)pNewLarge.get();
			const auto pWriteEnd = xMoveArray(pWriteBegin, xm_pBegin, xm_pEnd);

			xm_pLarge.swap(pNewLarge);
			xm_pBegin		= pWriteBegin;
			xm_pEnd			= pWriteEnd;
			xm_pEndOfStor	= pWriteBegin + uNewCapacity;
		}
	}

	bool xIsSmall() const noexcept {
		return xm_pBegin == (ELEMENT_T *)std::begin(xm_aSmall);
	}

	void xSwapWithLarge(VVector &rhs) noexcept(std::is_nothrow_move_constructible<ELEMENT_T>::value) {
		ASSERT(xIsSmall() && !rhs.xIsSmall());

		const auto pWriteBegin = (ELEMENT_T *)std::begin(rhs.xm_aSmall);
		const auto pWriteEnd = xMoveArray(pWriteBegin, xm_pBegin, xm_pEnd);

		std::swap(xm_pLarge, rhs.xm_pLarge);
		xm_pBegin			= rhs.xm_pBegin;
		xm_pEnd				= rhs.xm_pEnd;
		xm_pEndOfStor		= rhs.xm_pEndOfStor;
		rhs.xm_pBegin		= pWriteBegin;
		rhs.xm_pEnd			= pWriteEnd;
		rhs.xm_pEndOfStor	= (ELEMENT_T *)std::end(rhs.xm_aSmall);
	}
	template<typename TEST_T = int>
	void xSwapSmall(
		VVector &rhs,
		typename std::enable_if<std::is_nothrow_move_constructible<ELEMENT_T>::value, TEST_T>::type = 0
	) noexcept(std::is_nothrow_move_constructible<ELEMENT_T>::value) {
		ASSERT(xIsSmall() && rhs.xIsSmall());

		unsigned char aTemp[sizeof(ELEMENT_T) * ALT_STOR_THLD];

		const auto pTempBegin = (ELEMENT_T *)std::begin(aTemp);
		const auto pTempEnd = xMoveArray(pTempBegin, xm_pBegin, xm_pEnd);
		xm_pEnd = xMoveArray(xm_pBegin, rhs.xm_pBegin, rhs.xm_pEnd);
		rhs.xm_pEnd = xMoveArray(rhs.xm_pBegin, pTempBegin, pTempEnd);
	}
	template<typename TEST_T = int>
	void xSwapSmall(
		VVector &rhs,
		typename std::enable_if<!std::is_nothrow_move_constructible<ELEMENT_T>::value, TEST_T>::type = 0
	){
		ASSERT(xIsSmall() && rhs.xIsSmall());

		rhs.Reserve(ALT_STOR_THLD + 1);
		xSwapWithLarge(rhs);
	}
public:
	const ELEMENT_T *GetBegin() const noexcept {
		return xm_pBegin;
	}
	ELEMENT_T *GetBegin() noexcept {
		return xm_pBegin;
	}
	const ELEMENT_T *GetEnd() const noexcept {
		return xm_pEnd;
	}
	ELEMENT_T *GetEnd() noexcept {
		return xm_pEnd;
	}

	const ELEMENT_T *GetData() const noexcept {
		return xm_pBegin;
	}
	ELEMENT_T *GetData() noexcept {
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

	template<typename... PARAM_T>
	void PushNoCheck(PARAM_T &&...Params){
		ASSERT_MSG(xm_pEnd != xm_pEndOfStor, L"VVector::PushNoCheck() 失败：容器已满。");

		new(xm_pEnd) ELEMENT_T(std::forward<PARAM_T>(Params)...);
		++xm_pEnd;
	}
	void PopNoCheck() noexcept {
		ASSERT_MSG(xm_pEnd != xm_pBegin, L"VVector::PopNoCheck() 失败：容器为空。");

		(--xm_pEnd)->~ELEMENT_T();
	}

	template<typename... PARAM_T>
	void Push(PARAM_T &&...Params){
		Reserve(GetSize() + 1);
		PushNoCheck(std::forward<PARAM_T>(Params)...);
	}
	void Pop() noexcept {
		ASSERT(!IsEmpty());

		PopNoCheck();
	}
	template<typename... PARAM_T>
	void FillAtEnd(std::size_t uCount, const PARAM_T &...Params){
		Reserve(GetSize() + uCount);
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(Params...);
		}
	}
	void CopyToEnd(const ELEMENT_T *pFrom, std::size_t uCount){
		Reserve(GetSize() + uCount);
		auto pRead = pFrom;
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(*pRead);
			++pRead;
		}
	}
	void MoveToEnd(ELEMENT_T *pFrom, std::size_t uCount) noexcept(std::is_nothrow_move_constructible<ELEMENT_T>::value) {
		Reserve(GetSize() + uCount);
		auto pRead = pFrom;
		for(std::size_t i = 0; i < uCount; ++i){
			PushNoCheck(std::move(*(pRead++)));
		}
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		ASSERT(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			PopNoCheck();
		}
	}

	void Swap(VVector &rhs) noexcept(std::is_nothrow_move_constructible<ELEMENT_T>::value) {
		if(&rhs == this){
			return;
		}

		if(xIsSmall()){
			if(rhs.xm_pBegin != (ELEMENT_T *)std::begin(rhs.xm_aSmall)){
				std::swap(xm_pLarge, rhs.xm_pLarge);
				std::swap(xm_pBegin, rhs.xm_pBegin);
				std::swap(xm_pEnd, rhs.xm_pEnd);
				std::swap(xm_pEndOfStor, rhs.xm_pEndOfStor);
			} else {
				xSwapWithLarge(rhs);
			}
		} else {
			if(rhs.xm_pBegin != (ELEMENT_T *)std::begin(rhs.xm_aSmall)){
				rhs.xSwapWithLarge(*this);
			} else {
				xSwapSmall(rhs);
			}
		}
	}
public:
	explicit operator const ELEMENT_T *() const noexcept {
		return xm_pBegin;
	}
	explicit operator ELEMENT_T *() noexcept {
		return xm_pBegin;
	}
	const ELEMENT_T &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"VVector::operator[]() 失败：索引越界。");

		return xm_pBegin[uIndex];
	}
	ELEMENT_T &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"VVector::operator[]() 失败：索引越界。");

		return xm_pBegin[uIndex];
	}
};

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *begin(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
ELEMENT_T *begin(VVector<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *cbegin(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *end(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
ELEMENT_T *end(VVector<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *cend(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}

}

#endif
