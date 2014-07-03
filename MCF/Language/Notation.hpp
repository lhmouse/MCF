// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_NOTATION_HPP_
#define MCF_NOTATION_HPP_

#include "../Core/String.hpp"
#include "../Core/VVector.hpp"
#include "../Core/MultiIndexedMap.hpp"
#include <functional>
#include <initializer_list>

namespace MCF {

class NotationPackage {
	friend class Notation;

private:
	MultiIndexedMap<NotationPackage, WideString, SequenceIndex> xm_mapPackages;
	MultiIndexedMap<WideString, WideString, SequenceIndex> xm_mapValues;

private:
	NotationPackage() noexcept = default;

public:
	const NotationPackage *GetPackage(const WideStringObserver &wsoName) const noexcept;
	NotationPackage *GetPackage(const WideStringObserver &wsoName) noexcept;
	std::pair<NotationPackage *, bool> CreatePackage(const WideStringObserver &wsoName);
	std::pair<NotationPackage *, bool> CreatePackage(WideString wcsName);
	bool RemovePackage(const WideStringObserver &wsoName) noexcept;

	void TraversePackages(const std::function<void (const NotationPackage &)> &fnCallback) const;
	void TraversePackages(const std::function<void (NotationPackage &)> &fnCallback);

	const WideString *GetValue(const WideStringObserver &wsoName) const noexcept;
	WideString *GetValue(const WideStringObserver &wsoName) noexcept;
	std::pair<WideString *, bool> CreateValue(const WideStringObserver &wsoName, WideString wcsValue);
	std::pair<WideString *, bool> CreateValue(WideString wcsName, WideString wcsValue);
	bool RemoveValue(const WideStringObserver &wsoName) noexcept;

	void TraverseValues(const std::function<void (const WideString &)> &fnCallback) const;
	void TraverseValues(const std::function<void (WideString &)> &fnCallback);

	void Clear() noexcept;

public:
	template<typename PathSegments_t>
	const NotationPackage *GetPackageFromPath(const PathSegments_t &vPathSegments) const noexcept {
		auto pCur = this;
		for(const auto &vSegment : vPathSegments){
			pCur = pCur->GetPackage(vSegment);
			if(!pCur){
				return nullptr;
			}
		}
		return pCur;
	}
	template<typename PathSegments_t>
	NotationPackage *GetPackageFromPath(const PathSegments_t &vPathSegments) noexcept {
		auto pCur = this;
		for(const auto &vSegment : vPathSegments){
			pCur = pCur->GetPackage(vSegment);
			if(!pCur){
				return nullptr;
			}
		}
		return pCur;
	}
	template<typename PathSegments_t>
	std::pair<NotationPackage *, bool> CreatePackageFromPath(const PathSegments_t &vPathSegments){
		auto vRet = std::make_pair(this, false);
		for(const auto &vSegment : vPathSegments){
			vRet = vRet.first->CreatePackage(vSegment);
		}
		return std::move(vRet);
	}

	template<typename PathSegmentIterator_t>
	const NotationPackage *GetPackageFromPath(
		PathSegmentIterator_t itSegmentBegin,
		PathSegmentIterator_t itSegmentEnd
	) const noexcept {
		auto pCur = this;
		while(itSegmentBegin != itSegmentEnd){
			pCur = pCur->GetPackage(*itSegmentBegin);
			if(!pCur){
				return nullptr;
			}
			++itSegmentBegin;
		}
		return pCur;
	}
	template<typename PathSegmentIterator_t>
	NotationPackage *GetPackageFromPath(
		PathSegmentIterator_t itSegmentBegin,
		PathSegmentIterator_t itSegmentEnd
	) noexcept {
		auto pCur = this;
		while(itSegmentBegin != itSegmentEnd){
			pCur = pCur->GetPackage(*itSegmentBegin);
			if(!pCur){
				return nullptr;
			}
			++itSegmentBegin;
		}
		return pCur;
	}
	template<typename PathSegmentIterator_t>
	std::pair<NotationPackage *, bool> CreatePackageFromPath(
		PathSegmentIterator_t itSegmentBegin,
		PathSegmentIterator_t itSegmentEnd
	){
		auto vRet = std::make_pair(this, false);
		while(itSegmentBegin != itSegmentEnd){
			vRet = vRet.first->CreatePackage(*itSegmentBegin);
			++itSegmentBegin;
		}
		return std::move(vRet);
	}
};

class Notation : public NotationPackage {
public:
	enum ErrorType {
		ERR_NONE,
		ERR_NO_VALUE_NAME,
		ERR_NO_PACKAGE_NAME,
		ERR_UNEXCEPTED_PACKAGE_CLOSE,
		ERR_EQU_EXPECTED,
		ERR_UNCLOSED_PACKAGE,
		ERR_SOURCE_PACKAGE_NOT_FOUND,
		ERR_DUPLICATE_PACKAGE,
		ERR_DUPLICATE_VALUE
	};

	typedef NotationPackage Package;

public:
	Notation() noexcept = default;

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  "_wso) const;
};

}

#endif
