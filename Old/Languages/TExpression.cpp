// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TExpression.hpp"
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
				switch(wc){
				case L'\\':
					eState = kStEscaped;
					break;

				case L'\"':
					// eState = kStNormal;
					break;

				default:
					// eState = kStNormal;
					wsRet.Push(wc);
					break;
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

			default:
				ASSERT(false);
			}
		}
		if(eState == kStUcsCode){
			wsRet += Utf32StringView(&c32CodePoint, 1);
		}

		return wsRet;
	}
	void Escape(WideString &wsAppendTo, const WideStringView &wsvSrc){
		const auto uSrcLength = wsvSrc.GetLength();
		wsAppendTo.Reserve(wsAppendTo.GetLength() + uSrcLength);

		for(std::size_t i = 0; i < uSrcLength; ++i){
			const auto wc = wsvSrc[i];

			switch(wc){
			case L'(':
			case L')':
			case L';':
			case L' ':
				wsAppendTo += L'\\';
				wsAppendTo += wc;
				break;

			case L'\\':
				wsAppendTo += L'\\';
				wsAppendTo += L'\\';
				break;

			case L'\"':
				wsAppendTo += L'\\';
				wsAppendTo += L'\"';
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
std::pair<TExpression::ErrorType, const wchar_t *> TExpression::Parse(const WideStringView &wsvData){
	TExpression vTemp;

	auto pwcRead = wsvData.GetBegin();
	const auto pwcEnd = wsvData.GetEnd();
	if(pwcRead == pwcEnd){
		return std::make_pair(kErrNone, pwcRead);
	}

	Vector<TExpressionNode *> vecNodeStack(1, &vTemp);

	auto pwcNameBegin = pwcRead;

	enum {
		kStDelimiter,
		kStName,
		kStComment,
		kStNameQuoted,
		kStNameEscaped,
		kStNameQuotedEscaped,
		kStCommentEscaped
	} eState = kStDelimiter;

	const auto PushNode = [&]{
		ASSERT(!vecNodeStack.IsEmpty());

		auto &lstChildren = vecNodeStack.GetBack()->x_lstChildren;
		auto &vNewNode = lstChildren.Push()->Get();
		vNewNode.first = Unescape(WideStringView(pwcNameBegin, pwcRead));
		vecNodeStack.Push(&vNewNode.second);
	};
	const auto PushUnnamedNode = [&]{
		ASSERT(!vecNodeStack.IsEmpty());

		auto &lstChildren = vecNodeStack.GetBack()->x_lstChildren;
		auto &vNewNode = lstChildren.Push()->Get();
		vecNodeStack.Push(&vNewNode.second);
	};
	const auto PopNode = [&]{
		ASSERT(vecNodeStack.GetSize() > 1);

		vecNodeStack.Pop();
	};

	do {
		const wchar_t wc = *pwcRead;

		switch(wc){
		case L'(':
			switch(eState){
			case kStDelimiter:
				PushUnnamedNode();
				// eState = kStDelimiter;
				continue;

			case kStName:
				PushNode();
				eState = kStDelimiter;
				continue;

			case kStComment:
			case kStNameQuoted:
			case kStNameEscaped:
			case kStNameQuotedEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L')':
			switch(eState){
			case kStDelimiter:
				if(vecNodeStack.GetSize() <= 1){
					return std::make_pair(kErrUnexpectedNodeClose, pwcRead);
				}
				PopNode();
				// eState = kStDelimiter;
				continue;

			case kStName:
				PushNode();
				PopNode();
				if(vecNodeStack.GetSize() <= 1){
					return std::make_pair(kErrUnexpectedNodeClose, pwcRead);
				}
				PopNode();
				eState = kStDelimiter;
				continue;

			case kStComment:
			case kStNameQuoted:
			case kStNameEscaped:
			case kStNameQuotedEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L';':
			switch(eState){
			case kStDelimiter:
				eState = kStComment;
				continue;

			case kStName:
				PushNode();
				PopNode();
				eState = kStComment;
				continue;

			case kStComment:
			case kStNameQuoted:
			case kStNameEscaped:
			case kStNameQuotedEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L' ':
		case L'\t':
			switch(eState){
			case kStDelimiter:
				// eState = kStDelimiter;
				continue;

			case kStName:
				PushNode();
				PopNode();
				eState = kStDelimiter;
				continue;

			case kStComment:
			case kStNameQuoted:
			case kStNameEscaped:
			case kStNameQuotedEscaped:
			case kStCommentEscaped:
				break;
			};
			break;

		case L'\n':
			switch(eState){
			case kStDelimiter:
				// eState = kStDelimiter;
				continue;

			case kStName:
				PushNode();
				PopNode();
				eState = kStDelimiter;
				continue;

			case kStComment:
				eState = kStDelimiter;
				continue;

			case kStNameQuoted:
			case kStNameEscaped:
			case kStNameQuotedEscaped:
			case kStCommentEscaped:
				break;
			};
			break;
		}

		switch(eState){
		case kStDelimiter:
			pwcNameBegin = pwcRead;

			switch(wc){
			case L'\"':
				eState = kStNameQuoted;
				break;

			case L'\\':
				eState = kStNameEscaped;
				break;

			default:
				eState = kStName;
				break;
			}
			break;

		case kStName:
			switch(wc){
			case L'\"':
				eState = kStNameQuoted;
				break;

			case L'\\':
				eState = kStNameEscaped;
				break;
			}
			break;

		case kStComment:
			if(wc == L'\\'){
				eState = kStCommentEscaped;
			}
			break;

		case kStNameQuoted:
			switch(wc){
			case L'\"':
				eState = kStName;
				break;

			case L'\\':
				eState = kStNameQuotedEscaped;
				break;
			}
			break;

		case kStNameEscaped:
			eState = kStName;
			break;

		case kStNameQuotedEscaped:
			eState = kStNameQuoted;
			break;

		case kStCommentEscaped:
			eState = kStComment;
			break;
		}
	} while(++pwcRead != pwcEnd);

	switch(eState){
	case kStDelimiter:
		break;

	case kStName:
		PushNode();
		PopNode();
		break;

	case kStComment:
		break;

	case kStNameQuoted:
		return std::make_pair(kErrUnclosedQuote, pwcRead);

	case kStNameEscaped:
		PushNode();
		PopNode();
		break;

	case kStNameQuotedEscaped:
		return std::make_pair(kErrUnclosedQuote, pwcRead);

	case kStCommentEscaped:
		break;
	}
	if(vecNodeStack.GetSize() > 1){
		return std::make_pair(kErrUnclosedNode, pwcRead);
	}

	Swap(vTemp);
	return std::make_pair(kErrNone, pwcRead);
}
WideString TExpression::Export(const WideStringView &wsvIndent) const {
	WideString wsRet;

	Vector<std::pair<const TExpressionNode *, const ChildNode *>> vecNodeStack;
	vecNodeStack.Push(this, x_lstChildren.GetFirst());
	WideString wsIndent;
	for(;;){
		auto &vTop = vecNodeStack.GetBack();
		const auto &vTopNode = *vTop.first;

	jNextChild:
		if(vTop.second){
			const auto &wsName = vTop.second->Get().first;
			const auto &vNode = vTop.second->Get().second;
			vTop.second = vTop.second->GetNext();

			wsRet += wsIndent;
			if(!wsName.IsEmpty()){
				Escape(wsRet, wsName);
				if(vNode.x_lstChildren.IsEmpty()){
					wsRet += L'\n';
					goto jNextChild;
				}
			}
			wsRet += L'(';
			if(vNode.x_lstChildren.IsEmpty()){
				wsRet += L')';
				wsRet += L'\n';
				goto jNextChild;
			}
			wsRet += L'\n';
			wsIndent += wsvIndent;
			vecNodeStack.Push(&vNode, vNode.x_lstChildren.GetFirst());
			continue;
		}

		vecNodeStack.Pop();
		if(vecNodeStack.IsEmpty()){
			break;
		}

		wsIndent.Truncate(wsvIndent.GetLength());
		wsRet += wsIndent;
		wsRet += L')';
		wsRet += L'\n';
	}

	ASSERT(wsIndent.IsEmpty());

	return wsRet;
}

}
