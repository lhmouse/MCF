// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_XML_DOCUMENT_CLASS_HPP__
#define __MCF_XML_DOCUMENT_CLASS_HPP__

#include "StdMCF.hpp"
#include <string>

namespace MCF {
	class XMLDocumentClass {
	public:
		class XMLElementClass;
	private:
		XMLElementClass *xm_pRootElement;
	public:
		XMLDocumentClass();
		// bStringIsXML =
		//   true	pszXML_RootElementKey 是一串 XML 字符串。
		//			如果解析错误，构造一个空的 XML 文档对象（IsEmpty() 返回 true）；
		//   false	pszXML_RootElementKey 是根元素的键名。
		XMLDocumentClass(const char *pszXML_RootElementKey, bool bStringIsXML);
		XMLDocumentClass(const XMLDocumentClass &src);
		XMLDocumentClass(XMLDocumentClass &&src);
		XMLDocumentClass &operator=(const XMLDocumentClass &src);
		XMLDocumentClass &operator=(XMLDocumentClass &&src);
		~XMLDocumentClass();
	public:
		// 返回 nullptr 则成功，否则返回指向错误字符位置的指针。
		const char *Parse(const char *pszXMLString);
		void Create(const char *pszRootElementKey);

		bool IsEmpty() const;
		void Clear();

		const XMLElementClass *GetRoot() const;
		XMLElementClass *GetRoot();

		const XMLElementClass *PathQuery(const char *pszPath) const;
		XMLElementClass *PathQuery(const char *pszPath);

		// 任何有子节点的节点的 CData 将丢失；
		// 若 bToFormat = true，生成的字符串以 LF 换行、TAB 缩进。
		std::string Export(bool bToFormat = true) const;
	};

	typedef XMLDocumentClass::XMLElementClass XMLElementClass;
}

#endif
