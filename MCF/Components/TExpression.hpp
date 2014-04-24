// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_T_EXPRESSION_HPP_
#define MCF_T_EXPRESSION_HPP_

#include "../Core/String.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/VVector.hpp"
#include <memory>
#include <utility>

namespace MCF {

struct TExpressionNode {
	WideString m_wcsName;
	VVector<std::unique_ptr<TExpressionNode>> m_vecChildren;
};

class TExpression : public TExpressionNode {
public:
	typedef enum {
		ERR_NONE,
	} ErrorType;

	typedef TExpressionNode Node;

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  ") const;
};

}

#endif
