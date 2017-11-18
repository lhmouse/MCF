// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_COMPARATORS_HPP_
#define MCF_FUNCTION_COMPARATORS_HPP_

namespace MCF {

struct Equal {
	template<typename SelfT, typename OtherT>
	constexpr decltype(auto) operator()(const SelfT &vSelf, const OtherT &vOther) const noexcept(noexcept(vSelf == vOther)) {
		return vSelf == vOther;
	}
};

struct Unequal {
	template<typename SelfT, typename OtherT>
	constexpr decltype(auto) operator()(const SelfT &vSelf, const OtherT &vOther) const noexcept(noexcept(vSelf != vOther)) {
		return vSelf != vOther;
	}
};

struct Less {
	template<typename SelfT, typename OtherT>
	constexpr decltype(auto) operator()(const SelfT &vSelf, const OtherT &vOther) const noexcept(noexcept(vSelf < vOther)) {
		return vSelf < vOther;
	}
};

struct Greater {
	template<typename SelfT, typename OtherT>
	constexpr decltype(auto) operator()(const SelfT &vSelf, const OtherT &vOther) const noexcept(noexcept(vSelf > vOther)) {
		return vSelf > vOther;
	}
};

struct LessEqual {
	template<typename SelfT, typename OtherT>
	constexpr decltype(auto) operator()(const SelfT &vSelf, const OtherT &vOther) const noexcept(noexcept(vSelf <= vOther)) {
		return vSelf <= vOther;
	}
};

struct GreaterEqual {
	template<typename SelfT, typename OtherT>
	constexpr decltype(auto) operator()(const SelfT &vSelf, const OtherT &vOther) const noexcept(noexcept(vSelf >= vOther)) {
		return vSelf >= vOther;
	}
};

}

#endif
