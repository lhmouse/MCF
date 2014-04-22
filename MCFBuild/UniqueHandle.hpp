// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UNIQUE_HANDLE_HPP__
#define __MCF_UNIQUE_HANDLE_HPP__

namespace MCF {

template<typename HANDLE_T, class CLOSER_T>
class UniqueHandle {
private:
	HANDLE_T xm_hObj;
public:
	constexpr UniqueHandle() noexcept : UniqueHandle(CLOSER_T()()) {
	}
	constexpr explicit UniqueHandle(HANDLE_T hObj) noexcept : xm_hObj(hObj) {
	}
	UniqueHandle(UniqueHandle &&rhs) noexcept : UniqueHandle(rhs.Release()) {
	}
	UniqueHandle &operator=(HANDLE_T hObj) noexcept {
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
		return Get() != CLOSER_T()();
	}
	HANDLE_T Get() const noexcept {
		return xm_hObj;
	}
	HANDLE_T Release() noexcept {
		const HANDLE_T hOld = xm_hObj;
		xm_hObj = CLOSER_T()();
		return hOld;
	}

	void Reset(HANDLE_T hObj = CLOSER_T()()) noexcept {
		const HANDLE_T hOld = xm_hObj;
		xm_hObj = hObj;
		if(hOld != CLOSER_T()()){
			CLOSER_T()(hOld);
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
	operator HANDLE_T() const noexcept {
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
