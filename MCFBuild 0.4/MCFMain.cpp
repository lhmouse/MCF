// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCFCRT/exe/exe_include.h"
#include <cwchar>
using namespace MCFBuild;

namespace MCFBuild {

extern unsigned int WorkerEntry(const MCF::VVector<MCF::WideStringObserver> &vecArgs);

}

extern "C" unsigned int MCFMain()
try {
	PrintLn(FormatString(L"MCFBUILD_LOGO|0|4|0|0"_wso));
	PrintLn();

	const ARG_ITEM *pArgV;
	const auto uArgC = ::MCF_GetArgV(&pArgV);

	MCF::VVector<MCF::WideStringObserver> vecArgs;
	vecArgs.Reserve(uArgC);
	for(std::size_t i = 0; i < uArgC; ++i){
		vecArgs.PushNoCheck(MCF::WideStringObserver(pArgV[i].pwszBegin, pArgV[i].uLen));
	}
	return WorkerEntry(vecArgs);
} catch(MCF::Exception &e){
	auto wcsMessage(L"MCF_EXCEPTION|"_wso + e.m_wcsMessage);
	wchar_t awcCode[16];
	wcsMessage.Append(awcCode, (std::size_t)std::swprintf(awcCode, COUNT_OF(awcCode), L"|%lu|", e.m_ulErrorCode));
	auto wcsErrorDescription = MCF::GetWin32ErrorDesc(e.m_ulErrorCode);
	for(;;){
		if(wcsErrorDescription.IsEmpty()){
			break;
		}
		const auto wc = wcsErrorDescription.GetEnd()[-1];
		if((wc != L'\n') && (wc != L'\r')){
			break;
		}
		wcsErrorDescription.Pop();
	}
	wcsMessage += wcsErrorDescription;
	PrintLn();
	PrintLn(FormatString(wcsMessage));
	return e.m_ulErrorCode;
} catch(std::exception &e){
	PrintLn();
	PrintLn(FormatString(L"STD_EXCEPTION|"_ws + MCF::Utf8String(e.what())));
	return (unsigned int)-1;
} catch(...){
	PrintLn();
	PrintLn(FormatString(L"UNKNOWN_EXCEPTION"_wso));
	return (unsigned int)-2;
}
