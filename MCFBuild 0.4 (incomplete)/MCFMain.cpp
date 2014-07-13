// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "ConsoleOutput.hpp"
#include "ProjectFile.hpp"
#include "../MCFCRT/exe/exe_decl.h"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/Exception.hpp"
#include <cwchar>
using namespace MCFBuild;

extern "C" unsigned int MCFMain() noexcept
try {
	auto &vModel = Model::GetInstance();

	FormatPrint(L"MCFBUILD_LOGO|0|4|0|0"_wso);

	vModel.InitParams();

	if(vModel.GetOperation() == Model::OP_SHOW_HELP){
		FormatPrint(L"MCFBUILD_USAGE"_wso);
		return 0;
	}

	MCF::WideString wcsProcessCount;
	wcsProcessCount.Resize(5);
	wcsProcessCount.Resize((unsigned int)std::swprintf(
		wcsProcessCount.GetStr(), wcsProcessCount.GetSize(),
		L"%lu",
		vModel.GetProcessCount()
	));
	FormatPrint(
		L"ENVIRONMENT_MANIFEST|"_ws
		+ vModel.GetWorkingDir() + L'|'
		+ vModel.GetProject() + L'|'
		+ vModel.GetConfig() + L'|'
		+ vModel.GetSrcRoot() + L'|'
		+ vModel.GetIntermediateRoot() + L'|'
		+ vModel.GetDstRoot() + L'|'
		+ wcsProcessCount
	);

	FormatPrint(L"LOADING_PROJECT_FILE|"_wso);
	ProjectFile vProject(vModel.GetProject());

	return 0;
} catch(MCF::Exception &e){
	FormatPrint(L"EXCEPTION_HEADER"_wso);

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
	FormatPrint(wcsMessage);
	return e.m_ulErrorCode;
} catch(std::exception &e){
	FormatPrint(L"EXCEPTION_HEADER"_wso);

	auto wcsMessage = L"STD_EXCEPTION|"_ws;
	wcsMessage.Append<MCF::StringEncoding::UTF8>(MCF::Utf8StringObserver(e.what()));
	FormatPrint(wcsMessage);
	return (unsigned int)-1;
} catch(...){
	FormatPrint(L"EXCEPTION_HEADER"_wso);

	FormatPrint(L"UNKNOWN_EXCEPTION"_wso);
	return (unsigned int)-2;
}
