// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_LANGUAGES_M_NOTATION_HPP_
#define MCF_LANGUAGES_M_NOTATION_HPP_

#include "../Core/String.hpp"
#include "../Containers/MultiIndexMap.hpp"
#include <type_traits>
#include <memory>
#include <utility>

namespace MCF {

class MNotationPackage {
	friend class MNotation;

public:
	using Package = std::pair<WideString, MNotationPackage>;
	using Value = std::pair<WideString, WideString>;

private:
	struct xPackageComparator {
		bool operator()(const Package &lhs, const Package &rhs) const noexcept {
			return lhs.first < rhs.first;
		}
		bool operator()(const Package &lhs, const WideStringObserver &rhs) const noexcept {
			return lhs.first < rhs;
		}
		bool operator()(const WideStringObserver &lhs, const Package &rhs) const noexcept {
			return lhs < rhs.first;
		}
	};

public:
	using PackageMap = MultiIndexMap<Package,
		// MultiOrderedMemberIndex<Package, WideString, &Package::first>,
		MultiOrderedIndex<Package, xPackageComparator>,
		SequencedIndex<Package>>;
	using ValueMap = MultiIndexMap<Value,
		MultiOrderedMemberIndex<Value, WideString, &Value::first>,
		SequencedIndex<Value>>;

	using PackageNode = typename PackageMap::Node;
	using ValueNode = typename ValueMap::Node;

private:
	PackageMap xm_mapPackages;
	ValueMap xm_mapValues;

public:
	bool HasNoPackages() const noexcept {
		return xm_mapPackages.IsEmpty();
	}
	void ClearPackages() noexcept {
		xm_mapPackages.Clear();
	}

	std::pair<const PackageNode *, const PackageNode *> GetPackageRange(const WideStringObserver &wsoName) const noexcept {
		return xm_mapPackages.GetEqualRange<0>(wsoName);
	}
	std::pair<PackageNode *, PackageNode *> GetPackageRange(const WideStringObserver &wsoName) noexcept {
		return xm_mapPackages.GetEqualRange<0>(wsoName);
	}
	std::pair<PackageNode *, bool> InsertPackage(const WideStringObserver &wsoName, PackageNode *pSeqPos = nullptr){
		return InsertPackage(WideString(wsoName), pSeqPos);
	}
	std::pair<PackageNode *, bool> InsertPackage(WideString wsName, PackageNode *pSeqPos = nullptr){
		return xm_mapPackages.InsertWithHints(false, std::make_tuple(nullptr, pSeqPos), std::move(wsName), MNotationPackage());
	}
	PackageNode *ErasePackage(PackageNode *pNode) noexcept {
		const auto pRet = pNode->GetNext<1>();
		xm_mapPackages.Erase(pNode);
		return pRet;
	}
	bool ErasePackage(const WideStringObserver &wsoName) noexcept {
		const auto pPos = xm_mapPackages.Find<0>(wsoName);
		if(!pPos){
			return false;
		}
		xm_mapPackages.Erase(pPos);
		return true;
	}

	const PackageNode *GetPackage(const WideStringObserver &wsoName) const noexcept {
		return xm_mapPackages.Find<0>(wsoName);
	}
	PackageNode *GetPackage(const WideStringObserver &wsoName) noexcept {
		return xm_mapPackages.Find<0>(wsoName);
	}
	PackageNode *CreatePackage(const WideStringObserver &wsoName, PackageNode *pSeqPos = nullptr) noexcept {
		auto pNode = xm_mapPackages.GetLowerBound<0>(wsoName);
		if(!pNode || (pNode->first != wsoName)){
			pNode = xm_mapPackages.InsertWithHints(true, std::make_tuple(pNode, pSeqPos), WideString(wsoName), MNotationPackage()).first;
		}
		return pNode;
	}
	PackageNode *CreatePackage(WideString wsName, PackageNode *pSeqPos = nullptr) noexcept {
		auto pNode = xm_mapPackages.GetLowerBound<0>(wsName);
		if(!pNode || (pNode->first != wsName)){
			pNode = xm_mapPackages.InsertWithHints(true, std::make_tuple(pNode, pSeqPos), std::move(wsName), MNotationPackage()).first;
		}
		return pNode;
	}

	const PackageNode *GetFirstPackage() const noexcept {
		return xm_mapPackages.GetFirst<1>();
	}
	PackageNode *GetFirstPackage() noexcept {
		return xm_mapPackages.GetFirst<1>();
	}
	const PackageNode *GetLastPackage() const noexcept {
		return xm_mapPackages.GetLast<1>();
	}
	PackageNode *GetLastPackage() noexcept {
		return xm_mapPackages.GetLast<1>();
	}

