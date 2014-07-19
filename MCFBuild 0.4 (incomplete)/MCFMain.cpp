// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "ConsoleOutput.hpp"
#include "SHell.hpp"
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

	FormatPrint(L"MCFBUILD_LOGO\x000\x004\x000\x000"_wso);

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
		L"ENVIRONMENT_MANIFEST\0"_ws
		+ vModel.GetWorkingDir() + L'\0'
		+ vModel.GetProject() + L'\0'
		+ vModel.GetConfig() + L'\0'
		+ vModel.GetSrcRoot() + L'\0'
		+ vModel.GetIntermediateRoot() + L'\0'
		+ vModel.GetDstRoot() + L'\0'
		+ wcsProcessCount
	);

	FormatPrint(L"PREPARING_TO_BUILD\0"_wso);

	FormatPrint(L"LOADING_PROJECT_FILE\0"_wso);
	ProjectFile vProject(vModel.GetProject());

	MCF::WideString out, err;
	Shell(out, err, L"this_command_does_not_exist argument"_wso);
	Print(out);
	Print(err);

	return 0;
} catch(MCF::Exception &e){
	FormatPrint(L"EXCEPTION_HEADER"_wso);

	auto wcsMessage(L"MCF_EXCEPTION\0"_wso + e.m_wcsMessage);
	wchar_t awcCode[16];
	wcsMessage.Append(awcCode, (std::size_t)std::swprintf(awcCode, COUNT_OF(awcCode), L"|%lu\0", e.m_ulErrorCode));
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

	auto wcsMessage = L"STD_EXCEPTION\0"_ws;
	wcsMessage.Append<MCF::StringEncoding::UTF8>(MCF::Utf8StringObserver(e.what()));
	FormatPrint(wcsMessage);
	return (unsigned int)-1;
} catch(...){
	FormatPrint(L"EXCEPTION_HEADER"_wso);

	FormatPrint(L"UNKNOWN_EXCEPTION"_wso);
	return (unsigned int)-2;
}
