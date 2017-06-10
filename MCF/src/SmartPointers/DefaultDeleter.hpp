// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_DEFAULT_DELETER_HPP_
#define MCF_SMART_POINTERS_DEFAULT_DELETER_HPP_

#include <type_traits>
#include <cstddef>

namespace MCF {

template<class T>
struct DefaultDeleter {
	constexpr DefaultDeleter() noexcept = default;

	template<class U,
		std::enable_if_t<
			std::is_same<std::remove_cv_t<U>, std::remove_cv_t<T>>::value ||
				(std::is_convertible<std::remove_cv_t<U> *, std::remove_cv_t<T> *>::value && std::has_virtual_destructor<T>::value),
			int> = 0>
	DefaultDeleter(const DefaultDeleter<U> &) noexcept { }

	constexpr T *operator()() const noexcept {
		return nullptr;
	}
	void operator()(T *p) const noexcept {
		delete p;
	}
};

template<class T>
struct DefaultDeleter<T []> {
	constexpr DefaultDeleter() noexcept = default;

	template<class U,
		std::enable_if_t<
			std::is_same<std::remove_cv_t<U>, std::remove_cv_t<T>>::value,
			int> = 0>
	DefaultDeleter(const DefaultDeleter<U []> &) noexcept { }

	constexpr T *operator()() const noexcept {
		return nullptr;
	}
	void operator()(T *p) const noexcept {
		delete[] p;
	}
};

template<class T, std::size_t N>
struct DefaultDeleter<T [N]> {
	static_assert(((void)sizeof(T), false), "Deleting fixed-sized arrays is not supported.");
};

}

#endif
