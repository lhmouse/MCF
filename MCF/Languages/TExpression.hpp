// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_LANGUAGES_T_EXPRESSION_HPP_
#define MCF_LANGUAGES_T_EXPRESSION_HPP_

#include "../Core/String.hpp"
#include "../Containers/List.hpp"
#include <utility>

namespace MCF {

class TExpressionNode {
	friend class TExpression;

public:
	using Child = std::pair<WideString, TExpressionNode>;

	using ChildList = List<Child>;
	using ChildNode = typename ChildList::Node;
	using ConstCursor = typename ChildList::ConstCursor;
	using Cursor = typename ChildList::Cursor;

private:
	ChildList x_lstChildren;

public:
	bool IsEmpty() const noexcept {
		return x_lstChildren.IsEmpty();
	}
	void Clear() noexcept {
		x_lstChildren.Clear();
	}

	const ChildNode *GetFirstChild() const noexcept {
		return x_lstChildren.GetFirst();
	}
	ChildNode *GetFirstChild() noexcept {
		return x_lstChildren.GetFirst();
	}
	const ChildNode *GetLastChild() const noexcept {
		return x_lstChildren.GetLast();
	}
	ChildNode *GetLastChild() noexcept {
		return x_lstChildren.GetLast();
	}

	ConstCursor GetFirstCursor() const noexcept {
		return x_lstChildren.GetFirstCursor();
	}
	Cursor GetFirstCursor() noexcept {
		return x_lstChildren.GetFirstCursor();
	}
	ConstCursor GetLastCursor() const noexcept {
		return x_lstChildren.GetLastCursor();
	}
	Cursor GetLastCursor() noexcept {
		return x_lstChildren.GetLastCursor();
	}

	ChildNode *Insert(ChildNode *pPos, WideString wsName, TExpressionNode vNode){
		return Insert(pPos, Child(std::move(wsName), std::move(vNode)));
	}
	ChildNode *Insert(ChildNode *pPos, Child vChild){
		return x_lstChildren.Insert(pPos, std::move(vChild));
	}
	ChildNode *Erase(ChildNode *pPos) noexcept {
		return x_lstChildren.Erase(pPos);
	}

	void Swap(TExpressionNode &rhs) noexcept {
		x_lstChildren.Swap(rhs.x_lstChildren);
	}
};

inline void swap(TExpressionNode &lhs, TExpressionNode &rhs) noexcept {
	lhs.Swap(rhs);
}

class TExpression : public TExpressionNode {
public:
	enum ErrorType {
		kErrNone				= 0,
		kErrUnexpectedNodeClose	= 1,
		kErrUnclosedQuote		= 2,
		kErrUnclosedNode		= 3,
	};

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  "_wso) const;
};

}

#endif
