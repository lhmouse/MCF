// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "ConsoleOutput.hpp"
#include "Localization.hpp"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/Exception.hpp"
#include <cwchar>
using namespace MCFBuild;

extern "C" unsigned int MCFMain()
try {
	auto &vModel = Model::GetInstance();

	PrintLn(FormatString(L"MCFBUILD_LOGO|0|4|0|0"_wso));

	vModel.InitParams();

	if(vModel.GetOperation() == Model::OP_SHOW_HELP){
		PrintLn(FormatString(L"MCFBUILD_USAGE"_wso));
		return 0;
	}

	MCF::WideString wcsProcessCount;
	wcsProcessCount.Resize(5);
	wcsProcessCount.Resize((unsigned int)std::swprintf(
		wcsProcessCount.GetStr(), wcsProcessCount.GetSize(),
		L"%lu",
		vModel.GetProcessCount()
	));
	PrintLn(FormatString(
		L"ENVIRONMENT_MANIFEST|"_wso
		+ vModel.GetWorkingDir() + L'|'
		+ vModel.GetProject() + L'|'
		+ vModel.GetConfig() + L'|'
		+ vModel.GetSrcRoot() + L'|'
		+ vModel.GetIntermediateRoot() + L'|'
		+ vModel.GetDstRoot() + L'|'
		+ wcsProcessCount
	));

	//

	return 0;
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
	PrintLn(FormatString(wcsMessage));
	return e.m_ulErrorCode;
} catch(std::exception &e){
	PrintLn(FormatString(L"STD_EXCEPTION|"_ws + MCF::Utf8String(e.what())));
	return (unsigned int)-1;
} catch(...){
	PrintLn(FormatString(L"UNKNOWN_EXCEPTION"_wso));
	return (unsigned int)-2;
}
