// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_BLACK_HOLE_HPP_
#define MCF_FUNCTION_BLACK_HOLE_HPP_

namespace MCF {

struct BlackHole {
	template<typename ParamT>
	BlackHole &operator=(ParamT &&) noexcept {
		return *this;
	}

	template<typename ...ParamsT>
	constexpr void operator()(ParamsT &&...) const noexcept {
	}
};

}

#endif
