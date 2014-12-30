// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TExpression.hpp"
#include "../Containers/Vector.hpp"
using namespace MCF;

namespace {

WideString Unescape(const WideStringObserver &wsoSrc){
	WideString wsRet;
	wsRet.Reserve(wsoSrc.GetSize());

	enum {
		NORMAL,
		ESCAPED,
		UCS_CODE
	} eState = NORMAL;

	char32_t c32CodePoint = 0;
	std::size_t uHexExpecting = 0;

	const auto pwcEnd = wsoSrc.GetEnd();
	for(auto pwcCur = wsoSrc.GetBegin(); pwcCur != pwcEnd; ++pwcCur){
		const auto wc = *pwcCur;

		switch(eState){
		case NORMAL:
			switch(wc){
			case L'\\':
				eState = ESCAPED;
				break;

			case L'\"':
				// eState = NORMAL;
				break;

			default:
				// eState = NORMAL;
				wsRet.Push(wc);
				break;
			}
			break;

		case ESCAPED:
			switch(wc){
			case L'n':
				wsRet.Append(L'\n');
				eState = NORMAL;
				break;

			case L'b':
				wsRet.Append(L'\b');
				eState = NORMAL;
				break;

			case L'r':
				wsRet.Append(L'\r');
				eState = NORMAL;
				break;

			case L't':
				wsRet.Append(L'\t');
				eState = NORMAL;
				break;

			case L'\n':
				eState = NORMAL;
				break;

			case L'x':
				c32CodePoint = 0;
				uHexExpecting = 2;
				eState = UCS_CODE;
				break;

			case L'u':
				c32CodePoint = 0;
				uHexExpecting = 4;
				eState = UCS_CODE;
				break;

			case L'U':
				c32CodePoint = 0;
				uHexExpecting = 8;
				eState = UCS_CODE;
				break;

			default:
				wsRet.Append(wc);
				eState = NORMAL;
				break;
			}
			break;

		case UCS_CODE:
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
					// eState = UCS_CODE;
				} else {
					wsRet.Append(Utf32StringObserver(&c32CodePoint, 1));
					eState = NORMAL;
				}
			}
			break;

		default:
			ASSERT(false);
		}
	}
	if(eState == UCS_CODE){
		wsRet.Append(Utf32StringObserver(&c32CodePoint, 1));
	}

	return wsRet;
}
void Escape(WideString &wsAppendTo, const WideStringObserver &wsoSrc){
	const auto uSrcLength = wsoSrc.GetLength();
	wsAppendTo.Reserve(wsAppendTo.GetLength() + uSrcLength);

	for(std::size_t i = 0; i < uSrcLength; ++i){
		const auto wc = wsoSrc[i];

		switch(wc){
		case L'(':
		case L')':
		case L';':
		case L' ':
			wsAppendTo.Append(L'\\');
			wsAppendTo.Append(wc);
			break;

		case L'\\':
			wsAppendTo.Append(L'\\');
			wsAppendTo.Append(L'\\');
			break;

		case L'\"':
			wsAppendTo.Append(L'\\');
			wsAppendTo.Append(L'\"');
			break;

		case L'\n':
			wsAppendTo.Append(L'\\');
			wsAppendTo.Append(L'n');
			break;

		case L'\r':
			wsAppendTo.Append(L'\\');
			wsAppendTo.Append(L'r');
			break;

		case L'\t':
			wsAppendTo.Append(L'\\');
			wsAppendTo.Append(L't');
			break;

		default:
			wsAppendTo.Append(wc);
			break;
		}
	}
}

}

