// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_NOTATION_HPP_
#define MCF_NOTATION_HPP_

#include "../Core/String.hpp"
#include "../Core/MultiIndexedMap.hpp"

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
	bool RemovePackage(const WideStringObserver &wsoName) noexcept;
	void ClearPackages() noexcept;

	const WideString *GetValue(const WideStringObserver &wsoName) const noexcept;
	WideString *GetValue(const WideStringObserver &wsoName) noexcept;
	WideString *CreteValue(WideString wcsName);
	bool RemoveValue(const WideStringObserver &wsoName) noexcept;
	void ClearValues() noexcept;

	void Clear() noexcept;
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
		ERR_DUPLICATE_PACKAGE,
		ERR_DUPLICATE_VALUE
	} ErrorType;

	typedef NotationPackage Package;

public:
	Notation() noexcept = default;

public:
	std::pair<ErrorType, const wchar_t *> Parse(const WideStringObserver &wsoData);
	WideString Export(const WideStringObserver &wsoIndent = L"  "_wso) const;
};

}

#endif
