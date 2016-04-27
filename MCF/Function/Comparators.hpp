// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_COMPARATORS_HPP_
#define MCF_FUNCTION_COMPARATORS_HPP_

namespace MCF {

struct Equal {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t == u)) {
		return t == u;
	}
	template<typename T, typename U>
	constexpr decltype(auto) operator()(T *const &t, U *const &u) const noexcept {
		return static_cast<const volatile void *>(t) == static_cast<const volatile void *>(u);
	}
};
struct Unequal {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t != u)) {
		return t != u;
	}
	template<typename T, typename U>
	constexpr decltype(auto) operator()(T *const &t, U *const &u) const noexcept {
		return static_cast<const volatile void *>(t) != static_cast<const volatile void *>(u);
	}
};
struct Less {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t < u)) {
		return t < u;
	}
	template<typename T, typename U>
	constexpr decltype(auto) operator()(T *const &t, U *const &u) const noexcept {
		return static_cast<const volatile void *>(t) < static_cast<const volatile void *>(u);
	}
};
struct Greater {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t > u)) {
		return t > u;
	}
	template<typename T, typename U>
	constexpr decltype(auto) operator()(T *const &t, U *const &u) const noexcept {
		return static_cast<const volatile void *>(t) > static_cast<const volatile void *>(u);
	}
};
struct LessEqual {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t <= u)) {
		return t <= u;
	}
	template<typename T, typename U>
	constexpr decltype(auto) operator()(T *const &t, U *const &u) const noexcept {
		return static_cast<const volatile void *>(t) <= static_cast<const volatile void *>(u);
	}
};
struct GreaterEqual {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t >= u)) {
		return t >= u;
	}
	template<typename T, typename U>
	constexpr decltype(auto) operator()(T *const &t, U *const &u) const noexcept {
		return static_cast<const volatile void *>(t) >= static_cast<const volatile void *>(u);
	}
};

}

#endif