// 其他非静态成员函数。
std::pair<TExpression::ErrorType, const wchar_t *> TExpression::Parse(const WideStringObserver &wsoData){
	TExpression vTemp;

	auto pwcRead = wsoData.GetBegin();
	const auto pwcEnd = wsoData.GetEnd();
	if(pwcRead == pwcEnd){
		return std::make_pair(ERR_NONE, pwcRead);
	}

	Vector<TExpressionNode *> vecNodeStack(1, &vTemp);

	auto pwcNameBegin = pwcRead;

	enum {
		DELIMITER,
		NAME,
		COMMENT,
		NAME_QUOTED,
		NAME_ESCAPED,
		NAME_QUO_ESC,
		COMMENT_ESCAPED
	} eState = DELIMITER;

	const auto PushNode = [&]{
		ASSERT(!vecNodeStack.IsEmpty());

		auto &lstChildren = vecNodeStack.GetEnd()[-1]->xm_lstChildren;
		auto &vNewNode = lstChildren.Push()->Get();
		vNewNode.first = Unescape(WideStringObserver(pwcNameBegin, pwcRead));
		vecNodeStack.Push(&vNewNode.second);
	};
	const auto PushUnnamedNode = [&]{
		ASSERT(!vecNodeStack.IsEmpty());

		auto &lstChildren = vecNodeStack.GetEnd()[-1]->xm_lstChildren;
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
			case DELIMITER:
				PushUnnamedNode();
				// eState = DELIMITER;
				continue;

			case NAME:
				PushNode();
				eState = DELIMITER;
				continue;

			case COMMENT:
			case NAME_QUOTED:
			case NAME_ESCAPED:
			case NAME_QUO_ESC:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L')':
			switch(eState){
			case DELIMITER:
				if(vecNodeStack.GetSize() <= 1){
					return std::make_pair(ERR_UNEXCEPTED_NODE_CLOSE, pwcRead);
				}
				PopNode();
				// eState = DELIMITER;
				continue;

			case NAME:
				PushNode();
				PopNode();
				if(vecNodeStack.GetSize() <= 1){
					return std::make_pair(ERR_UNEXCEPTED_NODE_CLOSE, pwcRead);
				}
				PopNode();
				eState = DELIMITER;
				continue;

			case COMMENT:
			case NAME_QUOTED:
			case NAME_ESCAPED:
			case NAME_QUO_ESC:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L';':
			switch(eState){
			case DELIMITER:
				eState = COMMENT;
				continue;

			case NAME:
				PushNode();
				PopNode();
				eState = COMMENT;
				continue;

			case COMMENT:
			case NAME_QUOTED:
			case NAME_ESCAPED:
			case NAME_QUO_ESC:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L' ':
		case L'\t':
			switch(eState){
			case DELIMITER:
				// eState = DELIMITER;
				continue;

			case NAME:
				PushNode();
				PopNode();
				eState = DELIMITER;
				continue;

			case COMMENT:
			case NAME_QUOTED:
			case NAME_ESCAPED:
			case NAME_QUO_ESC:
			case COMMENT_ESCAPED:
				break;
			};
			break;

		case L'\n':
			switch(eState){
			case DELIMITER:
				// eState = DELIMITER;
				continue;

			case NAME:
				PushNode();
				PopNode();
				eState = DELIMITER;
				continue;

			case COMMENT:
				eState = DELIMITER;
				continue;

			case NAME_QUOTED:
			case NAME_ESCAPED:
			case NAME_QUO_ESC:
			case COMMENT_ESCAPED:
				break;
			};
			break;
		}

		switch(eState){
		case DELIMITER:
			pwcNameBegin = pwcRead;

			switch(wc){
			case L'\"':
				eState = NAME_QUOTED;
				break;

			case L'\\':
				eState = NAME_ESCAPED;
				break;

			default:
				eState = NAME;
				break;
			}
			break;

		case NAME:
			switch(wc){
			case L'\"':
				eState = NAME_QUOTED;
				break;

			case L'\\':
				eState = NAME_ESCAPED;
				break;
			}
			break;

		case COMMENT:
			if(wc == L'\\'){
				eState = COMMENT_ESCAPED;
			}
			break;

		case NAME_QUOTED:
			switch(wc){
			case L'\"':
				eState = NAME;
				break;

			case L'\\':
				eState = NAME_QUO_ESC;
				break;
			}
			break;

		case NAME_ESCAPED:
			eState = NAME;
			break;

		case NAME_QUO_ESC:
			eState = NAME_QUOTED;
			break;

		case COMMENT_ESCAPED:
			eState = COMMENT;
			break;
		}
	} while(++pwcRead != pwcEnd);

	switch(eState){
	case DELIMITER:
		break;

	case NAME:
		PushNode();
		PopNode();
		break;

	case COMMENT:
		break;

	case NAME_QUOTED:
		return std::make_pair(ERR_UNCLOSED_QUOTE, pwcRead);

	case NAME_ESCAPED:
		PushNode();
		PopNode();
		break;

	case NAME_QUO_ESC:
		return std::make_pair(ERR_UNCLOSED_QUOTE, pwcRead);

	case COMMENT_ESCAPED:
		break;
	}
	if(vecNodeStack.GetSize() > 1){
		return std::make_pair(ERR_UNCLOSED_NODE, pwcRead);
	}

	Swap(vTemp);
	return std::make_pair(ERR_NONE, pwcRead);
}
WideString TExpression::Export(const WideStringObserver &wsoIndent) const {
	WideString wsRet;

	Vector<std::pair<const TExpressionNode *, const ChildNode *>> vecNodeStack;
	vecNodeStack.Push(this, xm_lstChildren.GetFirst());
	WideString wsIndent;
	for(;;){
		auto &vTop = vecNodeStack.GetEnd()[-1];
		const auto &vTopNode = *vTop.first;

	jNextChild:
		if(vTop.second){
			const auto &wsName = vTop.second->Get().first;
			const auto &vNode = vTop.second->Get().second;
			vTop.second = vTop.second->GetNext();

			wsRet.Append(wsIndent);
			if(!wsName.IsEmpty()){
				Escape(wsRet, wsName);
				if(vNode.xm_lstChildren.IsEmpty()){
					wsRet.Append(L'\n');
					goto jNextChild;
				}
			}
			wsRet.Append(L'(');
			if(vNode.xm_lstChildren.IsEmpty()){
				wsRet.Append(L')');
				wsRet.Append(L'\n');
				goto jNextChild;
			}
			wsRet.Append(L'\n');
			wsIndent.Append(wsoIndent);
			vecNodeStack.Push(&vNode, vNode.xm_lstChildren.GetFirst());
			continue;
		}

		vecNodeStack.Pop();
		if(vecNodeStack.IsEmpty()){
			break;
		}

		wsIndent.Truncate(wsoIndent.GetLength());
		wsRet.Append(wsIndent);
		wsRet.Append(L')');
		wsRet.Append(L'\n');
	}

	ASSERT(wsIndent.IsEmpty());

	return wsRet;
}
