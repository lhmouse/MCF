// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "NotationClass.hpp"
#include <cassert>
#include <cwchar>
using namespace MCF;

void NotationClass::xEscapeAndAppend(VVector<wchar_t> &vecAppendTo, const WCString &wcsSrc){
	const auto uSrcLen = wcsSrc.GetLength();
	vecAppendTo.Reserve(vecAppendTo.GetSize() + uSrcLen * 2);
	auto pwchRead = wcsSrc.GetCStr();
	for(std::size_t i = 0; i < uSrcLen; ++i){
		const wchar_t ch = *(pwchRead++);
		switch(ch){
		case L'\\':
		case L'=':
		case L'{':
		case L'}':
		case L';':
			vecAppendTo.Push(L'\\');
			vecAppendTo.Push(ch);
			break;
		case L'\n':
			vecAppendTo.Push(L'\\');
			vecAppendTo.Push(L'n');
			break;
		case L'\b':
			vecAppendTo.Push(L'\\');
			vecAppendTo.Push(L'b');
			break;
		case L'\r':
			vecAppendTo.Push(L'\\');
			vecAppendTo.Push(L'r');
			break;
		case L'\t':
			vecAppendTo.Push(L'\\');
			vecAppendTo.Push(L't');
			break;
		default:
			vecAppendTo.Push(ch);
			break;
		}
	}
}
WCString NotationClass::xUnescapeAndConstruct(const wchar_t *pBegin, const wchar_t *pEnd){
	WCString wcsRet;
	wcsRet.Reserve((std::size_t)(pEnd - pBegin + 1));
	auto pwchWrite = wcsRet.GetCStr();

	enum STATE {
		NORMAL,
		SLASH_MATCH,
		HEX_WAIT_FOR_NEXT
	} eState = NORMAL;

	int nDecodedDigit;
	wchar_t chDecoded = 0;
	wchar_t awchHexBuffer[sizeof(wchar_t) * CHAR_BIT / 4 - 1];
	std::size_t uBufferIndex = 0;

	for(const wchar_t *pCur = pBegin; pCur != pEnd; ++pCur){
		const wchar_t ch = *pCur;
		switch(eState){
		case NORMAL:
			if(ch == L'\\'){
				eState = SLASH_MATCH;
			} else {
				*(pwchWrite++) = ch;
			}
			break;
		case SLASH_MATCH:
			switch(ch){
			case L'b':
				*(pwchWrite++) = L'\b';
				eState = NORMAL;
				break;
			case L'n':
				*(pwchWrite++) = L'\n';
				eState = NORMAL;
				break;
			case L'r':
				*(pwchWrite++) = L'\r';
				eState = NORMAL;
				break;
			case L't':
				*(pwchWrite++) = L'\t';
				eState = NORMAL;
				break;
			case L'x':
				uBufferIndex = 0;
				eState = HEX_WAIT_FOR_NEXT;
				break;
			case L'\n':
				eState = NORMAL;
				break;
			default:
				*(pwchWrite++) = ch;
				eState = NORMAL;
				break;
			}
			break;
		case HEX_WAIT_FOR_NEXT:
			if((L'0' <= ch) && (ch <= L'9')){
				nDecodedDigit = ch - L'0';
			} else if((L'a' <= ch) && (ch <= L'f')){
				nDecodedDigit = ch - L'a' + 0x0A;
			} else if((L'A' <= ch) && (ch <= L'F')){
				nDecodedDigit = ch - L'A' + 0x0A;
			} else {
				*(pwchWrite++) = L'x';
				for(std::size_t i = 0; i < uBufferIndex; ++i){
					*(pwchWrite++) = awchHexBuffer[i];
				}
				eState = NORMAL;
				break;
			}
			if(uBufferIndex == COUNT_OF(awchHexBuffer)){
				*(pwchWrite++) = (chDecoded << 4) | nDecodedDigit;
				eState = NORMAL;
			} else {
				awchHexBuffer[uBufferIndex++] = ch;
				chDecoded = (chDecoded << 4) | nDecodedDigit;
			}
			break;
		}
	}
	if(eState == HEX_WAIT_FOR_NEXT){
		*(pwchWrite++) = L'x';
		for(std::size_t i = 0; i < uBufferIndex; ++i){
			*(pwchWrite++) = awchHexBuffer[i];
		}
	}
	*pwchWrite = 0;

	return std::move(wcsRet);
}
void NotationClass::xExportPackageRecur(
	VVector<wchar_t> &vecAppendTo,
	const NotationClass::Package &pkgWhich,
	VVector<wchar_t> &vecPrefix,
	const wchar_t *pwchIndent,
	std::size_t uIndentLen
){
	const auto uCurrentPrefixLen = vecPrefix.GetSize();
	auto pwchCurrentPrefix = vecPrefix.GetData();

	if(!pkgWhich.mapPackages.empty()){
		if(pwchIndent != nullptr){
			vecPrefix.CopyToEnd(pwchIndent, uIndentLen);
		}
		pwchCurrentPrefix = vecPrefix.GetData();

		for(const auto &SubPackageItem : pkgWhich.mapPackages){
			vecAppendTo.CopyToEnd(pwchCurrentPrefix, uCurrentPrefixLen);
			xEscapeAndAppend(vecAppendTo, SubPackageItem.first);
			vecAppendTo.CopyToEnd(L" {\n", 3);

			xExportPackageRecur(vecAppendTo, SubPackageItem.second, vecPrefix, pwchIndent, uIndentLen);

			vecAppendTo.CopyToEnd(pwchCurrentPrefix, uCurrentPrefixLen);
			vecAppendTo.CopyToEnd(L"}\n", 2);
		}

		if(pwchIndent != nullptr){
			vecPrefix.TruncateFromEnd(uIndentLen);
		}
	}

	for(const auto &ValueItem : pkgWhich.mapValues){
		vecAppendTo.CopyToEnd(pwchCurrentPrefix, uCurrentPrefixLen);
		xEscapeAndAppend(vecAppendTo, ValueItem.first);
		vecAppendTo.CopyToEnd(L" = ", 3);
		xEscapeAndAppend(vecAppendTo, ValueItem.second);
		vecAppendTo.Push(L'\n');
	}
}

