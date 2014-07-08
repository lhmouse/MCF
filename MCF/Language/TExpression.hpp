// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_T_EXPRESSION_HPP_
#define MCF_T_EXPRESSION_HPP_

#include "../Core/String.hpp"
#include <deque>
#include <utility>

namespace MCF {

struct TExpressionNode {
public:
	typedef std::pair<WideString, TExpressionNode> Child;

public:
	std::deque<Child> m_deqChildren;
};

class TExpression : public TExpressionNode {
public:
	enum ErrorType {
		ERR_NONE,
		ERR_UNEXCEPTED_NODE_CLOSE,
		ERR_UNCLOSED_QUOTE,
		ERR_UNCLOSED_NODE,
	};

	typedef TExpressionNode Node;

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  "_wso) const;
};

}

#endif
