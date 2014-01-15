// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_XML_ELEMENT_CLASS_HPP__
#define __MCF_X_XML_ELEMENT_CLASS_HPP__

#include "StdMCF.hpp"
#include <string>
#include <map>
#include <vector>
#include "XMLDocumentClass.hpp"

namespace MCF {
	class XMLDocumentClass::XMLElementClass {
	private:
		class xXMLParserClass;
	private:
		static std::string xCreateEntities(const std::string &str);
		// 转义成功返回 nullptr，否则返回无法转义的 & 字符的位置。
		static const char *xRemoveEntities(std::string &strRet, const char *pBegin, const char *pEnd);
	private:
		XMLElementClass *xm_pParent;

		// 这些是该元素在父元素中位置的副本。
		std::string xm_strKey;
		std::size_t xm_uIndex;

		std::map<std::string, std::string> xm_mapAttributes;
		std::string xm_strCData;
		std::map<std::string, std::vector<XMLElementClass *>> xm_mapChildren;
	public:
		XMLElementClass(XMLElementClass *pParent, const std::string &strKey);
		XMLElementClass(XMLElementClass *pParent, std::string &&strKey);
		XMLElementClass(XMLElementClass &&src);
		XMLElementClass &operator=(XMLElementClass &&rhs);
		~XMLElementClass();
	private:
		// 不允许拷贝构造和赋值。参见成员函数 CloneTree()。
		XMLElementClass(const XMLElementClass &src);
		XMLElementClass &operator=(const XMLElementClass &src);
	private:
		void xAttach(XMLElementClass *pParent, std::string &&strKey);
		void xDetach();

		void xExportRecur(std::string &strAppendedTo) const;
		void xExportFRecur(std::string &strAppendedTo, std::string &strPrefix) const;
	public:
		const XMLElementClass *GetParent() const;
		XMLElementClass *GetParent();

		const std::string &GetKey() const;
		void SetKey(const std::string &strNewKey);
		void SetKey(std::string &&strNewKey);

		std::size_t GetIndex() const;

		const std::string *GetAttribute(const std::string &strName) const;
		bool SetAttribute(const std::string &strName, const std::string &strValue);
		bool SetAttribute(const std::string &strName, std::string &&strValue);
		void RemoveAttribute(const std::string &strName);
		void RemoveAllAttributes();

		const std::string &GetCData() const;
		std::string &GetCData();
		void SetCData(const std::string &strCData);
		void SetCData(std::string &&strCData);

		const XMLElementClass *GetChild(const std::string &strKey, std::size_t uIndex) const;
		XMLElementClass *GetChild(const std::string &strKey, std::size_t uIndex);
		std::size_t GetCountOfChildrenWithKey(const std::string &strKey) const;
		XMLElementClass *AppendChild(const std::string &strKey);
		XMLElementClass *AppendChild(std::string &&strKey);
		void RemoveChild(const std::string &strKey, std::size_t uIndex);
		void RemoveChildrenWithKey(const std::string &strKey);
		void RemoveAllChildren();

		// 复制当前元素和所有子元素。复制后的元素成为孤立的根元素。
		XMLElementClass CloneTree() const;

		const XMLElementClass *PathQuery(const char *pszPath) const;
		XMLElementClass *PathQuery(const char *pszPath);

		const char *Parse(const char *pszXMLString);
		std::string Export(bool bToFormat) const;
	};
}

#endif
