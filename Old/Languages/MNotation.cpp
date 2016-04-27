// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "MNotation.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

namespace {
	WideString Unescape(const WideStringView &wsvSrc){
		WideString wsRet;
		wsRet.Reserve(wsvSrc.GetSize());

		enum {
			kStNormal,
			kStEscaped,
			kStUcsCode,
		} eState = kStNormal;

		char32_t c32CodePoint = 0;
		std::size_t uHexExpecting = 0;

		const auto pwcEnd = wsvSrc.GetEnd();
		for(auto pwcCur = wsvSrc.GetBegin(); pwcCur != pwcEnd; ++pwcCur){
			const auto wc = *pwcCur;

			switch(eState){
			case kStNormal:
				if(wc == L'\\'){
					eState = kStEscaped;
				} else {
					// eState = kStNormal;
					wsRet.Push(wc);
				}
				break;

			case kStEscaped:
				switch(wc){
				case L'n':
					wsRet += L'\n';
					eState = kStNormal;
					break;

				case L'b':
					wsRet += L'\b';
					eState = kStNormal;
					break;

				case L'r':
					wsRet += L'\r';
					eState = kStNormal;
					break;

				case L't':
					wsRet += L'\t';
					eState = kStNormal;
					break;

				case L'\n':
					eState = kStNormal;
					break;

				case L'x':
					c32CodePoint = 0;
					uHexExpecting = 2;
					eState = kStUcsCode;
					break;

				case L'u':
					c32CodePoint = 0;
					uHexExpecting = 4;
					eState = kStUcsCode;
					break;

				case L'U':
					c32CodePoint = 0;
					uHexExpecting = 8;
					eState = kStUcsCode;
					break;

				default:
					wsRet += wc;
					eState = kStNormal;
					break;
				}
				break;

			case kStUcsCode:
				{
					auto uHex = (unsigned)(wc - L'0');
					do {
						if(uHex <= 9){
							break;
						}
						uHex += (unsigned)(L'0' - L'A');
						if(uHex <= 5){
							uHex += 0x0A;
							break;
						}
						uHex += (unsigned)(L'A' - L'a');
						if(uHex <= 5){
							uHex += 0x0A;
							break;
						}
						uHex = 0x10;
					} while(false);

					if(uHex <= 0x0F){
						c32CodePoint = (c32CodePoint << 4) | uHex;
						--uHexExpecting;
					} else {
						uHexExpecting = 0;
					}
					if(uHexExpecting != 0){
						// eState = kStUcsCode;
					} else {
						wsRet += Utf32StringView(&c32CodePoint, 1);
						eState = kStNormal;
					}
				}
				break;
			}
		}
		if(eState == kStUcsCode){
			wsRet += Utf32StringView(&c32CodePoint, 1);
		}

		return wsRet;
	}
	void Escape(WideString &wsAppendTo, const WideStringView &wsvSrc){
		const auto uSrcLength = wsvSrc.GetLength();
		wsAppendTo.ReserveMore(uSrcLength);

		for(std::size_t i = 0; i < uSrcLength; ++i){
			const auto wc = wsvSrc[i];

			switch(wc){
			case L'\\':
			case L'=':
			case L'{':
			case L'}':
			case L';':
				wsAppendTo += L'\\';
				wsAppendTo += wc;
				break;

			case L' ':
				if((i == 0) || (i == uSrcLength - 1)){
					wsAppendTo += L'\\';
				}
				wsAppendTo += L' ';
				break;

			case L'\n':
				wsAppendTo += L'\\';
				wsAppendTo += L'n';
				break;

			case L'\r':
				wsAppendTo += L'\\';
				wsAppendTo += L'r';
				break;

			case L'\t':
				wsAppendTo += L'\\';
				wsAppendTo += L't';
				break;

			default:
				wsAppendTo += wc;
				break;
			}
		}
	}
}

