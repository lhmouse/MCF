// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "XMLDocumentClass.hpp"
#include "xXMLElementClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
XMLDocumentClass::XMLDocumentClass(){
	xm_pRootElement = nullptr;
}
XMLDocumentClass::XMLDocumentClass(const char *pszXML_RootElementKey, bool bStringIsXML){
	xm_pRootElement = nullptr;

	if(bStringIsXML){
		Parse(pszXML_RootElementKey);
	} else {
		Create(pszXML_RootElementKey);
	}
}
XMLDocumentClass::XMLDocumentClass(const XMLDocumentClass &src){
	if(src.xm_pRootElement == nullptr){
		xm_pRootElement = nullptr;
	} else {
		xm_pRootElement = new XMLElementClass(src.xm_pRootElement->CloneTree());
	}
}
XMLDocumentClass::XMLDocumentClass(XMLDocumentClass &&src){
	xm_pRootElement = src.xm_pRootElement;
	src.xm_pRootElement = nullptr;
}
XMLDocumentClass &XMLDocumentClass::operator=(const XMLDocumentClass &src){
	if(src.xm_pRootElement == nullptr){
		xm_pRootElement = nullptr;
	} else {
		xm_pRootElement = new XMLElementClass(src.xm_pRootElement->CloneTree());
	}
	return *this;
}
XMLDocumentClass &XMLDocumentClass::operator=(XMLDocumentClass &&src){
	std::swap(xm_pRootElement, src.xm_pRootElement);
	return *this;
}
XMLDocumentClass::~XMLDocumentClass(){
	delete xm_pRootElement;
}

// 其他非静态成员函数。
const char *XMLDocumentClass::Parse(const char *pszXMLString){
	Clear();
	xm_pRootElement = new XMLElementClass(nullptr, std::string());
	const char *const pchErrorPos = xm_pRootElement->Parse(pszXMLString);
	if(pchErrorPos != nullptr){
		Clear();
	}
	return pchErrorPos;
}
void XMLDocumentClass::Create(const char *pszRootElementKey){
	Clear();
	xm_pRootElement = new XMLElementClass(nullptr, std::string(pszRootElementKey));
}

bool XMLDocumentClass::IsEmpty() const {
	return xm_pRootElement == nullptr;
}
void XMLDocumentClass::Clear(){
	delete xm_pRootElement;
	xm_pRootElement = nullptr;
}

const XMLDocumentClass::XMLElementClass *XMLDocumentClass::GetRoot() const {
	return xm_pRootElement;
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::GetRoot(){
	return xm_pRootElement;
}

const XMLDocumentClass::XMLElementClass *XMLDocumentClass::PathQuery(const char *pszPath) const {
	if(xm_pRootElement == nullptr){
		return nullptr;
	}
	return xm_pRootElement->PathQuery(pszPath);
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::PathQuery(const char *pszPath){
	if(xm_pRootElement == nullptr){
		return nullptr;
	}
	return xm_pRootElement->PathQuery(pszPath);
}

std::string XMLDocumentClass::Export(bool bToFormat) const {
	if(xm_pRootElement == nullptr){
		return std::string();
	}
	return xm_pRootElement->Export(bToFormat);
}
