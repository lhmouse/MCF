// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_COMPARATORS_HPP_
#define MCF_CORE_COMPARATORS_HPP_

namespace MCF {

struct Equal {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t == u)) {
		return t == u;
	}
};
struct Unequal {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t != u)) {
		return t != u;
	}
};
struct Less {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t < u)) {
		return t < u;
	}
};
struct Greater {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t > u)) {
		return t > u;
	}
};
struct LessEqual {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t <= u)) {
		return t <= u;
	}
};
struct GreaterEqual {
	template<typename T, typename U>
	constexpr decltype(auto) operator()(const T &t, const U &u) const noexcept(noexcept(t >= u)) {
		return t >= u;
	}
};

}

#endif
