// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NOTATION_HPP__
#define __MCF_NOTATION_HPP__

#include "../Core/String.hpp"
#include "../../MCFCRT/cpp/ext/multi_indexed_map.hpp"
#include <utility>
#include <initializer_list>

namespace MCF {

class Notation;

class Package {
	friend class Notation;

private:
	MultiIndexedMap<Package, Utf16String> xm_mapPackages;
	MultiIndexedMap<Utf16String, Utf16String> xm_mapValues;

private:
	Package() = default;

public:
	template<typename Name_t>
	const Package *GetPackage(const Name_t &vName) const noexcept {
		const auto pNode = xm_mapPackages.Find<0>(vName);
		if(!pNode){
			return nullptr;
		}
		return &(pNode->GetElement());
	}
	template<typename Name_t>
	Package *GetPackage(const Name_t &vName) noexcept {
		const auto pNode = xm_mapPackages.Find<0>(vName);
		if(!pNode){
			return nullptr;
		}
		return &(pNode->GetElement());
	}
	template<typename Name_t>
	Package *CreatePackage(Name_t &&vName){
		auto pNode = xm_mapPackages.Find<0>(vName);
		if(!pNode){
			pNode = xm_mapPackages.Insert(Package(), std::forward<Name_t>(vName));
		}
		return &(pNode->GetElement());
	}

	template<typename Name_t>
	const Utf16String *GetValue(const Name_t &vName) const noexcept {
		const auto pNode = xm_mapValues.Find<0>(vName);
		if(!pNode){
			return nullptr;
		}
		return &(pNode->GetElement());
	}
	template<typename Name_t>
	Utf16String *GetValue(const Name_t &vName) noexcept {
		const auto pNode = xm_mapValues.Find<0>(vName);
		if(!pNode){
			return nullptr;
		}
		return &(pNode->GetElement());
	}
	template<typename Name_t, typename InitValue_t>
	Utf16String *CreateValue(Name_t &&vName, InitValue_t &&vInitValue = Utf16String()){
		auto pNode = xm_mapValues.Find<0>(vName);
		if(!pNode){
			pNode = xm_mapValues.Insert(std::forward<InitValue_t>(vInitValue), std::forward<Name_t>(vName));
		}
		return &(pNode->GetElement());
	}

	template<class PathSegmentIterator_t>
	const Package *GetPackage(PathSegmentIterator_t itSegBegin, PathSegmentIterator_t itSegEnd) const noexcept {
		auto ppkgCur = this;
		for(auto it = itSegBegin; it != itSegEnd; ++it){
			if(!(ppkgCur = ppkgCur->GetPackage(*it))){
				return nullptr;
			}
		}
		return ppkgCur;
	}
	template<class PathSegmentIterator_t>
	Package *GetPackage(PathSegmentIterator_t itSegBegin, PathSegmentIterator_t itSegEnd) noexcept {
		auto ppkgCur = this;
		for(auto it = itSegBegin; it != itSegEnd; ++it){
			if(!(ppkgCur = ppkgCur->GetPackage(*it))){
				return nullptr;
			}
		}
		return ppkgCur;
	}
	template<class PathSegmentIterator_t>
	Package *CreatePackage(PathSegmentIterator_t itSegBegin, PathSegmentIterator_t itSegEnd){
		auto ppkgCur = this;
		for(auto it = itSegBegin; it != itSegEnd; ++it){
			ppkgCur = ppkgCur->CreatePackage(*it);
		}
		return ppkgCur;
	}

	template<class PathSegmentIterator_t>
	const Utf16String *GetValue(PathSegmentIterator_t itSegBegin, PathSegmentIterator_t itSegEnd) const noexcept {
		if(itSegBegin == itSegEnd){
			return nullptr;
		}
		auto itName = itSegEnd;
		--itName;
		const auto ppkgParent = GetPackage(itSegBegin, itName);
		if(!ppkgParent){
			return nullptr;
		}
		return ppkgParent->GetValue(*itName);
	}
	template<class PathSegmentIterator_t>
	Utf16String *GetValue(PathSegmentIterator_t itSegBegin, PathSegmentIterator_t itSegEnd) noexcept {
		if(itSegBegin == itSegEnd){
			return nullptr;
		}
		auto itName = itSegEnd;
		--itName;
		const auto ppkgParent = GetPackage(itSegBegin, itName);
		if(!ppkgParent){
			return nullptr;
		}
		return ppkgParent->GetValue(*itName);
	}
	template<class PathSegmentIterator_t, typename InitValue_t>
	Utf16String *CreateValue(PathSegmentIterator_t itSegBegin, PathSegmentIterator_t itSegEnd, InitValue_t &&vInitValue = Utf16String()){
		if(itSegBegin == itSegEnd){
			return nullptr;
		}
		auto itName = itSegEnd;
		--itName;
		const auto ppkgParent = CreatePackage(itSegBegin, itName);
		return ppkgParent->CreateValue(*itName, std::forward<InitValue_t>(vInitValue));
	}

	template<typename Name_t>
	const Package *GetPackage(std::initializer_list<Name_t> ilPath) const noexcept {
		return GetPackage(ilPath.begin(), ilPath.end());
	}
	template<typename Name_t>
	Package *GetPackage(std::initializer_list<Name_t> ilPath) noexcept {
		return GetPackage(ilPath.begin(), ilPath.end());
	}
	template<typename Name_t>
	Package *CreatePackage(std::initializer_list<Name_t> ilPath){
		return CreatePackage(ilPath.begin(), ilPath.end());
	}

	template<typename Name_t>
	const Utf16String *GetValue(std::initializer_list<Name_t> ilPath) const noexcept {
		return GetValue(ilPath.begin(), ilPath.end());
	}
	template<typename Name_t>
	Utf16String *GetValue(std::initializer_list<Name_t> ilPath) noexcept {
		return GetValue(ilPath.begin(), ilPath.end());
	}
	template<typename Name_t, typename InitValue_t>
	Utf16String *CreateValue(std::initializer_list<Name_t> ilPath, InitValue_t &&vInitValue = Utf16String()){
		return CreateValue(ilPath.begin(), ilPath.end(), std::forward<InitValue_t>(vInitValue));
	}

	void Clear() noexcept {
		xm_mapPackages.Clear();
		xm_mapValues.Clear();
	}
};

class Notation : public Package {
public:
	typedef enum {
		ERR_NONE,
		ERR_NO_VALUE_NAME,
		ERR_NO_PACKAGE_NAME,
		ERR_UNEXCEPTED_PACKAGE_CLOSE,
		ERR_EQU_EXPECTED,
		ERR_UNCLOSED_PACKAGE,
		ERR_ESCAPE_AT_EOF
	} ErrorType;

private:
	static void xEscapeAndAppend(Utf16String &wcsAppendTo, const wchar_t *pwchBegin, std::size_t uLength);
	static Utf16String xUnescapeAndConstruct(const wchar_t *pwchBegin, std::size_t uLength);

	static void xExportPackageRecur(Utf16String &wcsAppendTo, const Package &pkgWhich, Utf16String &wcsPrefix, const wchar_t *pwchIndent, std::size_t uIndentLen);

public:
	Notation() noexcept = default;
	explicit Notation(const wchar_t *pwchText, std::size_t uLen = (std::size_t)-1);

public:
	std::pair<ErrorType, const wchar_t *> Parse(const wchar_t *pwchText, std::size_t uLen = (std::size_t)-1);
	Utf16String Export(const wchar_t *pwchIndent = L"\t") const;
};

}

#endif
