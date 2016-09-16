// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_WHITE_HOLE_HPP_
#define MCF_CORE_WHITE_HOLE_HPP_

#include "../Config.hpp"

namespace MCF {

struct WhiteHole {
	template<typename ParamT>
	constexpr operator ParamT() const noexcept(ParamT(ParamT())) {
		return ParamT();
	}
};

}

#endif
