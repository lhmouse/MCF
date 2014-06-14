// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Model.hpp"
#include "FileSystem.hpp"
#include "../MCF/Core/System.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Core/MultiIndexedMap.hpp"
#include "../MCF/Core/File.hpp"
#include <cwchar>
using namespace MCFBuild;

namespace {

const auto				DEFAULT_PROJECT_NAME	= L"MCFBuild.mcfproj"_ws;
const auto				DEFAULT_CONFIG			= L"Default"_ws;
constexpr std::size_t	MAX_PROCESS_COUNT		= 17;

}

// 静态成员函数。
Model &Model::GetInstance() noexcept {
	static Model s_vModel;
	return s_vModel;
}

// 构造函数和析构函数。
Model::Model()
	: xm_vecArgV	(MCF::GetArgV())
	, xm_bRawOutput	(false)
{
	for(std::size_t i = 1; i < xm_vecArgV.GetSize(); ++i){
		const auto &wcsArg = xm_vecArgV[i];
		if((wcsArg == L"-M"_wso) || (wcsArg == L"/M"_wso)){
			xm_bRawOutput = true;
		}
	}
}

// 其他非静态成员函数。
void Model::InitParams(){
	xm_bVerbose = false;

	xm_wcsProject.Clear();
	xm_wcsConfig.Clear();
	xm_mapMacros.clear();

	xm_wcsWorkingDir.Clear();
	xm_wcsSrcRoot.Clear();
	xm_wcsIntermediateRoot.Clear();
	xm_wcsDstRoot.Clear();

	xm_eOperation = OP_BUILD;
	xm_uProcessCount = 0;

	xm_setSkippedDependencies.clear();

	for(std::size_t i = 1; i < xm_vecArgV.GetSize(); ++i){
		const auto &wcsArg = xm_vecArgV[i];
		const auto uArgLen = wcsArg.GetSize();
		if(uArgLen == 0){
			continue;
		}

#define THROW_INV_PARAM	\
	FORMAT_THROW(ERROR_INVALID_PARAMETER, L"INVALID_COMMAND_LINE_OPTION|"_wso + wcsArg)

		if((wcsArg[0] == L'-') || (wcsArg[0] == L'/')){
			if(uArgLen <= 1){
				continue;
			}
			switch(wcsArg[1]){
			case L'h':
			case L'?':
				if(uArgLen > 2){
					THROW_INV_PARAM;
				}
				xm_eOperation = OP_SHOW_HELP;
				break;

			case L'M':
				if(uArgLen > 2){
					THROW_INV_PARAM;
				}
				break;

			case L'v':
				if(uArgLen > 2){
					THROW_INV_PARAM;
				}
				xm_bVerbose = true;
				break;

			case L'C':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				}
				xm_wcsWorkingDir.Assign(wcsArg.GetBegin() + 2, wcsArg.GetEnd());
				break;

			case L'p':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				}
				xm_wcsProject.Assign(wcsArg.GetBegin() + 2, wcsArg.GetEnd());
				break;

			case L'D':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				} else {
					MCF::WideString wcsMacroName(wcsArg.GetBegin(), uArgLen - 2);
					MCF::WideString wcsMacroValue;

					const auto uEquPos = wcsMacroName.Find(L'=');
					if(uEquPos != wcsMacroName.NPOS){
						wcsMacroValue.Assign(wcsMacroName.GetBegin() + uEquPos + 1, wcsMacroName.GetEnd());
						wcsMacroName.Resize(uEquPos);
					}

					xm_mapMacros[std::move(wcsMacroName)] = std::move(wcsMacroValue);
				}
				break;

			case L's':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				}
				xm_wcsSrcRoot.Assign(wcsArg.GetBegin() + 2, wcsArg.GetEnd());
				break;

			case L'i':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				}
				xm_wcsIntermediateRoot.Assign(wcsArg.GetBegin() + 2, wcsArg.GetEnd());
				break;

			case L'd':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				}
				xm_wcsDstRoot.Assign(wcsArg.GetBegin() + 2, wcsArg.GetEnd());
				break;

			case L'c':
				if(uArgLen > 2){
					THROW_INV_PARAM;
				}
				xm_eOperation = OP_CLEAN;
				break;

			case L'j':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				} else {
					wchar_t *pwcEnd;
					xm_uProcessCount = std::wcstoul(wcsArg.GetCStr() + 2, &pwcEnd, 10);
					if(pwcEnd != wcsArg.GetEnd()){
						THROW_INV_PARAM;
					}
					if(xm_uProcessCount > MAX_PROCESS_COUNT){
						xm_uProcessCount = MAX_PROCESS_COUNT;
					}
				}
				break;

			case L'r':
				if(uArgLen > 2){
					THROW_INV_PARAM;
				}
				xm_eOperation = OP_REBUILD;
				break;

			case L'X':
				if(uArgLen <= 2){
					THROW_INV_PARAM;
				} else {
					MCF::File::UniqueId vUniqueId;
					wchar_t *pwcEnd;
					vUniqueId.u32VolumeSN = std::wcstoul(wcsArg.GetCStr() + 2, &pwcEnd, 16);
					if(*pwcEnd != L'.'){
						THROW_INV_PARAM;
					}
					vUniqueId.u32IndexLow = std::wcstoul(pwcEnd + 1, &pwcEnd, 16);
					if(*pwcEnd != L'.'){
						THROW_INV_PARAM;
					}
					vUniqueId.u32IndexHigh = std::wcstoul(pwcEnd + 1, &pwcEnd, 16);
					if(*pwcEnd != 0){
						THROW_INV_PARAM;
					}
					vUniqueId.u32Reserved = 0;
					xm_setSkippedDependencies.insert(vUniqueId);
				}
				break;

			default:
				THROW_INV_PARAM;
			}
		} else {
			xm_wcsConfig = wcsArg;
		}
	}

	if(!xm_wcsWorkingDir.IsEmpty()){
		if(!::SetCurrentDirectoryW(xm_wcsWorkingDir.GetCStr())){
			FORMAT_THROW(::GetLastError(), L"SET_WORKING_DIR_FAILED|"_wso + xm_wcsWorkingDir);
		}
	}
	xm_wcsWorkingDir = GetFullPath(L"."_ws);

	if(xm_wcsProject.IsEmpty()){
		xm_wcsProject = GetFullPath(DEFAULT_PROJECT_NAME);
	} else {
		xm_wcsProject = GetFullPath(xm_wcsProject);
	}
	if(xm_wcsConfig.IsEmpty()){
		xm_wcsConfig = DEFAULT_CONFIG;
	}

	if(xm_wcsSrcRoot.IsEmpty()){
		xm_wcsSrcRoot = GetFullPath(L"."_ws);
	} else {
		xm_wcsSrcRoot = GetFullPath(xm_wcsSrcRoot);
	}
	if(xm_wcsIntermediateRoot.IsEmpty()){
		xm_wcsIntermediateRoot = GetFullPath(L".Intermediate-"_wso + xm_wcsConfig);
	} else {
		xm_wcsIntermediateRoot = GetFullPath(xm_wcsSrcRoot);
	}
	if(xm_wcsDstRoot.IsEmpty()){
		xm_wcsDstRoot = GetFullPath(L".Built-"_ws + xm_wcsConfig);
	} else {
		xm_wcsDstRoot = GetFullPath(xm_wcsDstRoot);
	}

	if(xm_uProcessCount == 0){
		SYSTEM_INFO vSystemInfo;
		::GetSystemInfo(&vSystemInfo);
		xm_uProcessCount = vSystemInfo.dwNumberOfProcessors + 1;
		if(xm_uProcessCount > MAX_PROCESS_COUNT){
			xm_uProcessCount = MAX_PROCESS_COUNT;
		}
	}
}
