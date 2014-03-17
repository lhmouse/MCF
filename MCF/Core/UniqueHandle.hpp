// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UNIQUE_HANDLE_HPP__
#define __MCF_UNIQUE_HANDLE_HPP__

#include <new>
#include <utility>
#include <cstddef>

namespace MCF {

template<class Closer_t>
class UniqueHandle {
public:
	typedef decltype(Closer_t()()) Handle;

private:
	Handle xm_hObj;

public:
	constexpr UniqueHandle() noexcept : UniqueHandle(Closer_t()()) {
	}
	constexpr explicit UniqueHandle(Handle hObj) noexcept : xm_hObj(hObj) {
	}
	UniqueHandle(UniqueHandle &&rhs) noexcept : UniqueHandle(rhs.Release()) {
	}
	UniqueHandle &operator=(Handle hObj) noexcept {
		Reset(hObj);
		return *this;
	}
	UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~UniqueHandle(){
		Reset();
	}

	UniqueHandle(const UniqueHandle &) = delete;
	void operator=(const UniqueHandle &) = delete;

public:
	bool IsGood() const noexcept {
		return Get() != Closer_t()();
	}
	Handle Get() const noexcept {
		return xm_hObj;
	}
	Handle Release() noexcept {
		const Handle hOld = xm_hObj;
		xm_hObj = Closer_t()();
		return hOld;
	}

	void Reset(Handle hObj = Closer_t()()) noexcept {
		const Handle hOld = xm_hObj;
		xm_hObj = hObj;
		if(hOld != Closer_t()()){
			Closer_t()(hOld);
		}
	}
	void Reset(UniqueHandle &&rhs) noexcept {
		if(&rhs == this){
			return;
		}
		Reset(rhs.Release());
	}

	void Swap(UniqueHandle &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		std::swap(xm_hObj, rhs.xm_hObj);
	}

public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	explicit operator Handle() const noexcept {
		return Get();
	}

	bool operator==(const UniqueHandle &rhs) const noexcept {
		return Get() == rhs.Get();
	}
	bool operator!=(const UniqueHandle &rhs) const noexcept {
		return Get() != rhs.Get();
	}
	bool operator<(const UniqueHandle &rhs) const noexcept {
		return Get() < rhs.Get();
	}
	bool operator<=(const UniqueHandle &rhs) const noexcept {
		return Get() <= rhs.Get();
	}
	bool operator>(const UniqueHandle &rhs) const noexcept {
		return Get() > rhs.Get();
	}
	bool operator>=(const UniqueHandle &rhs) const noexcept {
		return Get() >= rhs.Get();
	}
};

}

#endif