// 其他非静态成员函数。
std::pair<MNotation::ErrorType, const wchar_t *> MNotation::Parse(const WideStringView &wsvData){
	MNotation vTemp;

	auto pwcRead = wsvData.GetBegin();
	const auto pwcEnd = wsvData.GetEnd();
	if(pwcRead == pwcEnd){
		return std::make_pair(kErrNone, pwcRead);
	}

	Vector<MNotationNode *> vecPackageStack(1, &vTemp);

	const wchar_t *pwcNameBegin = nullptr;
	const wchar_t *pwcNameEnd = nullptr;
	const wchar_t *pwcValueBegin = nullptr;
	const wchar_t *pwcValueEnd = nullptr;

	enum {
		kStNameIndent,
		kStNameBody,
		kStNamePadding,
		kStValueIndent,
		kStValueBody,
		kStValPadding,
		kStComment,
		kStNameEscaped,
		kStValueEscaped,
		kStCommentEscaped,
	} eState = kStNameIndent;

	ErrorType eError;
	const auto PushPackage = [&]{
		_MCFCRT_ASSERT(!vecPackageStack.IsEmpty());

		MNotationNode *ppkgSource = nullptr;
		if(pwcValueBegin){
			const auto wsSourceName = Unescape(WideStringView(pwcValueBegin, pwcValueEnd));
			const auto pSourceNode = vecPackageStack.GetEnd()[-1]->Get(wsSourceName);
			if(!pSourceNode){
				eError = kErrSourcePackageNotFound;
				return false;
			}
			ppkgSource = &(pSourceNode->Get().second);
		}

		const auto vResult = vecPackageStack.GetEnd()[-1]->Insert(Unescape(WideStringView(pwcNameBegin, pwcNameEnd)));
		if(!vResult.second){
//			eError = ERR_DUPLICATE_PACKAGE;
//			return false;
		}
		if(ppkgSource){
			vResult.first->Get().second = *ppkgSource;
		}

		vecPackageStack.Push(&(vResult.first->Get().second));
		pwcNameBegin = nullptr;
		pwcNameEnd = nullptr;
		pwcValueBegin = nullptr;
		pwcValueEnd = nullptr;
		return true;
	};
	const auto PopPackage = [&]{
		if(vecPackageStack.GetSize() <= 1){
			eError = kErrUnexpectedPackageClose;
			return false;
		}

		vecPackageStack.Pop();
		pwcNameBegin = nullptr;
		pwcNameEnd = nullptr;
		pwcValueBegin = nullptr;
		pwcValueEnd = nullptr;
		return true;
	};
	const auto AcceptValue = [&]{
		_MCFCRT_ASSERT(!vecPackageStack.IsEmpty());

		const auto vResult = vecPackageStack.GetEnd()[-1]->Insert(Unescape(WideStringView(pwcNameBegin, pwcNameEnd)));
		if(!vResult.second){
//			eError = kErrDuplicateValue;
//			return false;
		}
		vResult.first->Get().second.Insert(Unescape(WideStringView(pwcValueBegin, pwcValueEnd)));

		pwcNameBegin = nullptr;
		pwcNameEnd = nullptr;
		pwcValueBegin = nullptr;
		pwcValueEnd = nullptr;
		return true;
	};

	do {
		const wchar_t wc = *pwcRead;

		switch(wc){
		case L'=':
			switch(eState){
			case kStNameIndent:
			case kStNameBody:
			case kStNamePadding:
				eState = kStValueIndent;
				continue;

			case kStValueIndent:
			case kStValueBody:
			case kStValPadding:
			case kStComment:
			case kStNameEscaped:
			case kStValueEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L'{':
			switch(eState){
			case kStNameIndent:
			case kStNameBody:
			case kStNamePadding:
				if(!PushPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = kStNameIndent;
				continue;

			case kStValueIndent:
			case kStValueBody:
			case kStValPadding:
				if(!PushPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = kStNameIndent;
				continue;

			case kStComment:
			case kStNameEscaped:
			case kStValueEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L'}':
			switch(eState){
			case kStNameIndent:
				if(!PopPackage()){
					return std::make_pair(eError, pwcRead);
				}
				// eState = kStNameIndent;
				continue;

			case kStNameBody:
			case kStNamePadding:
				return std::make_pair(kErrEquExpected, pwcRead);

			case kStValueIndent:
			case kStValueBody:
			case kStValPadding:
				if(!AcceptValue() || !PopPackage()){
					return std::make_pair(eError, pwcRead);
				}
				eState = kStNameIndent;
				continue;

			case kStComment:
			case kStNameEscaped:
			case kStValueEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L';':
			switch(eState){
			case kStNameIndent:
				eState = kStComment;
				continue;

			case kStNameBody:
			case kStNamePadding:
				return std::make_pair(kErrEquExpected, pwcRead);

			case kStValueIndent:
			case kStValueBody:
			case kStValPadding:
				if(!AcceptValue()){
					return std::make_pair(eError, pwcRead);
				}
				eState = kStComment;
				continue;

			case kStComment:
			case kStNameEscaped:
			case kStValueEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L'\n':
			switch(eState){
			case kStNameIndent:
				continue;

			case kStNameBody:
			case kStNamePadding:
				return std::make_pair(kErrEquExpected, pwcRead);

			case kStValueIndent:
			case kStValueBody:
			case kStValPadding:
				if(!AcceptValue()){
					return std::make_pair(eError, pwcRead);
				}
				eState = kStNameIndent;
				continue;

			case kStComment:
				eState = kStNameIndent;
				continue;

			case kStNameEscaped:
			case kStValueEscaped:
			case kStCommentEscaped:
				break;
			};
			break;
		}

		switch(eState){
		case kStNameIndent:
			switch(wc){
			case L' ':
			case L'\t':
				pwcNameBegin = pwcRead;
				pwcNameEnd = pwcRead;
				// eState = kStNameIndent;
				break;

			default:
				pwcNameBegin = pwcRead;
				pwcNameEnd = pwcRead + 1;
				eState = (wc == L'\\') ? kStNameEscaped : kStNameBody;
				break;
			}
			break;

		case kStNameBody:
			switch(wc){
			case L' ':
			case L'\t':
				eState = kStNamePadding;
				break;

			default:
				pwcNameEnd = pwcRead + 1;
				if(wc == L'\\'){
					eState = kStNameEscaped;
				}
				break;
			}
			break;

		case kStNamePadding:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = kStNamePadding;
				break;

			default:
				pwcNameEnd = pwcRead + 1;
				eState = (wc == L'\\') ? kStNameEscaped : kStNameBody;
				break;
			}
			break;

		case kStValueIndent:
			switch(wc){
			case L' ':
			case L'\t':
				pwcValueBegin = pwcRead;
				pwcValueEnd = pwcRead;
				// eState = kStValueIndent;
				break;

			default:
				pwcValueBegin = pwcRead;
				pwcValueEnd = pwcRead + 1;
				eState = (wc == L'\\') ? kStValueEscaped : kStValueBody;
				break;
			}
			break;

		case kStValueBody:
			switch(wc){
			case L' ':
			case L'\t':
				eState = kStValPadding;
				break;

			default:
				pwcValueEnd = pwcRead + 1;
				if(wc == L'\\'){
					eState = kStValueEscaped;
				}
				break;
			}
			break;

		case kStValPadding:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = kStValPadding;
				break;

			default:
				pwcValueEnd = pwcRead + 1;
				eState = (wc == L'\\') ? kStValueEscaped : kStValueBody;
				break;
			}
			break;

		case kStComment:
			if(wc == L'\\'){
				eState = kStCommentEscaped;
			}
			break;

		case kStNameEscaped:
			pwcNameEnd = pwcRead + 1;
			eState = kStNameBody;
			break;

		case kStValueEscaped:
			pwcValueEnd = pwcRead + 1;
			eState = kStValueBody;
			break;

		case kStCommentEscaped:
			eState = kStComment;
			break;
		}
	} while(++pwcRead != pwcEnd);

	switch(eState){
	case kStNameIndent:
		break;

	case kStNameBody:
	case kStNamePadding:
	case kStNameEscaped:
		return std::make_pair(kErrEquExpected, pwcRead);

	case kStValueIndent:
	case kStValueBody:
	case kStValPadding:
	case kStValueEscaped:
		if(!AcceptValue()){
			return std::make_pair(eError, pwcRead);
		}
		break;

	case kStComment:
	case kStCommentEscaped:
		break;
	};
	if(vecPackageStack.GetSize() > 1){
		return std::make_pair(kErrUnclosedPackage, pwcRead);
	}

	Swap(vTemp);
	return std::make_pair(kErrNone, pwcRead);
}
WideString MNotation::Export(const WideStringView &wsvIndent) const {
	WideString wsRet;

	Vector<std::pair<const MNotationNode *, const ChildNode *>> vecPackageStack;
	vecPackageStack.Push(this, x_mapChildren.GetFirst<1>());

	WideString wsIndent;
	for(;;){
		auto &vTop = vecPackageStack.GetEnd()[-1];

	jNextChild:
		if(vTop.second){
			const auto &wsName = vTop.second->Get().first;
			const auto &vNode = vTop.second->Get().second;
			vTop.second = vTop.second->GetNext<1>();

			wsRet += wsIndent;
			if(!wsName.IsEmpty()){
				Escape(wsRet, wsName);
				wsRet += L' ';
			}
			if(vNode.x_mapChildren.GetSize() == 1){
				const auto pNode = vNode.x_mapChildren.GetFirst<0>();
				if(pNode->Get().second.x_mapChildren.IsEmpty()){
					wsRet += L'=';
					wsRet += L' ';
					Escape(wsRet, pNode->Get().first);
					wsRet += L'\n';
					goto jNextChild;
				}
			}
			wsRet += L'{';
			wsRet += L'\n';
			wsIndent += wsvIndent;
			vecPackageStack.Push(&vNode, vNode.x_mapChildren.GetFirst<1>());
			continue;
		}
/*		if(vTop.second){
			wsRet += wsIndent;
			if(!wsName.IsEmpty()){
				Escape(wsRet, wsName);
				wsRet += L' ';
			}
			wsRet += L'{';
			wsRet += L'\n';
			wsIndent += wsvIndent;
			vecPackageStack.Push(&(vTop.second->Get().second), vTop.second->Get().second.x_mapChildren.GetFirst<1>());
			continue;
		}*/
/*
		for(auto pNode = vTop.first->x_mapValues.GetFirst<1>(); pNode; pNode = pNode->GetNext<1>()){
			wsRet += wsIndent;
			if(!pNode->first.IsEmpty()){
				Escape(wsRet, pNode->first);
				wsRet += L' ';
			}
		}
*/
		vecPackageStack.Pop();
		if(vecPackageStack.IsEmpty()){
			break;
		}

		wsIndent.Truncate(wsvIndent.GetLength());
		wsRet += wsIndent;
		wsRet += L'}';
		wsRet += L'\n';
	}

	_MCFCRT_ASSERT(wsIndent.IsEmpty());

	return wsRet;
}

}
