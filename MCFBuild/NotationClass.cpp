// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include "NotationClass.hpp"
#include <list>
#include <stack>
#include <vector>
#include <cstring>
#include <cassert>
using namespace MCF;

// 静态成员函数。
char *__attribute__((noinline)) NotationClass::xStrTokS(char *s, char ch, char **ctx){
	char *from;
	if(s != nullptr){
		from = s;
	} else if(*ctx != nullptr){
		from = *ctx;
	} else {
		return nullptr;
	}
	for(;;){
		if(*from == 0){
			return nullptr;
		}
		if(*from != ch){
			break;
		}
		++from;
	}

	char *delim = from;
	for(;;){
		++delim;
		if(*delim == 0){
			*ctx = nullptr;
			break;
		}
		if(*delim == ch){
			*delim = 0;
			*ctx = delim + 1;
			break;
		}
	}

	return from;
}

void NotationClass::xEscapeAndAppend(std::string &strAppendTo, const std::string &strSrc){
	strAppendTo.reserve(strAppendTo.size() + strSrc.size() * 2);
	for(auto iter = strSrc.cbegin(); iter != strSrc.cend(); ++iter){
		const char ch = *iter;
		switch(ch){
		case '\\':
		case '=':
		case '{':
		case '}':
		case ';':
			strAppendTo.push_back('\\');
			strAppendTo.push_back(ch);
			break;
		case '\n':
			strAppendTo.push_back('\\');
			strAppendTo.push_back('n');
			break;
		case '\b':
			strAppendTo.push_back('\\');
			strAppendTo.push_back('b');
			break;
		case '\r':
			strAppendTo.push_back('\\');
			strAppendTo.push_back('r');
			break;
		case '\t':
			strAppendTo.push_back('\\');
			strAppendTo.push_back('t');
			break;
		default:
			strAppendTo.push_back(ch);
			break;
		}
	}
}
std::string NotationClass::xUnescapeAndConstruct(const char *pBegin, const char *pEnd){
	std::string strRet;
	strRet.reserve((std::size_t)(pEnd - pBegin));

	enum {
		NORMAL,
		SLASH_MATCH,
		HEX_WAIT_FOR_FIRST,
		HEX_WAIT_FOR_SECOND
	} eState = NORMAL;

	const auto DecodeHex = [](char ch) -> char {
		if(('0' <= ch) && (ch <= '9')){
			return ch - '0';
		} else if(('a' <= ch) && (ch <= 'f')){
			return ch - 'a' + 0x0A;
		} else if(('A' <= ch) && (ch <= 'F')){
			return ch - 'A' + 0x0A;
		}
		return -1;
	};

	char chFirstHex = 0;

	for(const char *pCur = pBegin; pCur != pEnd; ++pCur){
		const char ch = *pCur;
		switch(eState){
		case NORMAL:
			if(ch == '\\'){
				eState = SLASH_MATCH;
			} else {
				strRet.push_back(ch);
			}
			break;
		case SLASH_MATCH:
			switch(ch){
			case 'b':
				strRet.push_back('\b');
				eState = NORMAL;
				break;
			case 'n':
				strRet.push_back('\n');
				eState = NORMAL;
				break;
			case 'r':
				strRet.push_back('\r');
				eState = NORMAL;
				break;
			case 't':
				strRet.push_back('\t');
				eState = NORMAL;
				break;
			case 'x':
				eState = HEX_WAIT_FOR_FIRST;
				break;
			case '\n':
				eState = NORMAL;
				break;
			default:
				strRet.push_back(ch);
				eState = NORMAL;
				break;
			}
			break;
		case HEX_WAIT_FOR_FIRST:
			if(DecodeHex(ch) != -1){
				chFirstHex = ch;
				eState = HEX_WAIT_FOR_SECOND;
			} else {
				strRet.push_back('x');
				strRet.push_back(ch);
				eState = NORMAL;
			}
			break;
		case HEX_WAIT_FOR_SECOND:
			{
				const char chLow = DecodeHex(ch);
				if(chLow != -1){
					const char chHigh = DecodeHex(chFirstHex);
					strRet.push_back((char)(((std::uintptr_t)chHigh << 4) | (std::uintptr_t)chLow));
				} else {
					strRet.push_back('x');
					strRet.push_back(chFirstHex);
					strRet.push_back(ch);
				}
			}
			eState = NORMAL;
			break;
		}
	}
	if(eState == HEX_WAIT_FOR_SECOND){
		strRet.push_back('x');
		strRet.push_back(chFirstHex);
	}

	return std::move(strRet);
}
void NotationClass::xExportPackageRecur(std::string &strAppendTo, const NotationClass::Package &pkgWhich, const std::string &strPrefix){
	for(auto iter = pkgWhich.mapPackages.cbegin(); iter != pkgWhich.mapPackages.cend(); ++iter){
		strAppendTo += strPrefix;
		xEscapeAndAppend(strAppendTo, iter->first);
		strAppendTo += ' ';
		strAppendTo += '{';
		strAppendTo += '\n';

		xExportPackageRecur(strAppendTo, iter->second, strPrefix + '\t');

		strAppendTo += strPrefix;
		strAppendTo += '}';
		strAppendTo += '\n';
	}

	for(auto iter = pkgWhich.mapValues.cbegin(); iter != pkgWhich.mapValues.cend(); ++iter){
		strAppendTo += strPrefix;
		xEscapeAndAppend(strAppendTo, iter->first);
		strAppendTo += ' ';
		strAppendTo += '=';
		strAppendTo += ' ';
		xEscapeAndAppend(strAppendTo, iter->second);
		strAppendTo += '\n';
	}
}

