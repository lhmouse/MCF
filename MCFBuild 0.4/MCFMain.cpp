// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "ConsoleOutput.hpp"
#include "Localization.hpp"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Core/Exception.hpp"
#include <cwchar>
using namespace MCFBuild;

extern "C" unsigned int MCFMain()
try {
	PrintLn(FormatString(L"MCFBUILD_LOGO|0|4|0|0"_wso));
	PrintLn();

	return Model::GetInstance().Run();
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
