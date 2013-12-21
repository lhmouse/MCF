// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NOTATION_CLASS_HPP__
#define __MCF_NOTATION_CLASS_HPP__

#include "../StdMCF.hpp"

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
			std::map<MCF::WCString, Package> mapPackages;
			std::map<MCF::WCString, MCF::WCString> mapValues;

			bool IsEmpty() const {
				return mapPackages.empty() && mapValues.empty();
			}
			void Clear(){
				mapPackages.clear();
				mapValues.clear();
			}
		};
	private:
		static void xEscapeAndAppend(std::string &strAppendTo, const std::string &strSrc);
		static std::string xUnescapeAndConstruct(const char *pBegin, const char *pEnd);
		static void xExportPackageRecur(std::string &strAppendTo, const Package &pkgWhich, const std::string &strPrefix);
	private:
		Package xm_Root;
	public:
		NotationClass();
		explicit NotationClass(const char *pszText);
		NotationClass(const char *pchText, std::size_t uLen);
	public:
		std::pair<ERROR_TYPE, const char *> Parse(const char *pszText);
		std::pair<ERROR_TYPE, const char *> Parse(const char *pchText, std::size_t uLen);
		std::string Export() const;

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

		const Package *OpenPackage(const char *pszPackagePath) const;
		Package *OpenPackage(const char *pszPackagePath);
		Package *CreatePackage(const char *pszPackagePath, bool bClearExisting);

		const std::string *GetValue(const char *pszPackagePath, const char *pszName) const;
		std::string *GetValue(const char *pszPackagePath, const char *pszName);
		std::string *SetValue(const char *pszPackagePath, const char *pszName, const char *pszValue);
		std::string *SetValue(const char *pszPackagePath, const char *pszName, const std::string &strValue);
		std::string *SetValue(const char *pszPackagePath, const char *pszName, std::string &&strValue);
	};
}

#endif