// 构造函数和析构函数。
NotationClass::NotationClass(){
}
NotationClass::NotationClass(const char *pszText){
	Parse(pszText);
}
NotationClass::NotationClass(const char *pchText, std::size_t uLen){
	Parse(pchText, uLen);
}

// 其他非静态成员函数。
std::pair<NotationClass::ERROR_TYPE, const char *> NotationClass::Parse(const char *pszText){
	return Parse(pszText, std::strlen(pszText));
}
std::pair<NotationClass::ERROR_TYPE, const char *> NotationClass::Parse(const char *pchText, std::size_t uLen){
	xm_Root.Clear();

	if(uLen == 0){
		return std::make_pair(ERR_NONE, nullptr);
	}

	const char *pszRead = pchText;

	Package NewRoot;
	std::stack<Package *> stkPackages;
	stkPackages.push(&NewRoot);

	const char *pNameBegin = pszRead;
	const char *pNameEnd = pszRead;
	const char *pValueBegin = pszRead;
	const char *pValueEnd = pszRead;
	enum {
		NAME_INDENT,
		NAME_BODY,
		NAME_PADDING,
		VAL_INDENT,
		VAL_BODY,
		VAL_PADDING,
		COMMENT
	} eState = NAME_INDENT;
	bool bEscaped = false;

	const auto PushPackage = [&]() -> void {
		assert(pNameBegin != pNameEnd);

		stkPackages.push(&(stkPackages.top()->mapPackages[xUnescapeAndConstruct(pNameBegin, pNameEnd)] = Package()));

		pNameBegin = pszRead;
		pNameEnd = pszRead;
	};
	const auto PopPackage = [&]() -> void {
		assert(stkPackages.size() > 0);

		stkPackages.pop();

		pNameBegin = pszRead;
		pNameEnd = pszRead;
	};
	const auto SubmitValue = [&]() -> void {
		assert(pNameBegin != pNameEnd);

		stkPackages.top()->mapValues[xUnescapeAndConstruct(pNameBegin, pNameEnd)] = xUnescapeAndConstruct(pValueBegin, pValueEnd);

		pValueBegin = pszRead;
		pValueEnd = pszRead;
	};

	for(std::size_t i = 0; i < uLen; ++i, ++pszRead){
		const char ch = *pszRead;

		if(bEscaped){
			bEscaped = false;
		} else {
			switch(ch){
			case '\\':
				bEscaped = true;
				continue;
			case '=':
				switch(eState){
				case NAME_INDENT:
					return std::make_pair(ERR_NO_VALUE_NAME, pszRead);
				case NAME_BODY:
				case NAME_PADDING:
					eState = VAL_INDENT;
					continue;
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					break;
				case COMMENT:
					continue;
				};
				break;
			case '{':
				switch(eState){
				case NAME_INDENT:
					return std::make_pair(ERR_NO_VALUE_NAME, pszRead);
				case NAME_BODY:
				case NAME_PADDING:
					PushPackage();
					eState = NAME_INDENT;
					continue;
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					PushPackage();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case '}':
				switch(eState){
				case NAME_INDENT:
					if(stkPackages.size() == 1){
						return std::make_pair(ERR_UNEXCEPTED_PACKAGE_CLOSE, pszRead);
					}
					PopPackage();
					eState = NAME_INDENT;
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					if(stkPackages.size() == 1){
						return std::make_pair(ERR_UNEXCEPTED_PACKAGE_CLOSE, pszRead);
					}
					PopPackage();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case ';':
				switch(eState){
				case NAME_INDENT:
					eState = COMMENT;
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					eState = COMMENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case '\n':
				switch(eState){
				case NAME_INDENT:
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					eState = NAME_INDENT;
					continue;
				};
				break;
			}
		}

		if(ch != '\n'){
			switch(eState){
			case NAME_INDENT:
				if((ch == ' ') || (ch == '\t')){
					// eState = NAME_INDENT;
				} else {
					pNameBegin = pszRead;
					pNameEnd = pszRead + 1;
					eState = NAME_BODY;
				}
				continue;
			case NAME_BODY:
				if((ch == ' ') || (ch == '\t')){
					eState = NAME_PADDING;
				} else {
					pNameEnd = pszRead + 1;
					// eState = NAME_BODY;
				}
				continue;
			case NAME_PADDING:
				if((ch == ' ') || (ch == '\t')){
					// eState = NAME_PADDING;
				} else {
					pNameEnd = pszRead + 1;
					eState = NAME_BODY;
				}
				continue;
			case VAL_INDENT:
				if((ch == ' ') || (ch == '\t')){
					// eState = VAL_INDENT;
				} else {
					pValueBegin = pszRead;
					pValueEnd = pszRead + 1;
					eState = VAL_BODY;
				}
				continue;
			case VAL_BODY:
				if((ch == ' ') || (ch == '\t')){
					eState = VAL_PADDING;
				} else {
					pValueEnd = pszRead + 1;
					// eState = VAL_BODY;
				}
				continue;
			case VAL_PADDING:
				if((ch == ' ') || (ch == '\t')){
					// eState = VAL_PADDING;
				} else {
					pValueEnd = pszRead + 1;
					eState = VAL_BODY;
				}
				continue;
			case COMMENT:
				continue;
			}
		}
	}
	if(bEscaped){
		return std::make_pair(ERR_ESCAPE_AT_EOF, pszRead);
	}
	if(stkPackages.size() > 1){
		return std::make_pair(ERR_UNCLOSED_PACKAGE, pszRead);
	}
	switch(eState){
	case NAME_BODY:
	case NAME_PADDING:
		return std::make_pair(ERR_EQU_EXPECTED, pszRead);
	case VAL_INDENT:
	case VAL_BODY:
	case VAL_PADDING:
		SubmitValue();
		break;
	default:
		break;
	};

	xm_Root = std::move(NewRoot);

	return std::make_pair(ERR_NONE, nullptr);
}
std::string NotationClass::Export() const {
	std::string strRet;
	xExportPackageRecur(strRet, xm_Root, "");
	return std::move(strRet);
}

bool NotationClass::IsEmpty() const {
	return xm_Root.IsEmpty();
}
void NotationClass::Clear(){
	xm_Root.Clear();
}

const NotationClass::Package *NotationClass::OpenRoot() const {
	return &xm_Root;
}
NotationClass::Package *NotationClass::OpenRoot(){
	return &xm_Root;
}
const NotationClass::Package *NotationClass::OpenPackage(const char *pszPackagePath) const {
	const Package *pRet = OpenRoot();

	if(pszPackagePath != nullptr){
		const std::size_t uStrSize = std::strlen(pszPackagePath) + 1;
		std::vector<char> vecTempPath(pszPackagePath, pszPackagePath + (std::ptrdiff_t)uStrSize);

		char *pContext;
		char *pToken = xStrTokS(vecTempPath.data(), '/', &pContext);
		do {
			const auto iter = pRet->mapPackages.find(pToken);
			if(iter == pRet->mapPackages.end()){
				pRet = nullptr;
				break;
			}
			pRet = &iter->second;

			pToken = xStrTokS(nullptr, '/', &pContext);
		} while(pToken != nullptr);
	}

	return pRet;
}
NotationClass::Package *NotationClass::OpenPackage(const char *pszPackagePath){
	Package *pRet = OpenRoot();

	if(pszPackagePath != nullptr){
		const std::size_t uStrSize = std::strlen(pszPackagePath) + 1;
		std::vector<char> vecTempPath(pszPackagePath, pszPackagePath + (std::ptrdiff_t)uStrSize);

		char *pContext;
		char *pToken = xStrTokS(vecTempPath.data(), '/', &pContext);
		do {
			const auto iter = pRet->mapPackages.find(pToken);
			if(iter == pRet->mapPackages.end()){
				pRet = nullptr;
				break;
			}
			pRet = &iter->second;

			pToken = xStrTokS(nullptr, '/', &pContext);
		} while(pToken != nullptr);
	}

	return pRet;
}
NotationClass::Package *NotationClass::CreatePackage(const char *pszPackagePath, bool bClearExisting){
	Package *pRet = OpenRoot();

	if(pszPackagePath != nullptr){
		std::vector<char> vecTempPath;
		const std::size_t uStrSize = std::strlen(pszPackagePath) + 1;
		vecTempPath.assign(pszPackagePath, pszPackagePath + (std::ptrdiff_t)uStrSize);

		char *pContext;
		char *pToken = xStrTokS(vecTempPath.data(), '/', &pContext);
		do {
			pRet = &pRet->mapPackages[pToken];

			pToken = xStrTokS(nullptr, '/', &pContext);
		} while(pToken != nullptr);
	}

	if(bClearExisting){
		pRet->Clear();
	}

	return pRet;
}

const std::string *NotationClass::GetValue(const char *pszPackagePath, const char *pszName) const {
	const Package *const pPackage = OpenPackage(pszPackagePath);
	if(pPackage == nullptr){
		return nullptr;
	}
	const auto iter = pPackage->mapValues.find(pszName);
	if(iter == pPackage->mapValues.end()){
		return nullptr;
	}
	return &iter->second;
}
std::string *NotationClass::GetValue(const char *pszPackagePath, const char *pszName){
	Package *const pPackage = OpenPackage(pszPackagePath);
	if(pPackage == nullptr){
		return nullptr;
	}
	const auto iter = pPackage->mapValues.find(pszName);
	if(iter == pPackage->mapValues.end()){
		return nullptr;
	}
	return &iter->second;
}
std::string *NotationClass::SetValue(const char *pszPackagePath, const char *pszName, const char *pszValue){
	return SetValue(pszPackagePath, pszName, std::string(pszValue));
}
std::string *NotationClass::SetValue(const char *pszPackagePath, const char *pszName, const std::string &strValue){
	return SetValue(pszPackagePath, pszName, std::string(strValue));
}
std::string *NotationClass::SetValue(const char *pszPackagePath, const char *pszName, std::string &&strValue){
	return &(CreatePackage(pszPackagePath, false)->mapValues[pszName] = std::move(strValue));
}
