// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NOTATION_HPP__
#define __MCF_NOTATION_HPP__

#include "../Core/String.hpp"
#include "../Core/Utilities.hpp"
#include "../../MCFCRT/cpp/ext/multi_indexed_map.hpp"
#include <utility>
#include <initializer_list>

namespace MCF {

class NotationPackage {
	friend class Notation;

private:
	MultiIndexedMap<NotationPackage, WideString> xm_mapPackages;
	MultiIndexedMap<WideString, WideString> xm_mapValues;

private:
	NotationPackage() = default;

public:
	const NotationPackage *GetPackage(const WideStringObserver &wsoName) const noexcept;
	NotationPackage *GetPackage(const WideStringObserver &wsoName) noexcept;
	NotationPackage *CreatePackage(WideString wcsName);

	const WideString *GetValue(const WideStringObserver &wsoName) const noexcept;
	WideString *GetValue(const WideStringObserver &wsoName) noexcept;
	WideString *CreteValue(WideString wcsName);

	void Clear() noexcept;

	template<class... PathSegs_t>
	const NotationPackage *GetPackageFromPath(const PathSegs_t &... vPathSegs) const noexcept {
		auto *pRet = this;
		CallOnEach(
			[&](const WideStringObserver &wsoName) noexcept {
				if(pRet){
					pRet = pRet->GetPackage(wsoName);
				}
			},
			WideStringObserver(vPathSegs)...
		);
		return pRet;
	}
	template<class... PathSegs_t>
	NotationPackage *GetPackageFromPath(const PathSegs_t &... vPathSegs) noexcept {
		auto *pRet = this;
		CallOnEach(
			[&](const WideStringObserver &wsoName) noexcept {
				if(pRet){
					pRet = pRet->GetPackage(wsoName);
				}
			},
			WideStringObserver(vPathSegs)...
		);
		return pRet;
	}
	template<class... PathSegs_t>
	NotationPackage *CreatePackageFromPath(PathSegs_t &&... vPathSegs) noexcept {
		auto *pRet = this;
		return CallOnEach(
			[&](WideString wcsName) noexcept {
				if(pRet){
					pRet = pRet->CreatePackage(std::move(wcsName));
				}
			},
			WideString(std::forward<PathSegs_t>(vPathSegs))...
		);
		return pRet;
	}
};

class Notation : public NotationPackage {
public:
	typedef enum {
		ERR_NONE,
		ERR_NO_VALUE_NAME,
		ERR_NO_PACKAGE_NAME,
		ERR_UNEXCEPTED_PACKAGE_CLOSE,
		ERR_EQU_EXPECTED,
		ERR_UNCLOSED_PACKAGE,
		ERR_ESCAPE_AT_EOF,
		ERR_DUPLICATE_PACKAGE,
		ERR_DUPLICATE_VALUE
	} ErrorType;

	typedef NotationPackage Package;

public:
	Notation() noexcept = default;

public:
	std::pair<ErrorType, WideStringObserver::Iterator> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  ") const;
};

}

#endif
