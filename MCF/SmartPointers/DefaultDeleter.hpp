// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_DEFAULT_DELETER_HPP_
#define MCF_SMART_POINTERS_DEFAULT_DELETER_HPP_

namespace MCF {

template<class T>
struct DefaultDeleter {
	constexpr T *operator()() const noexcept {
		return nullptr;
	}
	void operator()(T *p) const noexcept {
		delete p;
	}
};

template<class T>
struct DefaultDeleter<T []> {
	constexpr T *operator()() const noexcept {
		return nullptr;
	}
	void operator()(T *p) const noexcept {
		delete[] p;
	}
};

}

#endif
