// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "ConsoleOutput.hpp"
#include "../MCFCRT/exe/exe_decl.h"
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

	MCF::WideString wsProcessCount;
	wsProcessCount.Resize(7);
	wsProcessCount.Resize((unsigned int)std::swprintf(
		wsProcessCount.GetStr(), wsProcessCount.GetSize(),
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
		+ wsProcessCount
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
	auto wsMessage(L"MCF_EXCEPTION\0"_wso + e.m_wcsMessage);
	wsMessage.Push(0);
	wchar_t awcCode[16];
	// TODO
	wsMessage.Append(awcCode, (std::size_t)std::swprintf(awcCode, COUNT_OF(awcCode), L"%lu", e.m_ulErrorCode) + 1);
	wsMessage += MCF::GetWin32ErrorDesc(e.m_ulErrorCode);
	FormatPrint(wsMessage);
	return e.m_ulErrorCode;
} catch(std::exception &e){
	auto wsMessage = L"STD_EXCEPTION\0"_ws;
	wsMessage.Append<MCF::StringEncoding::UTF8>(MCF::Utf8StringObserver(e.what()));
	FormatPrint(wsMessage);
	return (unsigned int)-1;
} catch(...){
	FormatPrint(L"UNKNOWN_EXCEPTION"_wso);
	return (unsigned int)-2;
}
