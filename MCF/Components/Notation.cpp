// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Notation.hpp"
#include "../../MCFCRT/cpp/ext/vvector.hpp"
#include <cwchar>
using namespace MCF;

// 静态成员函数。
void Notation::xEscapeAndAppend(Utf16String &wcsAppendTo, const wchar_t *pwchBegin, std::size_t uLength){
	wcsAppendTo.Reserve(wcsAppendTo.GetLength() + uLength * 2);

	for(std::size_t i = 0; i < uLength; ++i){
		const auto ch = pwchBegin[i];
		switch(ch){
		case L'\\':
		case L'=':
		case L'{':
		case L'}':
		case L';':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(ch);
			break;
		case L'\n':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'n');
			break;
		case L'\b':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'b');
			break;
		case L'\r':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'r');
			break;
		case L'\t':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L't');
			break;
		default:
			wcsAppendTo.Append(ch);
			break;
		}
	}
}
Utf16String Notation::xUnescapeAndConstruct(const wchar_t *pwchBegin, std::size_t uLength){
	Utf16String wcsRet;
	wcsRet.Reserve(uLength);

	enum STATE {
		NORMAL,
		SLASH_MATCH,
		HEX_WAIT_FOR_NEXT
	} eState = NORMAL;

	int nDecodedDigit;
	wchar_t chDecoded = 0;
	wchar_t awchHexBuffer[sizeof(wchar_t) * CHAR_BIT / 4 - 1];
	std::size_t uBufferIndex = 0;

	for(std::size_t i = 0; i < uLength; ++i){
		const auto ch = pwchBegin[i];
		switch(eState){
		case NORMAL:
			if(ch == L'\\'){
				eState = SLASH_MATCH;
			} else {
				wcsRet.Push(ch);
			}
			break;
		case SLASH_MATCH:
			switch(ch){
			case L'b':
				wcsRet.Push(L'\b');
				eState = NORMAL;
				break;
			case L'n':
				wcsRet.Push(L'\n');
				eState = NORMAL;
				break;
			case L'r':
				wcsRet.Push(L'\r');
				eState = NORMAL;
				break;
			case L't':
				wcsRet.Push(L'\t');
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
				wcsRet.Push(ch);
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
				wcsRet.Push(L'x');
				for(std::size_t i = 0; i < uBufferIndex; ++i){
					wcsRet.Push(awchHexBuffer[i]);
				}
				eState = NORMAL;
				break;
			}
			if(uBufferIndex == COUNT_OF(awchHexBuffer)){
				wcsRet.Push((chDecoded << 4) | nDecodedDigit);
				eState = NORMAL;
			} else {
				awchHexBuffer[uBufferIndex++] = ch;
				chDecoded = (chDecoded << 4) | nDecodedDigit;
			}
			break;
		}
	}

	return std::move(wcsRet);
}

