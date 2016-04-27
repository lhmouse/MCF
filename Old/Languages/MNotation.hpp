// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_LANGUAGES_M_NOTATION_HPP_
#define MCF_LANGUAGES_M_NOTATION_HPP_

#include "../Core/String.hpp"
#include "../Containers/MultiIndexMap.hpp"
#include <utility>

namespace MCF {

class MNotationNode {
	friend class MNotation;

private:
	struct xChildComparator {
		int operator()(const std::pair<WideString, MNotationNode> &lhs, const std::pair<WideString, MNotationNode> &rhs) const noexcept {
			return lhs.first.Compare(rhs.first);
		}
		int operator()(const std::pair<WideString, MNotationNode> &lhs, const WideStringView &rhs) const noexcept {
			return lhs.first.Compare(rhs);
		}
		int operator()(const WideStringView &lhs, const std::pair<WideString, MNotationNode> &rhs) const noexcept {
			return lhs.Compare(rhs.first);
		}
	};

public:
	using Child = std::pair<WideString, MNotationNode>;

	using ChildMap = MultiIndexMap<Child,
		// MultiOrderedMemberIndex<Child, WideString, &Child::first>,
		MultiOrderedIndex<Child, xChildComparator>,
		SequencedIndex<Child>>;
	using ChildNode = typename ChildMap::Node;
	using ConstCursor = typename ChildMap::ConstCursor<1>;
	using Cursor = typename ChildMap::Cursor<1>;

private:
	ChildMap x_mapChildren;

public:
	bool IsEmpty() const noexcept {
		return x_mapChildren.IsEmpty();
	}
	std::size_t GetSize() const noexcept {
		return x_mapChildren.GetSize();
	}
	void Clear() noexcept {
		x_mapChildren.Clear();
	}

	std::pair<const ChildNode *, const ChildNode *> GetRange(const WideStringView &wsvName) const noexcept {
		return x_mapChildren.GetEqualRange<0>(wsvName);
	}
	std::pair<ChildNode *, ChildNode *> GetRange(const WideStringView &wsvName) noexcept {
		return x_mapChildren.GetEqualRange<0>(wsvName);
	}
	std::pair<ChildNode *, bool> Insert(const WideStringView &wsvName, ChildNode *pSeqPos = nullptr){
		return Insert(WideString(wsvName), pSeqPos);
	}
	std::pair<ChildNode *, bool> Insert(WideString wsName, ChildNode *pSeqPos = nullptr){
		return x_mapChildren.InsertWithHints(false, std::make_tuple(nullptr, pSeqPos), std::move(wsName), MNotationNode());
	}
	ChildNode *Erase(ChildNode *pNode) noexcept {
		const auto pRet = pNode->GetNext<1>();
		x_mapChildren.Erase(pNode);
		return pRet;
	}
	bool Erase(const WideStringView &wsvName) noexcept {
		const auto pPos = x_mapChildren.Find<0>(wsvName);
		if(!pPos){
			return false;
		}
		x_mapChildren.Erase(pPos);
		return true;
	}

	const ChildNode *Get(const WideStringView &wsvName) const noexcept {
		return x_mapChildren.Find<0>(wsvName);
	}
	ChildNode *Get(const WideStringView &wsvName) noexcept {
		return x_mapChildren.Find<0>(wsvName);
	}
	ChildNode *Create(const WideStringView &wsvName, ChildNode *pSeqPos = nullptr) noexcept {
		auto pNode = x_mapChildren.GetLowerBound<0>(wsvName);
		if(!pNode || (pNode->Get().first != wsvName)){
			pNode = x_mapChildren.InsertWithHints(true, std::make_tuple(pNode, pSeqPos), WideString(wsvName), MNotationNode()).first;
		}
		return pNode;
	}
	ChildNode *Create(WideString wsName, ChildNode *pSeqPos = nullptr) noexcept {
		auto pNode = x_mapChildren.GetLowerBound<0>(wsName);
		if(!pNode || (pNode->Get().first != wsName)){
			pNode = x_mapChildren.InsertWithHints(true, std::make_tuple(pNode, pSeqPos), std::move(wsName), MNotationNode()).first;
		}
		return pNode;
	}

	const ChildNode *GetFirst() const noexcept {
		return x_mapChildren.GetFirst<1>();
	}
	ChildNode *GetFirst() noexcept {
		return x_mapChildren.GetFirst<1>();
	}
	const ChildNode *GetLast() const noexcept {
		return x_mapChildren.GetLast<1>();
	}
	ChildNode *GetLast() noexcept {
		return x_mapChildren.GetLast<1>();
	}

	ConstCursor GetFirstCursor() const noexcept {
		return x_mapChildren.GetFirstCursor<1>();
	}
	Cursor GetFirstCursor() noexcept {
		return x_mapChildren.GetFirstCursor<1>();
	}
	ConstCursor GetLastCursor() const noexcept {
		return x_mapChildren.GetLastCursor<1>();
	}
	Cursor GetLastCursor() noexcept {
		return x_mapChildren.GetLastCursor<1>();
	}

	void Swap(MNotationNode &rhs) noexcept {
		x_mapChildren.Swap(rhs.x_mapChildren);
	}
};

inline void swap(MNotationNode &lhs, MNotationNode &rhs) noexcept {
	lhs.Swap(rhs);
}

class MNotation : public MNotationNode {
public:
	enum ErrorType {
		kErrNone					= 0,
		kErrUnexpectedPackageClose	= 1,
		kErrEquExpected				= 2,
		kErrUnclosedPackage			= 3,
		kErrSourcePackageNotFound	= 4,
	};

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringView &wsvData);
	WideString Export(const WideStringView &wsvIndent = L"  "_wsv) const;
};

}

#endif