// 构造函数和析构函数。
NotationClass::NotationClass(){
}
NotationClass::NotationClass(const wchar_t *pwszText){
	Parse(pwszText);
}
NotationClass::NotationClass(const wchar_t *pwchText, std::size_t uLen){
	Parse(pwchText, uLen);
}

// 其他非静态成员函数。
std::pair<NotationClass::ERROR_TYPE, const wchar_t *> NotationClass::Parse(const wchar_t *pwszText){
	return Parse(pwszText, std::wcslen(pwszText));
}
std::pair<NotationClass::ERROR_TYPE, const wchar_t *> NotationClass::Parse(const wchar_t *pwchText, std::size_t uLen){
	xm_Root.Clear();

	if(uLen == 0){
		return std::make_pair(ERR_NONE, nullptr);
	}

	const wchar_t *pwszRead = pwchText;

	Package NewRoot;
	MCF::VVector<Package *> vecPackageStack;
	vecPackageStack.Push(&NewRoot);

	const wchar_t *pNameBegin = pwszRead;
	const wchar_t *pNameEnd = pwszRead;
	const wchar_t *pValueBegin = pwszRead;
	const wchar_t *pValueEnd = pwszRead;
	enum STATE {
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

		vecPackageStack.Push(&(vecPackageStack.GetEnd()[-1]->mapPackages[xUnescapeAndConstruct(pNameBegin, pNameEnd)] = Package()));

		pNameBegin = pwszRead;
		pNameEnd = pwszRead;
	};
	const auto PopPackage = [&]() -> void {
		assert(vecPackageStack.GetSize() > 0);

		vecPackageStack.Pop();

		pNameBegin = pwszRead;
		pNameEnd = pwszRead;
	};
	const auto SubmitValue = [&]() -> void {
		assert(pNameBegin != pNameEnd);

		vecPackageStack.GetEnd()[-1]->mapValues[xUnescapeAndConstruct(pNameBegin, pNameEnd)] = xUnescapeAndConstruct(pValueBegin, pValueEnd);

		pValueBegin = pwszRead;
		pValueEnd = pwszRead;
	};

	for(std::size_t i = 0; i < uLen; ++i, ++pwszRead){
		const wchar_t ch = *pwszRead;

		if(bEscaped){
			bEscaped = false;
		} else {
			switch(ch){
			case L'\\':
				bEscaped = true;
				continue;
			case L'=':
				switch(eState){
				case NAME_INDENT:
					return std::make_pair(ERR_NO_VALUE_NAME, pwszRead);
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
			case L'{':
				switch(eState){
				case NAME_INDENT:
					return std::make_pair(ERR_NO_VALUE_NAME, pwszRead);
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
			case L'}':
				switch(eState){
				case NAME_INDENT:
					if(vecPackageStack.GetSize() == 1){
						return std::make_pair(ERR_UNEXCEPTED_PACKAGE_CLOSE, pwszRead);
					}
					PopPackage();
					eState = NAME_INDENT;
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					if(vecPackageStack.GetSize() == 1){
						return std::make_pair(ERR_UNEXCEPTED_PACKAGE_CLOSE, pwszRead);
					}
					PopPackage();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case L';':
				switch(eState){
				case NAME_INDENT:
					eState = COMMENT;
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
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
			case L'\n':
				switch(eState){
				case NAME_INDENT:
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
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

		if(ch != L'\n'){
			switch(eState){
			case NAME_INDENT:
				if((ch == L' ') || (ch == L'\t')){
					// eState = NAME_INDENT;
				} else {
					pNameBegin = pwszRead;
					pNameEnd = pwszRead + 1;
					eState = NAME_BODY;
				}
				continue;
			case NAME_BODY:
				if((ch == L' ') || (ch == L'\t')){
					eState = NAME_PADDING;
				} else {
					pNameEnd = pwszRead + 1;
					// eState = NAME_BODY;
				}
				continue;
			case NAME_PADDING:
				if((ch == L' ') || (ch == L'\t')){
					// eState = NAME_PADDING;
				} else {
					pNameEnd = pwszRead + 1;
					eState = NAME_BODY;
				}
				continue;
			case VAL_INDENT:
				if((ch == L' ') || (ch == L'\t')){
					// eState = VAL_INDENT;
				} else {
					pValueBegin = pwszRead;
					pValueEnd = pwszRead + 1;
					eState = VAL_BODY;
				}
				continue;
			case VAL_BODY:
				if((ch == L' ') || (ch == L'\t')){
					eState = VAL_PADDING;
				} else {
					pValueEnd = pwszRead + 1;
					// eState = VAL_BODY;
				}
				continue;
			case VAL_PADDING:
				if((ch == L' ') || (ch == L'\t')){
					// eState = VAL_PADDING;
				} else {
					pValueEnd = pwszRead + 1;
					eState = VAL_BODY;
				}
				continue;
			case COMMENT:
				continue;
			}
		}
	}
	if(bEscaped){
		return std::make_pair(ERR_ESCAPE_AT_EOF, pwszRead);
	}
	if(vecPackageStack.GetSize() > 1){
		return std::make_pair(ERR_UNCLOSED_PACKAGE, pwszRead);
	}
	switch(eState){
	case NAME_BODY:
	case NAME_PADDING:
		return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
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
WCString NotationClass::Export(const wchar_t *pwchIndent) const {
	VVector<wchar_t> vecTemp;
	VVector<wchar_t> vecPrefix;
	xExportPackageRecur(
		vecTemp,
		xm_Root,
		vecPrefix,
		pwchIndent,
		(pwchIndent == nullptr) ? (std::size_t)0 : std::wcslen(pwchIndent)
	);
	return WCString(vecTemp.GetData(), vecTemp.GetSize());
}
