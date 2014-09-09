// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_NOTATION_HPP_
#define MCF_NOTATION_HPP_

#include "../Core/String.hpp"
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <memory>

namespace MCF {

class NotationPackage {
	friend class Notation;

private:
	class xPackages;
	class xValues;

private:
	std::unique_ptr<xPackages> xm_pPackages;
	std::unique_ptr<xValues> xm_pValues;

private:
	NotationPackage() noexcept;
	NotationPackage(const NotationPackage &rhs);
	NotationPackage(NotationPackage &&rhs) noexcept;
	NotationPackage &operator=(const NotationPackage &rhs);
	NotationPackage &operator=(NotationPackage &&rhs) noexcept;
	~NotationPackage() noexcept;

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
	template<typename PathSegments>
	const NotationPackage *GetPackageFromPath(const PathSegments &vPathSegments) const noexcept {
		auto pCur = this;
		for(const auto &vSegment : vPathSegments){
			pCur = pCur->GetPackage(vSegment);
			if(!pCur){
				return nullptr;
			}
		}
		return pCur;
	}
	template<typename PathSegments>
	NotationPackage *GetPackageFromPath(const PathSegments &vPathSegments) noexcept {
		auto pCur = this;
		for(const auto &vSegment : vPathSegments){
			pCur = pCur->GetPackage(vSegment);
			if(!pCur){
				return nullptr;
			}
		}
		return pCur;
	}
	template<typename PathSegments>
	std::pair<NotationPackage *, bool> CreatePackageFromPath(const PathSegments &vPathSegments){
		auto vRet = std::make_pair(this, false);
		for(const auto &vSegment : vPathSegments){
			vRet = vRet.first->CreatePackage(vSegment);
		}
		return std::move(vRet);
	}

	template<typename PathSegmentIterator>
	const NotationPackage *GetPackageFromPath(
		PathSegmentIterator itSegmentBegin,
		typename std::common_type<PathSegmentIterator>::type itSegmentEnd
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
	template<typename PathSegmentIterator>
	NotationPackage *GetPackageFromPath(
		PathSegmentIterator itSegmentBegin,
		typename std::common_type<PathSegmentIterator>::type itSegmentEnd
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
	template<typename PathSegmentIterator>
	std::pair<NotationPackage *, bool> CreatePackageFromPath(
		PathSegmentIterator itSegmentBegin,
		typename std::common_type<PathSegmentIterator>::type itSegmentEnd
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
		ERR_NONE,						// 0
		ERR_NO_VALUE_NAME,				// 1
		ERR_NO_PACKAGE_NAME,			// 2
		ERR_UNEXCEPTED_PACKAGE_CLOSE,	// 3
		ERR_EQU_EXPECTED,				// 4
		ERR_UNCLOSED_PACKAGE,			// 5
		ERR_SOURCE_PACKAGE_NOT_FOUND,	// 6
		ERR_DUPLICATE_PACKAGE,			// 7
		ERR_DUPLICATE_VALUE,			// 8
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