	bool HasNoValues() const noexcept {
		return xm_mapValues.IsEmpty();
	}
	void ClearValues() noexcept {
		xm_mapValues.Clear();
	}

	std::pair<const ValueNode *, const ValueNode *> GetValueRange(const WideStringObserver &wsoName) const noexcept {
		return xm_mapValues.GetEqualRange<0>(wsoName);
	}
	std::pair<ValueNode *, ValueNode *> GetValueRange(const WideStringObserver &wsoName) noexcept {
		return xm_mapValues.GetEqualRange<0>(wsoName);
	}
	std::pair<ValueNode *, bool> InsertValue(const WideStringObserver &wsoName, WideString wsValue, ValueNode *pSeqPos = nullptr){
		return InsertValue(WideString(wsoName), std::move(wsValue), pSeqPos);
	}
	std::pair<ValueNode *, bool> InsertValue(WideString wsName, WideString wsValue, ValueNode *pSeqPos = nullptr){
		return xm_mapValues.InsertWithHints(false, std::make_tuple(nullptr, pSeqPos), std::move(wsName), std::move(wsValue));
	}
	ValueNode *EraseValue(ValueNode *pNode) noexcept {
		const auto pRet = pNode->GetNext<1>();
		xm_mapValues.Erase(pNode);
		return pRet;
	}
	std::size_t EraseValues(const WideStringObserver &wsoName) noexcept {
		const auto vRange = xm_mapValues.GetEqualRange<0>(wsoName);
		std::size_t uRet = 0;
		for(auto pCur = vRange.first; pCur != vRange.second; pCur = pCur->GetNext<0>()){
			xm_mapValues.Erase(pCur);
			++uRet;
		}
		return uRet;
	}

	const ValueNode *GetValue(const WideStringObserver &wsoName) const noexcept {
		return xm_mapValues.Find<0>(wsoName);
	}
	ValueNode *GetValue(const WideStringObserver &wsoName) noexcept {
		return xm_mapValues.Find<0>(wsoName);
	}
	ValueNode *SetValue(const WideStringObserver &wsoName, WideString wsValue, ValueNode *pSeqPos = nullptr){
		auto pNode = xm_mapValues.GetLowerBound<0>(wsoName);
		if(!pNode || (pNode->first != wsoName)){
			pNode = xm_mapValues.InsertWithHints(true, std::make_tuple(pNode, pSeqPos), WideString(wsoName), std::move(wsValue)).first;
		} else {
			pNode->second = std::move(wsValue);
		}
		return pNode;
	}
	ValueNode *SetValue(WideString wsName, WideString wsValue, ValueNode *pSeqPos = nullptr){
		auto pNode = xm_mapValues.GetLowerBound<0>(wsName);
		if(!pNode || (pNode->first != wsValue)){
			pNode = xm_mapValues.InsertWithHints(true, std::make_tuple(pNode, pSeqPos), std::move(wsValue), std::move(wsValue)).first;
		} else {
			pNode->second = std::move(wsValue);
		}
		return pNode;
	}

	const ValueNode *GetFirstValue() const noexcept {
		return xm_mapValues.GetFirst<1>();
	}
	ValueNode *GetFirstValue() noexcept {
		return xm_mapValues.GetFirst<1>();
	}
	const ValueNode *GetLastValue() const noexcept {
		return xm_mapValues.GetLast<1>();
	}
	ValueNode *GetLastValue() noexcept {
		return xm_mapValues.GetLast<1>();
	}

	void Clear() noexcept {
		xm_mapPackages.Clear();
		xm_mapValues.Clear();
	}

	void Swap(MNotationPackage &rhs) noexcept {
		xm_mapPackages.Swap(rhs.xm_mapPackages);
		xm_mapValues.Swap(rhs.xm_mapValues);
	}
};

inline void swap(MNotationPackage &lhs, MNotationPackage &rhs) noexcept {
	lhs.Swap(rhs);
}

class MNotation : public MNotationPackage {
public:
	enum ErrorType {
		ERR_NONE						= 0,
		ERR_UNEXCEPTED_PACKAGE_CLOSE	= 1,
		ERR_EQU_EXPECTED				= 2,
		ERR_UNCLOSED_PACKAGE			= 3,
		ERR_SOURCE_PACKAGE_NOT_FOUND	= 4,
	};

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  "_wso) const;
};

}

#endif
