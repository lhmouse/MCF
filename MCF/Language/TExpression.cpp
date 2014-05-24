// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TExpression.hpp"
using namespace MCF;

namespace {

WideString Unescape(const WideStringObserver &wsoSrc){
	WideString wcsRet;
	wcsRet.Reserve(wsoSrc.GetSize());

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
			wcsRet.Append(u32CodePoint);
		} else {
			u32CodePoint -= 0x10000;
			wcsRet.Append((u32CodePoint >> 10)   | 0xD800);
			wcsRet.Append((u32CodePoint & 0x3FF) | 0xDC00);
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
				wcsRet.Push(wc);
				break;
			}
			break;

		case ESCAPED:
			switch(wc){
			case L'n':
				wcsRet.Append(L'\n');
				eState = NORMAL;
				break;

			case L'b':
				wcsRet.Append(L'\b');
				eState = NORMAL;
				break;

			case L'r':
				wcsRet.Append(L'\r');
				eState = NORMAL;
				break;

			case L't':
				wcsRet.Append(L'\t');
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
				wcsRet.Append(wc);
				eState = NORMAL;
				break;
			}
			break;

		case UCS_CODE:
			{
				auto uHex = (unsigned int)(wc - L'0');
				do {
					if(uHex <= 9){
						break;
					}
					uHex += (unsigned int)(L'0' - L'A');
					if(uHex <= 5){
						uHex += 0x0A;
						break;
					}
					uHex += (unsigned int)(L'A' - L'a');
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

	return std::move(wcsRet);
}
void Escape(WideString &wcsAppendTo, const WideStringObserver &wsoSrc){
	const auto uSrcLength = wsoSrc.GetLength();
	wcsAppendTo.Reserve(wcsAppendTo.GetLength() + uSrcLength);

	for(std::size_t i = 0; i < uSrcLength; ++i){
		const auto wc = wsoSrc[i];

		switch(wc){
		case L'(':
		case L')':
		case L';':
		case L' ':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(wc);
			break;

		case L'\\':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'\\');
			break;

		case L'\"':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'\"');
			break;

		case L'\n':
			wcsAppendTo.Append(L'\\');
			wcsAppendTo.Append(L'n');
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
			wcsAppendTo.Append(wc);
			break;
		}
	}
}

}

// 其他非静态成员函数。
std::pair<TExpression::ErrorType, const wchar_t *> TExpression::Parse(const WideStringObserver &wsoData){
	m_vecChildren.Clear();

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

		auto wcsName = Unescape(WideStringObserver(pwcNameBegin, pwcRead));
		auto &vNewChild = vecNodeStack.GetEnd()[-1]->m_vecChildren.Push(std::move(wcsName), Node());
		vecNodeStack.Push(&(vNewChild.second));
	};
	const auto PushUnnamedNode = [&]{
		ASSERT(!vecNodeStack.IsEmpty());

		auto &vNewChild = vecNodeStack.GetEnd()[-1]->m_vecChildren.Push();
		vecNodeStack.Push(&(vNewChild.second));
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
	WideString wcsRet;

	VVector<std::pair<const Node *, std::size_t>> vecNodeStack;
	vecNodeStack.Push(this, 0);
	WideString wcsIndent;
	for(;;){
		auto &vTop = vecNodeStack.GetEnd()[-1];
		const auto &vTopNode = *(vTop.first);

	jNextChild:
		if(vTop.second < vTopNode.m_vecChildren.GetSize()){
			const auto &vChild = vTopNode.m_vecChildren[vTop.second++];
			wcsRet.Append(wcsIndent);
			if(!vChild.first.IsEmpty()){
				Escape(wcsRet, vChild.first);
				if(vChild.second.m_vecChildren.IsEmpty()){
					wcsRet.Append(L'\n');
					goto jNextChild;
				}
			}
			wcsRet.Append(L'(');
			if(vChild.second.m_vecChildren.IsEmpty()){
				wcsRet.Append(L')');
				wcsRet.Append(L'\n');
				goto jNextChild;
			}
			wcsRet.Append(L'\n');
			wcsIndent.Append(wsoIndent);
			vecNodeStack.Push(&(vChild.second), 0);
			continue;
		}

		vecNodeStack.Pop();
		if(vecNodeStack.IsEmpty()){
			break;
		}

		wcsIndent.Truncate(wsoIndent.GetLength());
		wcsRet.Append(wcsIndent);
		wcsRet.Append(L')');
		wcsRet.Append(L'\n');
	}

	ASSERT(wcsIndent.IsEmpty());

	return std::move(wcsRet);
}
