// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_VLA_HPP__
#define __MCF_CRT_VLA_HPP__

#include <memory>
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <cstddef>

namespace MCF {

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD = 64>
class VLA {
private:
	unsigned char xm_aSmall[sizeof(ELEMENT_T) * ALT_STOR_THLD];
	std::unique_ptr<unsigned char[]> xm_pLarge;

	ELEMENT_T *xm_pBegin;
	ELEMENT_T *xm_pEnd;
private:
	void xInitStorage(std::size_t uCount){
		if(uCount <= ALT_STOR_THLD){
			xm_pBegin = (ELEMENT_T *)std::begin(xm_aSmall);
		} else {
			xm_pLarge.reset(new unsigned char[sizeof(ELEMENT_T) * uCount]);
			xm_pBegin = (ELEMENT_T *)xm_pLarge.get();
		}
		xm_pEnd = xm_pBegin + uCount;
	}
public:
	explicit VLA(std::size_t uCount){
		xInitStorage(uCount);

		if(!std::is_pod<ELEMENT_T>::value){
			for(auto p = xm_pBegin; p != xm_pEnd; ++p){
				new(p) ELEMENT_T();
			}
		}
	}
	template<typename... PARAM_T>
	VLA(std::size_t uCount, const PARAM_T &...Params){
		xInitStorage(uCount);

		for(auto p = xm_pBegin; p != xm_pEnd; ++p){
			new(p) ELEMENT_T(Params...);
		}
	}
	explicit VLA(std::initializer_list<ELEMENT_T> InitList){
		xInitStorage(InitList.size());

		auto p = xm_pBegin;
		for(auto iter = InitList.begin(); iter != InitList.end(); ++iter){
			new(p++) ELEMENT_T(*iter);
		}
	}
	~VLA(){
		auto p = xm_pEnd;
		while(p != xm_pBegin){
			(--p)->~ELEMENT_T();
		}
	}

	VLA(const VLA &) = delete;
	VLA(VLA &&) = delete;
	void operator=(const VLA &) = delete;
	void operator=(VLA &&) = delete;
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
public:
	operator const ELEMENT_T *() const noexcept {
		return xm_pBegin;
	}
	operator ELEMENT_T *() noexcept {
		return xm_pBegin;
	}
/*	const ELEMENT_T &operator[](std::size_t uIndex) const noexcept {
		return xm_pBegin[uIndex];
	}
	ELEMENT_T &operator[](std::size_t uIndex) noexcept {
		return xm_pBegin[uIndex];
	}*/
};


template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *begin(const VLA<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
ELEMENT_T *begin(VLA<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *cbegin(const VLA<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}

template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *end(const VLA<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
ELEMENT_T *end(VLA<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename ELEMENT_T, std::size_t ALT_STOR_THLD>
const ELEMENT_T *cend(const VLA<ELEMENT_T, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}

}

#endif
