// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "ConsoleOutput.hpp"
#include "../MCFCRT/exe/exe_decl.h"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/Exception.hpp"
#include <cwchar>
using namespace MCFBuild;

namespace MCFBuild {

extern void Build();

}

extern "C" unsigned int MCFMain() noexcept
try {
	auto &vModel = Model::GetInstance();

	FormatPrint(L"MCFBUILD_LOGO\0" "0.4.0.0"
#ifndef NDEBUG
		" Debug"
#endif
		""_wso);

	vModel.InitParams();

	if(vModel.GetOperation() == Model::OP_SHOW_HELP){
		FormatPrint(L"MCFBUILD_USAGE"_wso);
		return 0;
	}

	MCF::WideString wcsProcessCount;
	wcsProcessCount.Resize(7);
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

	try {
		FormatPrint(L"BUILD_STARTED\0"_wso);
		Build();
		FormatPrint(L"BUILD_SUCCEEDED\0"_wso);
	} catch(...){
		FormatPrint(L"BUILD_FAILED\0"_wso);
		throw;
	}

	return 0;
} catch(MCF::Exception &e){
	auto wcsMessage(L"MCF_EXCEPTION\0"_wso + e.m_wcsMessage);
	wcsMessage.Push(0);
	wchar_t awcCode[16];
	// TODO
	wcsMessage.Append(awcCode, (std::size_t)std::swprintf(awcCode, COUNT_OF(awcCode), L"%lu", e.m_ulErrorCode) + 1);
	wcsMessage += MCF::GetWin32ErrorDesc(e.m_ulErrorCode);
	FormatPrint(wcsMessage);
	return e.m_ulErrorCode;
} catch(std::exception &e){
	auto wcsMessage = L"STD_EXCEPTION\0"_ws;
	wcsMessage.Append<MCF::StringEncoding::UTF8>(MCF::Utf8StringObserver(e.what()));
	FormatPrint(wcsMessage);
	return (unsigned int)-1;
} catch(...){
	FormatPrint(L"UNKNOWN_EXCEPTION"_wso);
	return (unsigned int)-2;
}
