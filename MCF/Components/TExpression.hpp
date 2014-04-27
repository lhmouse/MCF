// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_T_EXPRESSION_HPP_
#define MCF_T_EXPRESSION_HPP_

#include "../Core/String.hpp"
#include "../Core/VVector.hpp"
#include <utility>

namespace MCF {

class TExpressionNode {
	friend class TExpression;

public:
	typedef std::pair<WideString, TExpressionNode> Child;

private:
	Vector<Child> xm_vecChildren;

public:
	const Vector<Child> &GetChildren() const noexcept {
		return xm_vecChildren;
	}
	Vector<Child> &GetChildren() noexcept {
		return xm_vecChildren;
	}
	TExpressionNode &PushChild(const WideStringObserver &wsoName){
		return xm_vecChildren.Push(WideString(wsoName), TExpressionNode()).second;
	}
	void Clear() noexcept {
		xm_vecChildren.Clear();
	}
};

class TExpression : public TExpressionNode {
public:
	typedef enum {
		ERR_NONE,
		ERR_UNEXCEPTED_NODE_CLOSE,
		ERR_UNCLOSED_QUOTE,
		ERR_UNCLOSED_NODE
	} ErrorType;

	typedef TExpressionNode Node;

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  ") const;
};

}

#endif
