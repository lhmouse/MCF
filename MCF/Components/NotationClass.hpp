// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NOTATION_CLASS_HPP__
#define __MCF_NOTATION_CLASS_HPP__

#include "../MCFCRT/MCFCRT.h"
#include "../Core/String.hpp"
#include <utility>
#include <map>

namespace MCF {
	class NotationClass {
	public:
		typedef enum {
			ERR_NONE,
			ERR_NO_VALUE_NAME,
			ERR_NO_PACKAGE_NAME,
			ERR_UNEXCEPTED_PACKAGE_CLOSE,
			ERR_EQU_EXPECTED,
			ERR_UNCLOSED_PACKAGE,
			ERR_ESCAPE_AT_EOF
		} ERROR_TYPE;

		struct Package {
			std::map<WCString, Package> mapPackages;
			std::map<WCString, WCString> mapValues;

			bool IsEmpty() const {
				return mapPackages.empty() && mapValues.empty();
			}
			void Clear(){
				mapPackages.clear();
				mapValues.clear();
			}
		};
	private:
		static void xEscapeAndAppend(VVector<wchar_t> &vecAppendTo, const WCString &wcsSrc);
		static WCString xUnescapeAndConstruct(const wchar_t *pwchBegin, const wchar_t *pwchEnd);
		static void xExportPackageRecur(
			VVector<wchar_t> &vecAppendTo,
			const Package &pkgWhich,
			VVector<wchar_t> &Prefix,
			const wchar_t *pwchIndent,
			std::size_t uIndentLen
		);
	private:
		Package xm_Root;
	public:
		NotationClass();
		explicit NotationClass(const wchar_t *pwszText);
		NotationClass(const wchar_t *pwchText, std::size_t uLen);
	public:
		std::pair<ERROR_TYPE, const wchar_t *> Parse(const wchar_t *pwszText);
		std::pair<ERROR_TYPE, const wchar_t *> Parse(const wchar_t *pwchText, std::size_t uLen);
		WCString Export(const wchar_t *pwchIndent = L"\t") const;

		bool IsEmpty() const {
			return xm_Root.IsEmpty();
		}
		void Clear(){
			xm_Root.Clear();
		}

		const Package *OpenRoot() const {
			return &xm_Root;
		}
		Package *OpenRoot(){
			return &xm_Root;
		}

		const Package *OpenPackage(const wchar_t *pwszPackagePath) const;
		Package *OpenPackage(const wchar_t *pwszPackagePath);
		Package *CreatePackage(const wchar_t *pwszPackagePath, bool bClearExisting);

		const WCString *GetValue(const wchar_t *pwszPackagePath, const wchar_t *pwszName) const;
		WCString *GetValue(const wchar_t *pwszPackagePath, const wchar_t *pwszName);
		WCString *SetValue(const wchar_t *pwszPackagePath, const wchar_t *pwszName, const wchar_t *pwszValue);
		WCString *SetValue(const wchar_t *pwszPackagePath, const wchar_t *pwszName, const WCString &wcsValue);
		WCString *SetValue(const wchar_t *pwszPackagePath, const wchar_t *pwszName, WCString &&wcsValue);
	};
}

#endif
