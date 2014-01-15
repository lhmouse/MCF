// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xXMLElementClass.hpp"
#include "xXMLParserClass.hpp"
using namespace MCF;

// 静态成员函数。
std::string XMLDocumentClass::XMLElementClass::xCreateEntities(const std::string &str){
	std::string ret;
	ret.reserve(str.size() * 2);

	for(auto iter = str.cbegin(); iter != str.cend(); ++iter){
		switch(*iter){
			case '&':	ret.append("&amp;"); break;
			case '<':	ret.append("&lt;"); break;
			case '>':	ret.append("&gt;"); break;
			case '\'':	ret.append("&apos;"); break;
			case '"':	ret.append("&quot;"); break;
			default:	ret.push_back(*iter); break;
		}
	}

	return std::move(ret);
}
const char *XMLDocumentClass::XMLElementClass::xRemoveEntities(std::string &strRet, const char *pBegin, const char *pEnd){
	strRet.clear();
	strRet.reserve((std::size_t)(pEnd - pBegin));

	const char *pRead = pBegin, *pAndPos;
	while((pAndPos = (const char *)std::memchr(pRead, '&', (std::size_t)(pEnd - pRead))) != nullptr){
		strRet.append(pRead, pAndPos);
		pRead = pAndPos;

		if(std::strncmp(pAndPos, "&amp;", 5) == 0){
			strRet.push_back('&');
			pRead += 5;
		} else if(std::strncmp(pAndPos, "&lt;", 4) == 0){
			strRet.push_back('<');
			pRead += 4;
		} else if(std::strncmp(pAndPos, "&gt;", 4) == 0){
			strRet.push_back('>');
			pRead += 4;
		} else if(std::strncmp(pAndPos, "&apos;", 6) == 0){
			strRet.push_back('\'');
			pRead += 6;
		} else if(std::strncmp(pAndPos, "&quot;", 6) == 0){
			strRet.push_back('"');
			pRead += 6;
		} else {
			return pAndPos;
		}
	}
	strRet.append(pRead, pEnd);
	return nullptr;
}

// 构造函数和析构函数。
XMLDocumentClass::XMLElementClass::XMLElementClass(XMLDocumentClass::XMLElementClass *pParent, const std::string &strKey){
	xm_pParent = nullptr;
	xm_uIndex = 0;

	xAttach(pParent, std::string(strKey));
}
XMLDocumentClass::XMLElementClass::XMLElementClass(XMLDocumentClass::XMLElementClass *pParent, std::string &&strKey){
	xm_pParent = nullptr;
	xm_uIndex = 0;

	xAttach(pParent, std::move(strKey));
}
XMLDocumentClass::XMLElementClass::XMLElementClass(XMLDocumentClass::XMLElementClass &&src) :
	xm_pParent			(std::move(src.xm_pParent)),
	xm_strKey			(std::move(src.xm_strKey)),
	xm_mapAttributes	(std::move(src.xm_mapAttributes)),
	xm_strCData			(std::move(src.xm_strCData)),
	xm_mapChildren		(std::move(src.xm_mapChildren))
{
	if(xm_pParent != nullptr){
		xm_uIndex = src.xm_uIndex;
		src.xm_pParent = nullptr;
		xm_pParent->xm_mapChildren[xm_strKey][xm_uIndex] = this;
	}
	for(auto i = xm_mapChildren.cbegin(); i != xm_mapChildren.cend(); ++i){
		for(auto j = i->second.cbegin(); j != i->second.cend(); ++j){
			(*j)->xm_pParent = this;
		}
	}
}
XMLDocumentClass::XMLElementClass &XMLDocumentClass::XMLElementClass::operator=(XMLDocumentClass::XMLElementClass &&src){
	this->~XMLElementClass();
	return *(new(this) XMLElementClass(std::move(src)));
}
XMLDocumentClass::XMLElementClass::~XMLElementClass(){
	xDetach();
	RemoveAllChildren();
}

// 其他非静态成员函数。
void XMLDocumentClass::XMLElementClass::xAttach(XMLDocumentClass::XMLElementClass *pParent, std::string &&strKey){
	xDetach();

	xm_pParent = pParent;
	xm_strKey = std::move(strKey);

	if(xm_pParent == nullptr){
		return;
	}

	std::vector<XMLElementClass *> &vecSiblings = xm_pParent->xm_mapChildren[xm_strKey];
	xm_uIndex = vecSiblings.size();
	vecSiblings.emplace_back(this);
}
void XMLDocumentClass::XMLElementClass::xDetach(){
	if(xm_pParent == nullptr){
		return;
	}

	auto iter = xm_pParent->xm_mapChildren.find(xm_strKey);
	for(auto j = iter->second.erase(iter->second.cbegin() + (std::ptrdiff_t)xm_uIndex); j != iter->second.cend(); ++j){
		--((*j)->xm_uIndex);
	}
	if(iter->second.empty()){
		xm_pParent->xm_mapChildren.erase(iter);
	}

	xm_pParent	= nullptr;
	xm_strKey.clear();
}