void Notation::xExportPackageRecur(
	Utf16String &wcsAppendTo,
	const Notation::Package &pkgWhich,
	Utf16String &wcsPrefix,
	const wchar_t *pwchIndent,
	std::size_t uIndentLen
){
	const auto uCurrentPrefixLen = wcsPrefix.GetLength();
	auto pwchCurrentPrefix = wcsPrefix.GetCStr();

	if(!pkgWhich.xm_mapPackages.IsEmpty()){
		if(pwchIndent){
			wcsPrefix.Append(pwchIndent, uIndentLen);
		}
		pwchCurrentPrefix = wcsPrefix.GetCStr();

		auto pPackageNode = pkgWhich.xm_mapPackages.GetFront<0>();
		while(pPackageNode){
			wcsAppendTo.Append(pwchCurrentPrefix, uCurrentPrefixLen);
			xEscapeAndAppend(wcsAppendTo, pPackageNode->GetIndex<0>().GetCStr(), pPackageNode->GetIndex<0>().GetLength());
			wcsAppendTo.Append(L" {\n", 3);

			xExportPackageRecur(wcsAppendTo, pPackageNode->GetElement(), wcsPrefix, pwchIndent, uIndentLen);

			wcsAppendTo.Append(pwchCurrentPrefix, uCurrentPrefixLen);
			wcsAppendTo.Append(L"}\n", 2);

			pPackageNode = pkgWhich.xm_mapPackages.GetNext<0>(pPackageNode);
		}

		if(pwchIndent){
			wcsPrefix.Pop(uIndentLen);
		}
	}

	auto pValueNode = pkgWhich.xm_mapValues.GetFront<0>();
	while(pValueNode){
		wcsAppendTo.Append(pwchCurrentPrefix, uCurrentPrefixLen);
		xEscapeAndAppend(wcsAppendTo, pValueNode->GetIndex<0>().GetCStr(), pValueNode->GetIndex<0>().GetLength());
		wcsAppendTo.Append(L" = ", 3);
		xEscapeAndAppend(wcsAppendTo, pValueNode->GetElement().GetCStr(), pValueNode->GetElement().GetLength());
		wcsAppendTo.Append(L'\n');

		pValueNode = pkgWhich.xm_mapValues.GetNext<0>(pValueNode);
	}
}

// 构造函数和析构函数。
Notation::Notation(const wchar_t *pwchText, std::size_t uLen){
	Parse(pwchText, uLen);
}

// 其他非静态成员函数。
std::pair<Notation::ErrorType, const wchar_t *> Notation::Parse(const wchar_t *pwchText, std::size_t uLen){
	Clear();

	if(uLen == 0){
		return std::make_pair(ERR_NONE, nullptr);
	}

	if(uLen == (std::size_t)-1){
		uLen = std::wcslen(pwchText);
	}

	const wchar_t *pwszRead = pwchText;

	MCF::VVector<Package *> vecPackageStack;
	vecPackageStack.Push(this);

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

	const auto PushPackage = [&]{
		ASSERT(pNameBegin != pNameEnd);

		auto wcsName = xUnescapeAndConstruct(pNameBegin, (std::size_t)(pNameEnd - pNameBegin));

		auto &mapPackages = vecPackageStack.GetEnd()[-1]->xm_mapPackages;
		auto pNewPackageNode = mapPackages.Find<0>(wcsName);
		if(!pNewPackageNode){
			pNewPackageNode = mapPackages.Insert(Package(), std::move(wcsName));
		}
		vecPackageStack.Push(&(pNewPackageNode->GetElement()));

		pNameBegin = pwszRead;
		pNameEnd = pwszRead;
	};
	const auto PopPackage = [&]{
		ASSERT(vecPackageStack.GetSize() > 0);

		vecPackageStack.Pop();

		pNameBegin = pwszRead;
		pNameEnd = pwszRead;
	};
	const auto SubmitValue = [&]{
		ASSERT(pNameBegin != pNameEnd);

		auto wcsName = xUnescapeAndConstruct(pNameBegin, (std::size_t)(pNameEnd - pNameBegin));
		auto wcsValue = xUnescapeAndConstruct(pValueBegin, (std::size_t)(pValueEnd - pValueBegin));

		auto &mapValues = vecPackageStack.GetEnd()[-1]->xm_mapValues;
		const auto pValueNode = mapValues.Find<0>(wcsName);
		if(pValueNode){
			pValueNode->GetElement() = std::move(wcsValue);
		} else {
			mapValues.Insert(std::move(wcsValue), std::move(wcsName));
		}

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

	return std::make_pair(ERR_NONE, nullptr);
}
Utf16String Notation::Export(const wchar_t *pwchIndent) const {
	Utf16String wcsResult;
	Utf16String wcsPrefix;
	const std::size_t uIndentLength = pwchIndent ? (std::size_t)0 : std::wcslen(pwchIndent);
	xExportPackageRecur(wcsResult, *this, wcsPrefix, pwchIndent, uIndentLength);
	return std::move(wcsResult);
}
