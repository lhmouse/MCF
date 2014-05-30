// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Localization.hpp"
#include "../MCF/Core/VVector.hpp"
#include <cstdlib>
#include <map>
using namespace MCFBuild;

namespace {

const std::map<MCF::WideStringObserver, MCF::WideStringObserver> g_mapStrings = {
	{ L"CREATE_PIPE_FAILED"_WSO,			L"创建管道失败。"_WSO },
	{ L"SET_PIPE_INFO_FAILED"_WSO,			L"设置管道句柄信息失败。"_WSO },
	{ L"CREATE_PROCESS_FAILED"_WSO,			L"以命令行“%0”创建进程失败。"_WSO },
	{ L"ACCESS_DIRECTORY_FAILED"_WSO,		L"访问目录“%0”失败。"_WSO },
	{ L"FILE_TOO_LARGE"_WSO,				L"文件“%0”太大。"_WSO }
};

}

namespace MCFBuild {

MCF::WideString FormatString(const MCF::WideStringObserver &wsoRaw){
	MCF::VVector<std::size_t> vecDelims;
	for(std::size_t i = 0; i < wsoRaw.GetSize(); ++i){
		if(wsoRaw[i] == L'|'){
			vecDelims.Push(i);
		}
	}
	vecDelims.Push(wsoRaw.GetSize());

	const auto wsoId = wsoRaw.Slice(0, (std::ptrdiff_t)vecDelims[0]);
	const auto itDesc = g_mapStrings.find(wsoId);
	if(itDesc == g_mapStrings.end()){
		return L"MissingText: "_WS +  wsoId;
	}

	MCF::WideString wcsRet;
	bool bExpectingIndex = false;
	for(const auto wc : itDesc->second){
		if(bExpectingIndex){
			bExpectingIndex = false;

			if(wc == L'%'){
				wcsRet.Push(L'%');
				continue;
			} else if((L'0' <= wc) || (wc <= L'9')){
				const auto uIndex = (std::size_t)(wc - L'0');
				if(uIndex + 1 < vecDelims.GetSize()){
					wcsRet.Append(wsoRaw.Slice(
						(std::ptrdiff_t)vecDelims[uIndex] + 1,
						(std::ptrdiff_t)vecDelims[uIndex + 1]
					));
				}
				continue;
			}
		}
		if(wc == L'%'){
			bExpectingIndex = true;
			continue;
		}
		wcsRet.Push(wc);
	}
	return std::move(wcsRet);
}

}
