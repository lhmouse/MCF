// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_VVECTOR_HPP__
#define __MCF_CRT_VVECTOR_HPP__

#include <memory>
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <utility>
#include <cstddef>
#include <cassert>

namespace MCF {

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD = 0x400ul / sizeof(ELEMENT_T)>
class VVector {
private:
	unsigned char xm_aSmall[sizeof(ELEMENT_T) * ALT_STOR_THLD];
	std::unique_ptr<unsigned char[]> xm_pLarge;

	ELEMENT_T *xm_pBegin;
	ELEMENT_T *xm_pEnd;
	const ELEMENT_T *xm_pEndOfStor;
public:
	VVector(){
		xm_pBegin		= (ELEMENT_T *)std::begin(xm_aSmall);
		xm_pEnd			= (ELEMENT_T *)std::begin(xm_aSmall);
		xm_pEndOfStor	= (ELEMENT_T *)std::end(xm_aSmall);
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
		for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
			xPushNoCheck(*iter);
		}
	}
	VVector(const VVector &rhs) : VVector() {
		*this = rhs;
	}
	VVector(VVector &&rhs) : VVector() {
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
				for(auto pRead = rhs.xm_pBegin; pRead != rhs.xm_pEnd; ++pRead){
					if(std::is_nothrow_move_constructible<ELEMENT_T>::value){
						xPushNoCheck(std::move(*pRead));
					} else {
						xPushNoCheck(*pRead);
					}
				}
			}
		}
		return *this;
	}
	VVector &operator=(std::initializer_list<ELEMENT_T> rhs){
		Clear();
		Reserve(rhs.size());
		for(auto iter = rhs.begin(); iter != rhs.end(); ++iter){
			xPushNoCheck(*iter);
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
#ifdef __amd64__
			uNewCapacity |= uNewCapacity >> 32;
#endif
			++uNewCapacity;

			std::unique_ptr<unsigned char[]> pNewLarge(new unsigned char[sizeof(ELEMENT_T) * uNewCapacity]);
			const auto pWriteBegin = (ELEMENT_T *)pNewLarge.get();
			const std::size_t uSize = (std::size_t)(xm_pEnd - xm_pBegin);

			auto pRead = xm_pBegin;
			auto pWrite = pWriteBegin;
			if(std::is_nothrow_move_constructible<ELEMENT_T>::value){
				while(pRead != xm_pEnd){
					new(pWrite) ELEMENT_T(std::move(*pRead));
					++pRead;
					++pWrite;
				}
			} else {
				try {
					while(pRead != xm_pEnd){
						new(pWrite) ELEMENT_T(*(const ELEMENT_T *)pRead);
						++pRead;
						++pWrite;
					}
				} catch(...){
					while(pWrite != pWriteBegin){
						(--pWrite)->~ELEMENT_T();
					}
					throw;
				}
			}
			while(pRead != xm_pBegin){
				(--pRead)->~ELEMENT_T();
			}

			xm_pLarge.swap(pNewLarge);
			xm_pBegin		= pWriteBegin;
			xm_pEnd			= pWriteBegin + uSize;
			xm_pEndOfStor	= pWriteBegin + uNewCapacity;
		}
	}
	template<typename... PARAM_T>
	void xPushNoCheck(PARAM_T &&...Params){
		new(xm_pEnd++) ELEMENT_T(std::forward<PARAM_T>(Params)...);
	}
	void xPopNoCheck(){
		(--xm_pEnd)->~ELEMENT_T();
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
			xPopNoCheck();
		}
	}

	std::size_t GetCapacity() const noexcept {
		return (std::size_t)(xm_pEndOfStor - xm_pBegin);
	}
	void Reserve(std::size_t uCapacity){
		xCheckCapacity(uCapacity);
	}

	template<typename... PARAM_T>
	void Push(PARAM_T &&...Params){
		Reserve(GetSize() + 1);
		xPushNoCheck(std::forward<PARAM_T>(Params)...);
	}
	void Pop() noexcept {
		assert(!IsEmpty());

		xPopNoCheck();
	}
	template<typename... PARAM_T>
	void FillAtEnd(std::size_t uCount, const PARAM_T &...Params){
		Reserve(GetSize() + uCount);
		for(std::size_t i = 0; i < uCount; ++i){
			xPushNoCheck(Params...);
		}
	}
	void CopyToEnd(const ELEMENT_T *pFrom, std::size_t uCount){
		Reserve(GetSize() + uCount);
		auto pRead = pFrom;
		for(std::size_t i = 0; i < uCount; ++i){
			xPushNoCheck(*(pRead++));
		}
	}
	void MoveToEnd(ELEMENT_T *pFrom, std::size_t uCount){
		Reserve(GetSize() + uCount);
		auto pRead = pFrom;
		for(std::size_t i = 0; i < uCount; ++i){
			xPushNoCheck(std::move(*(pRead++)));
		}
	}
	void TruncateFromEnd(std::size_t uCount) noexcept {
		assert(GetSize() >= uCount);

		for(std::size_t i = 0; i < uCount; ++i){
			xPopNoCheck();
		}
	}

	void Swap(VVector &rhs){
		if(&rhs == this){
			return;
		}

		const auto SwapSmallAndLarge = [](VVector &vecSmall, VVector &vecLarge) -> void {
			const auto pWriteBegin = (ELEMENT_T *)std::begin(vecLarge.xm_aSmall);
			auto pRead = vecSmall.xm_pBegin;
			auto pWrite = pWriteBegin;
			if(std::is_nothrow_move_constructible<ELEMENT_T>::value){
				while(pRead != vecSmall.xm_pEnd){
					new(pWrite) ELEMENT_T(std::move(*pRead));
					++pRead;
					++pWrite;
				}
			} else {
				try {
					while(pRead != vecSmall.xm_pEnd){
						new(pWrite) ELEMENT_T(*(const ELEMENT_T *)pRead);
						++pRead;
						++pWrite;
					}
				} catch(...){
					while(pWrite != pWriteBegin){
						(--pWrite)->~ELEMENT_T();
					}
					throw;
				}
			}
			while(pRead != vecSmall.xm_pBegin){
				(--pRead)->~ELEMENT_T();
			}

			std::swap(vecSmall.xm_pLarge, vecLarge.xm_pLarge);
			vecSmall.xm_pBegin = vecLarge.xm_pBegin;
			vecSmall.xm_pEnd = vecLarge.xm_pEnd;
			vecSmall.xm_pEndOfStor = vecLarge.xm_pEndOfStor;
			vecLarge.xm_pBegin = pWriteBegin;
			vecLarge.xm_pEnd = pWrite;
			vecLarge.xm_pEndOfStor = (ELEMENT_T *)std::end(vecLarge.xm_aSmall);
		};

		if(xm_pBegin != (ELEMENT_T *)std::begin(xm_aSmall)){
			if(rhs.xm_pBegin != (ELEMENT_T *)std::begin(rhs.xm_aSmall)){
				std::swap(xm_pLarge, rhs.xm_pLarge);
				std::swap(xm_pBegin, rhs.xm_pBegin);
				std::swap(xm_pEnd, rhs.xm_pEnd);
				std::swap(xm_pEndOfStor, rhs.xm_pEndOfStor);
			} else {
				SwapSmallAndLarge(rhs, *this);
			}
		} else {
			if(rhs.xm_pBegin != (ELEMENT_T *)std::begin(rhs.xm_aSmall)){
				SwapSmallAndLarge(*this, rhs);
			} else {
				if(std::is_nothrow_move_constructible<ELEMENT_T>::value){
					unsigned char aTemp[sizeof(ELEMENT_T) * ALT_STOR_THLD];

					const auto MoveAndDestroy = [](ELEMENT_T *&pOut, ELEMENT_T *pBegin, ELEMENT_T *&pEnd){
						for(auto pRead = pBegin; pRead != pEnd; ++pRead){
							new(pOut++) ELEMENT_T(std::move(*pRead));
						}
						while(pEnd != pBegin){
							(--pEnd)->~ELEMENT_T();
						}
					};

					const auto pTempBegin = (ELEMENT_T *)std::begin(aTemp);
					auto pTempEnd = pTempBegin;

					MoveAndDestroy(pTempEnd, xm_pBegin, xm_pEnd);
					MoveAndDestroy(xm_pEnd, rhs.xm_pBegin, rhs.xm_pEnd);
					MoveAndDestroy(rhs.xm_pEnd, pTempBegin, pTempEnd);
				} else {
					rhs.Reserve(ALT_STOR_THLD + 1);
					SwapSmallAndLarge(*this, rhs);
				}
			}
		}
	}
	void Swap(VVector &&rhs){
		Swap(rhs);
	}
public:
	explicit operator const ELEMENT_T *() const noexcept {
		return xm_pBegin;
	}
	explicit operator ELEMENT_T *() noexcept {
		return xm_pBegin;
	}
	const ELEMENT_T &operator[](std::size_t uIndex) const noexcept {
		assert(uIndex < GetSize());

		return xm_pBegin[uIndex];
	}
	ELEMENT_T &operator[](std::size_t uIndex) noexcept {
		assert(uIndex < GetSize());

		return xm_pBegin[uIndex];
	}
};

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *begin(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec){
	return vec.GetBegin();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
ELEMENT_T *begin(VVector<ELEMENT_T, ALT_STOR_THLD> &vec){
	return vec.GetBegin();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *cbegin(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec){
	return vec.GetBegin();
}

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *end(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec){
	return vec.GetEnd();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
ELEMENT_T *end(VVector<ELEMENT_T, ALT_STOR_THLD> &vec){
	return vec.GetEnd();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *cend(const VVector<ELEMENT_T, ALT_STOR_THLD> &vec){
	return vec.GetEnd();
}

}

#endif