void XMLDocumentClass::XMLElementClass::xExportRecur(std::string &strAppendedTo) const {
	strAppendedTo += '<';
	strAppendedTo += xm_strKey;

	for(auto i = xm_mapAttributes.cbegin(); i != xm_mapAttributes.cend(); ++i){
		strAppendedTo += ' ';
		strAppendedTo += i->first;
		strAppendedTo += '=';
		strAppendedTo += '"';
		strAppendedTo += xCreateEntities(i->second);
		strAppendedTo += '"';
	}

	if(xm_mapChildren.empty()){
		if(xm_strCData.empty()){
			strAppendedTo += '/';
			strAppendedTo += '>';
		} else {
			strAppendedTo += '>';
			strAppendedTo += xCreateEntities(xm_strCData);
			strAppendedTo += '<';
			strAppendedTo += '/';
			strAppendedTo += xm_strKey;
			strAppendedTo += '>';
		}
	} else {
		strAppendedTo += '>';

		for(auto i = xm_mapChildren.cbegin(); i != xm_mapChildren.cend(); ++i){
			for(auto j = i->second.cbegin(); j != i->second.cend(); ++j){
				(*j)->xExportRecur(strAppendedTo);
			}
		}

		strAppendedTo += '<';
		strAppendedTo += '/';
		strAppendedTo += xm_strKey;
		strAppendedTo += '>';
	}
}
void XMLDocumentClass::XMLElementClass::xExportFRecur(std::string &strAppendedTo, std::string &strPrefix) const {
	strAppendedTo += strPrefix;
	strAppendedTo += '<';
	strAppendedTo += xm_strKey;

	for(auto i = xm_mapAttributes.cbegin(); i != xm_mapAttributes.cend(); ++i){
		strAppendedTo += ' ';
		strAppendedTo += i->first;
		strAppendedTo += '=';
		strAppendedTo += '"';
		strAppendedTo += xCreateEntities(i->second);
		strAppendedTo += '"';
	}

	if(xm_mapChildren.empty()){
		if(xm_strCData.empty()){
			strAppendedTo += '/';
			strAppendedTo += '>';
			strAppendedTo += '\n';
		} else {
			strAppendedTo += '>';
			strAppendedTo += xCreateEntities(xm_strCData);
			strAppendedTo += '<';
			strAppendedTo += '/';
			strAppendedTo += xm_strKey;
			strAppendedTo += '>';
			strAppendedTo += '\n';
		}
	} else {
		strAppendedTo += '>';
		strAppendedTo += '\n';

		strPrefix.push_back('\t');
		for(auto i = xm_mapChildren.cbegin(); i != xm_mapChildren.cend(); ++i){
			for(auto j = i->second.cbegin(); j != i->second.cend(); ++j){
				(*j)->xExportFRecur(strAppendedTo, strPrefix);
			}
		}
		strPrefix.pop_back();

		strAppendedTo += strPrefix;
		strAppendedTo += '<';
		strAppendedTo += '/';
		strAppendedTo += xm_strKey;
		strAppendedTo += '>';
		strAppendedTo += '\n';
	}
}

const XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::GetParent() const {
	return xm_pParent;
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::GetParent(){
	return xm_pParent;
}

const std::string &XMLDocumentClass::XMLElementClass::GetKey() const {
	return xm_strKey;
}
void XMLDocumentClass::XMLElementClass::SetKey(const std::string &strNewKey){
	if(xm_strKey != strNewKey){
		XMLElementClass *const pParent = xm_pParent;

		xDetach();
		xAttach(pParent, std::string(strNewKey));
	}
}
void XMLDocumentClass::XMLElementClass::SetKey(std::string &&strNewKey){
	if(xm_strKey != strNewKey){
		XMLElementClass *const pParent = xm_pParent;

		xDetach();
		xAttach(pParent, std::move(strNewKey));
	}
}

std::size_t XMLDocumentClass::XMLElementClass::GetIndex() const {
	return xm_uIndex;
}

const std::string *XMLDocumentClass::XMLElementClass::GetAttribute(const std::string &strName) const {
	const auto iter = xm_mapAttributes.find(strName);
	if(iter != xm_mapAttributes.end()){
		return &iter->second;
	} else {
		return nullptr;
	}
}
bool XMLDocumentClass::XMLElementClass::SetAttribute(const std::string &strName, const std::string &strValue){
	if(strName.empty() || !xXMLParserClass::CanBeginIDs(*strName.begin())){
		return false;
	}
	for(auto iter = strName.cbegin() + 1; iter != strName.cend(); ++iter){
		if(!xXMLParserClass::IsLegalInIDs(*iter)){
			return false;
		}
	}
	xm_mapAttributes[strName] = strValue;
	return true;
}
bool XMLDocumentClass::XMLElementClass::SetAttribute(const std::string &strName, std::string &&strValue){
	if(strName.empty() || !xXMLParserClass::CanBeginIDs(*strName.begin())){
		return false;
	}
	for(auto iter = strName.cbegin() + 1; iter != strName.cend(); ++iter){
		if(!xXMLParserClass::IsLegalInIDs(*iter)){
			return false;
		}
	}
	xm_mapAttributes[strName] = std::move(strValue);
	return true;
}
void XMLDocumentClass::XMLElementClass::RemoveAttribute(const std::string &strName){
	xm_mapAttributes.erase(strName);
}
void XMLDocumentClass::XMLElementClass::RemoveAllAttributes(){
	xm_mapAttributes.clear();
}

const std::string &XMLDocumentClass::XMLElementClass::GetCData() const {
	return xm_strCData;
}
std::string &XMLDocumentClass::XMLElementClass::GetCData(){
	return xm_strCData;
}
void XMLDocumentClass::XMLElementClass::SetCData(const std::string &strCData){
	xm_strCData = strCData;
}
void XMLDocumentClass::XMLElementClass::SetCData(std::string &&strCData){
	xm_strCData = std::move(strCData);
}

const XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::GetChild(const std::string &strKey, std::size_t uIndex) const {
	const auto iter = xm_mapChildren.find(strKey);
	if(iter != xm_mapChildren.end()){
		return iter->second.at(uIndex);
	} else {
		return nullptr;
	}
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::GetChild(const std::string &strKey, std::size_t uIndex){
	const auto iter = xm_mapChildren.find(strKey);
	if(iter != xm_mapChildren.end()){
		return iter->second.at(uIndex);
	} else {
		return nullptr;
	}
}
std::size_t XMLDocumentClass::XMLElementClass::GetCountOfChildrenWithKey(const std::string &strKey) const {
	const auto iter = xm_mapChildren.find(strKey);
	if(iter != xm_mapChildren.end()){
		return iter->second.size();
	} else {
		return 0;
	}
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::AppendChild(const std::string &strKey){
	return new XMLElementClass(this, strKey);
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::AppendChild(std::string &&strKey){
	return new XMLElementClass(this, strKey);
}
void XMLDocumentClass::XMLElementClass::RemoveChild(const std::string &strKey, std::size_t uIndex){
	const auto iter = xm_mapChildren.find(strKey);
	if(iter != xm_mapChildren.end()){
		delete iter->second.at(uIndex);
	}
}
void XMLDocumentClass::XMLElementClass::RemoveChildrenWithKey(const std::string &strKey){
	const auto iter = xm_mapChildren.find(strKey);
	if(iter != xm_mapChildren.end()){
		const auto &vecChildrenWithThisKey = iter->second;

		// *** 警告 ***
		// 一旦 vector 中的最后一个元素删除，vector 本身就会被 xDetach() 删除，不能继续调用 empty() 方法。
		while(vecChildrenWithThisKey.size() > 1){
			delete vecChildrenWithThisKey.back();
		}
		delete vecChildrenWithThisKey.back();
	}
}
void XMLDocumentClass::XMLElementClass::RemoveAllChildren(){
	while(!xm_mapChildren.empty()){
		const auto &vecChildrenWithThisKey = xm_mapChildren.begin()->second;

		// 参考 RemoveChildrenWithKey() 中的注释。
		while(vecChildrenWithThisKey.size() > 1){
			delete vecChildrenWithThisKey.back();
		}
		delete vecChildrenWithThisKey.back();
	}
}

XMLDocumentClass::XMLElementClass XMLDocumentClass::XMLElementClass::CloneTree() const {
	XMLElementClass ret(nullptr, xm_strKey);

	ret.xm_mapAttributes = xm_mapAttributes;
	ret.xm_strCData = xm_strCData;

	for(auto iter = xm_mapChildren.cbegin(); iter != xm_mapChildren.cend(); ++iter){
		std::vector<XMLElementClass *> &vecClonedChildrenWithThisKey = ret.xm_mapChildren[iter->first];
		vecClonedChildrenWithThisKey.reserve(iter->second.size());
		for(auto j = iter->second.cbegin(); j != iter->second.cend(); ++j){
			(new XMLElementClass(std::move((*j)->CloneTree())))->xAttach(&ret, std::string(iter->first));
		}
	}

	return std::move(ret);
}

const XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::PathQuery(const char *pszPath) const {
	if((pszPath == nullptr) || (pszPath[0] == 0)){
		return this;
	}

	const char *const pchSlashPos = std::strchr(pszPath, '/');
	std::size_t uTokenLen;
	if(pchSlashPos == nullptr){
		uTokenLen = std::strlen(pszPath);
	} else {
		uTokenLen = (std::size_t)(pchSlashPos - pszPath);
	}

	const XMLElementClass *pFoundElement = nullptr;

	std::string strKey(pszPath, uTokenLen);
	if(strKey.empty()){
		pFoundElement = this;
		if(pchSlashPos != nullptr){
			while(pFoundElement->xm_pParent != nullptr){
				pFoundElement = pFoundElement->xm_pParent;
			}
		}
	} else if(strKey == "."){
		pFoundElement = this;
	} else if(strKey == ".."){
		pFoundElement = (xm_pParent == nullptr) ? this : xm_pParent;
	} else {
		unsigned long ulIndex = 0;

		const std::size_t uLeftBracketPos = strKey.find_first_of('[');
		const std::size_t uRightBracketPos = strKey.find_last_of(']');
		if((uLeftBracketPos != std::string::npos) && (uRightBracketPos != std::string::npos) && (uLeftBracketPos < uRightBracketPos)){
			strKey[uRightBracketPos] = 0;
			ulIndex = std::strtoul(strKey.data() + uLeftBracketPos + 1, nullptr, 10);
		}

		if(uLeftBracketPos != std::string::npos){
			strKey[uLeftBracketPos] = 0;
		}
		if(uRightBracketPos != std::string::npos){
			strKey[uRightBracketPos] = 0;
		}
		strKey.resize(std::strlen(strKey.data()));

		pFoundElement = GetChild(strKey, ulIndex);
	}

	if(pchSlashPos == nullptr){
		return pFoundElement;
	} else if(pFoundElement == nullptr){
		return nullptr;
	} else {
		const char *pszNextToken = pchSlashPos;
		while(*pszNextToken == '/'){
			++pszNextToken;
		}
		return pFoundElement->PathQuery(pszNextToken);
	}
}
XMLDocumentClass::XMLElementClass *XMLDocumentClass::XMLElementClass::PathQuery(const char *pszPath){
	if((pszPath == nullptr) || (pszPath[0] == 0)){
		return this;
	}

	const char *const pchSlashPos = std::strchr(pszPath, '/');
	std::size_t uTokenLen;
	if(pchSlashPos == nullptr){
		uTokenLen = std::strlen(pszPath);
	} else {
		uTokenLen = (std::size_t)(pchSlashPos - pszPath);
	}

	XMLElementClass *pFoundElement = nullptr;

	std::string strKey(pszPath, uTokenLen);
	if(strKey.empty()){
		pFoundElement = this;
		if(pchSlashPos != nullptr){
			while(pFoundElement->xm_pParent != nullptr){
				pFoundElement = pFoundElement->xm_pParent;
			}
		}
	} else if(strKey == "."){
		pFoundElement = this;
	} else if(strKey == ".."){
		pFoundElement = (xm_pParent == nullptr) ? this : xm_pParent;
	} else {
		unsigned long ulIndex = 0;

		const std::size_t uLeftBracketPos = strKey.find_first_of('[');
		const std::size_t uRightBracketPos = strKey.find_last_of(']');
		if((uLeftBracketPos != std::string::npos) && (uRightBracketPos != std::string::npos) && (uLeftBracketPos < uRightBracketPos)){
			strKey[uRightBracketPos] = 0;
			ulIndex = std::strtoul(strKey.data() + uLeftBracketPos + 1, nullptr, 10);
		}

		if(uLeftBracketPos != std::string::npos){
			strKey[uLeftBracketPos] = 0;
		}
		if(uRightBracketPos != std::string::npos){
			strKey[uRightBracketPos] = 0;
		}
		strKey.resize(std::strlen(strKey.data()));

		pFoundElement = GetChild(strKey, ulIndex);
	}

	if(pFoundElement == nullptr){
		return nullptr;
	} else {
		if(pchSlashPos == nullptr){
			return pFoundElement;
		} else {
			const char *pszNextToken = pchSlashPos;
			while(*pszNextToken == '/'){
				++pszNextToken;
			}
			return pFoundElement->PathQuery(pszNextToken);
		}
	}
}

const char *XMLDocumentClass::XMLElementClass::Parse(const char *pszXMLString){
	xXMLParserClass XMLParser;

	const char *const pErrorPos = XMLParser.Parse(pszXMLString);
	if(pErrorPos == nullptr){
		*this = std::move(*XMLParser.GetMovableRootElement());
	}
	return pErrorPos;
}
std::string XMLDocumentClass::XMLElementClass::Export(bool bToFormat) const {
	std::string ret;
	if(bToFormat){
		std::string strPrefix;
		xExportFRecur(ret, strPrefix);
	} else {
		xExportRecur(ret);
	}
	return std::move(ret);
}
