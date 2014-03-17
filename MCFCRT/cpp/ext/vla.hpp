// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_VLA_HPP__
#define __MCF_CRT_VLA_HPP__

#include <memory>
#include <initializer_list>
#include <type_traits>
#include <iterator>
#include <cstddef>

namespace MCF {

template<typename Element_t, std::size_t ALT_STOR_THLD = 64>
class Vla {
private:
	unsigned char xm_aSmall[sizeof(Element_t) * ALT_STOR_THLD];
	std::unique_ptr<unsigned char []> xm_pLarge;

	Element_t *xm_pBegin;
	Element_t *xm_pEnd;

private:
	void xInitStorage(std::size_t uCount){
		if(uCount <= ALT_STOR_THLD){
			xm_pBegin = (Element_t *)std::begin(xm_aSmall);
		} else {
			xm_pLarge.reset(new unsigned char[sizeof(Element_t) * uCount]);
			xm_pBegin = (Element_t *)xm_pLarge.get();
		}
		xm_pEnd = xm_pBegin + uCount;
	}

public:
	explicit Vla(std::size_t uCount){
		xInitStorage(uCount);

		if(!std::is_pod<Element_t>::value){
			for(auto p = xm_pBegin; p != xm_pEnd; ++p){
				new(p) Element_t();
			}
		}
	}
	template<typename... Params_t>
	Vla(std::size_t uCount, const Params_t &...Params){
		xInitStorage(uCount);

		for(auto p = xm_pBegin; p != xm_pEnd; ++p){
			new(p) Element_t(Params...);
		}
	}
	explicit Vla(std::initializer_list<Element_t> InitList){
		xInitStorage(InitList.size());

		auto p = xm_pBegin;
		for(auto iter = InitList.begin(); iter != InitList.end(); ++iter){
			new(p++) Element_t(std::move(*iter));
		}
	}
	~Vla(){
		auto p = xm_pEnd;
		while(p != xm_pBegin){
			(--p)->~Element_t();
		}
	}

	Vla(const Vla &) = delete;
	Vla(Vla &&) = delete;
	void operator=(const Vla &) = delete;
	void operator=(Vla &&) = delete;

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

public:
	explicit operator const Element_t *() const noexcept {
		return xm_pBegin;
	}
	explicit operator Element_t *() noexcept {
		return xm_pBegin;
	}
/*	const Element_t &operator[](std::size_t uIndex) const noexcept {
		return xm_pBegin[uIndex];
	}
	Element_t &operator[](std::size_t uIndex) noexcept {
		return xm_pBegin[uIndex];
	}*/
};


template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *begin(const Vla<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
Element_t *begin(Vla<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *cbegin(const Vla<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetBegin();
}

template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *end(const Vla<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
Element_t *end(Vla<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}
template<typename Element_t, std::size_t ALT_STOR_THLD>
const Element_t *cend(const Vla<Element_t, ALT_STOR_THLD> &vec) noexcept {
	return vec.GetEnd();
}

}

#endif
