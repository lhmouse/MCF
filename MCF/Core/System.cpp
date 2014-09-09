// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "System.hpp"
#include <cwchar>
using namespace MCF;

namespace MCF {

VVector<WideString> GetArgV(){
	VVector<WideString> vecRet;

	enum tagState {
		DELIM,
		IN_ARG,
		QUOTE_BEGIN,
		IN_QUOTE,
		QUOTE_END,
		QUOTE_AFTER_QUOTE
	} eState = DELIM;

	const WideStringObserver wsoCommandLine(::GetCommandLineW());
	auto pwcRead = wsoCommandLine.GetBegin();
	const auto pwcEnd = wsoCommandLine.GetEnd();
	while(pwcRead != pwcEnd){
		const wchar_t wc = *(pwcRead++);

		switch(eState){
		case DELIM:
			switch(wc){
			case L' ':
			case L'\t':
				// eState = DELIM;
				break;

			case L'\"':
				vecRet.Push();
				eState = QUOTE_BEGIN;
				break;

			default:
				vecRet.Push();
				eState = IN_ARG;
				break;
			}
			break;

		case IN_ARG:
			switch(wc){
			case L' ':
			case L'\t':
				eState = DELIM;
				break;

			case L'\"':
				eState = QUOTE_BEGIN;
				break;

			default:
				// eState = IN_ARG;
				break;
			}
			break;

		case QUOTE_BEGIN:
			switch(wc){
			case L'\"':
				eState = QUOTE_END;
				break;

			default:
				eState = IN_QUOTE;
				break;
			}
			break;

		case IN_QUOTE:
			switch(wc){
			case L'\"':
				eState = QUOTE_END;
				break;

			default:
				// eState = IN_QUOTE;
				break;
			}
			break;

		case QUOTE_END:
			switch(wc){
			case L' ':
			case L'\t':
				eState = DELIM;
				break;

			case L'\"':
				eState = QUOTE_AFTER_QUOTE;
				break;

			default:
				eState = IN_ARG;
				break;
			}
			break;

		case QUOTE_AFTER_QUOTE:
			switch(wc){
			case L'\"':
				eState = QUOTE_END;
				break;

			default:
				eState = IN_QUOTE;
				break;
			}
			break;
		}
		switch(eState){
		case DELIM:
		case QUOTE_BEGIN:
		case QUOTE_END:
			break;

		case IN_ARG:
		case IN_QUOTE:
		case QUOTE_AFTER_QUOTE:
			ASSERT(!vecRet.IsEmpty());

			vecRet.GetEnd()[-1].Push(wc);
			break;
		}
	}

	return std::move(vecRet);
}

}
