// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_INTEROPERATION_T_EXPRESSION_HPP_
#define MCF_INTEROPERATION_T_EXPRESSION_HPP_

#include "../Core/String.hpp"
#include <cstddef>

namespace MCF {

class TExpression {
private:

public:
	constexpr TExpression() noexcept {
	}

public:
	void Swap(TExpression &rhs) noexcept {
		using std::swap;
	}
};

void swap(TExpression &lhs, TExpression &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
