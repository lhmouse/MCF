// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TExpression.hpp"
#include "../Containers/VVector.hpp"
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

	std::uint32_t u32CodePoint = 0;
	std::size_t uHexExpecting = 0;

	const auto PushUtf = [&]{
		if(u32CodePoint > 0x10FFFF){
			u32CodePoint = 0xFFFD;
		}
		if(u32CodePoint <= 0xFFFF){
			wsRet.Append(u32CodePoint);
		} else {
			u32CodePoint -= 0x10000;
			wsRet.Append((u32CodePoint >> 10)   | 0xD800);
			wsRet.Append((u32CodePoint & 0x3FF) | 0xDC00);
		}
	};

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
				u32CodePoint = 0;
				uHexExpecting = 2;
				eState = UCS_CODE;
				break;

			case L'u':
				u32CodePoint = 0;
				uHexExpecting = 4;
				eState = UCS_CODE;
				break;

			case L'U':
				u32CodePoint = 0;
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
					u32CodePoint = (u32CodePoint << 4) | uHex;
					--uHexExpecting;
				} else {
					uHexExpecting = 0;
				}
				if(uHexExpecting != 0){
					// eState = UCS_CODE;
				} else {
					PushUtf();
					eState = NORMAL;
				}
			}
			break;

		default:
			ASSERT(false);
		}
	}
	if(eState == UCS_CODE){
		PushUtf();
	}

	return std::move(wsRet);
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
	m_deqChildren.clear();

	auto pwcRead = wsoData.GetBegin();
	const auto pwcEnd = wsoData.GetEnd();
	if(pwcRead == pwcEnd){
		return std::make_pair(ERR_NONE, pwcRead);
	}

	VVector<Node *> vecNodeStack(1, this);

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

		auto &deqChildren = vecNodeStack.GetEnd()[-1]->m_deqChildren;
		deqChildren.emplace_back();
		deqChildren.back().first = Unescape(WideStringObserver(pwcNameBegin, pwcRead));
		vecNodeStack.Push(&(deqChildren.back().second));
	};
	const auto PushUnnamedNode = [&]{
		ASSERT(!vecNodeStack.IsEmpty());

		auto &deqChildren = vecNodeStack.GetEnd()[-1]->m_deqChildren;
		deqChildren.emplace_back();
		vecNodeStack.Push(&(deqChildren.back().second));
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

	return std::make_pair(ERR_NONE, pwcRead);
}
WideString TExpression::Export(const WideStringObserver &wsoIndent) const {
	WideString wsRet;

	VVector<std::pair<const Node *, std::size_t>> vecNodeStack;
	vecNodeStack.Push(this, 0);
	WideString wsIndent;
	for(;;){
		auto &vTop = vecNodeStack.GetEnd()[-1];
		const auto &vTopNode = *(vTop.first);

	jNextChild:
		if(vTop.second < vTopNode.m_deqChildren.size()){
			const auto &vChild = vTopNode.m_deqChildren[vTop.second++];
			wsRet.Append(wsIndent);
			if(!vChild.first.IsEmpty()){
				Escape(wsRet, vChild.first);
				if(vChild.second.m_deqChildren.empty()){
					wsRet.Append(L'\n');
					goto jNextChild;
				}
			}
			wsRet.Append(L'(');
			if(vChild.second.m_deqChildren.empty()){
				wsRet.Append(L')');
				wsRet.Append(L'\n');
				goto jNextChild;
			}
			wsRet.Append(L'\n');
			wsIndent.Append(wsoIndent);
			vecNodeStack.Push(&(vChild.second), 0);
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

	return std::move(wsRet);
}
