// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_BLACK_WHITE_HOLE_HPP_
#define MCF_FUNCTION_BLACK_WHITE_HOLE_HPP_

namespace MCF {

struct BlackHole {
	template<typename ...ParamsT>
	constexpr BlackHole(ParamsT &&...) noexcept {
		//
	}
	template<typename ...ParamsT>
	void operator()(ParamsT &&...) const noexcept {
		//
	}
};

struct WhiteHole {
	template<typename ParamT>
	constexpr operator ParamT() const {
		return { };
	}
	template<typename ...ParamsT>
	void operator()(ParamsT &...vParams) const {
		(static_cast<void>(vParams = { }), ...);
	}
};

}

#endif
