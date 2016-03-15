// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_BLACK_HOLE_HPP_
#define MCF_FUNCTION_BLACK_HOLE_HPP_

namespace MCF {

struct BlackHole {
	template<typename ParamT>
	constexpr const BlackHole &operator=(ParamT &&) const noexcept {
		return *this;
	}
	template<typename ...ParamsT>
	constexpr BlackHole &operator()(ParamsT &&...) const noexcept {
		return *this;
	}
};

}

#endif
