// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_COMPARATORS_HPP_
#define MCF_FUNCTION_COMPARATORS_HPP_

#include <cstdint>

namespace MCF {

struct Equal {
	template<typename Tx, typename Ty>
	constexpr bool operator()(const Tx &x, const Ty &y) const {
		return x == y;
	}
	template<typename Tx, typename Ty>
	bool operator()(Tx *const &px, Ty *const &py) const noexcept {
		return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) == reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));
	}
};
struct Unequal {
	template<typename Tx, typename Ty>
	constexpr bool operator()(const Tx &x, const Ty &y) const {
		return x != y;
	}
	template<typename Tx, typename Ty>
	bool operator()(Tx *const &px, Ty *const &py) const noexcept {
		return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) != reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));
	}
};

struct Less {
	template<typename Tx, typename Ty>
	constexpr bool operator()(const Tx &x, const Ty &y) const {
		return x < y;
	}
	template<typename Tx, typename Ty>
	bool operator()(Tx *const &px, Ty *const &py) const noexcept {
		return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) < reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));
	}
};
struct Greater {
	template<typename Tx, typename Ty>
	constexpr bool operator()(const Tx &x, const Ty &y) const {
		return x > y;
	}
	template<typename Tx, typename Ty>
	bool operator()(Tx *const &px, Ty *const &py) const noexcept {
		return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) > reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));
	}
};

struct LessEqual {
	template<typename Tx, typename Ty>
	constexpr bool operator()(const Tx &x, const Ty &y) const {
		return x <= y;
	}
	template<typename Tx, typename Ty>
	bool operator()(Tx *const &px, Ty *const &py) const noexcept {
		return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) <= reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));
	}
};
struct GreaterEqual {
	template<typename Tx, typename Ty>
	constexpr bool operator()(const Tx &x, const Ty &y) const {
		return x >= y;
	}
	template<typename Tx, typename Ty>
	bool operator()(Tx *const &px, Ty *const &py) const noexcept {
		return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) >= reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));
	}
};


#undef DEFINE_COMPARATOR_

}

#endif
