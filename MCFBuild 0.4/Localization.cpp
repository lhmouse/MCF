// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Localization.hpp"
#include "Model.hpp"
#include "../MCF/Core/VVector.hpp"
#include <cstdlib>
#include <map>
using namespace MCFBuild;

namespace {

const std::map<MCF::WideStringObserver, MCF::WideStringObserver> g_mapStrings = {
	{ L"MCFBUILD_LOGO"_wso,					L"MCF 构建实用工具 [版本 %0.%1.%2.%3]\n"
											"Copyleft 2014, LH_Mouse. All wrongs reserved.\n"_wso },
	{ L"MCFBUILD_USAGE"_wso,				L"" //     10        20        30        40        50        60        70        80
											"命令行参数可以以 - 或 / 开头，其效果是一样的。\n"
											"任何不以 - 或 / 开头的命令行参数都会被当作配置名。\n"
											"\n"
											"命令行选项（按相关性顺序）：\n"
											"\n"
											"  -h 或 -?      显示此帮助信息。\n"
											"  -M            使用原始格式输出消息。"
											"\n"
											"  -v            显示详细资料。\n"
											"\n"
											"  -C目录        在进行任何其他操作之前，切换工作目录到此路径。\n"
											"\n"
											"  -p文件        指定要调用的项目文件路径。\n"
											"                默认值是“MCFBuild.mcfproj”。\n"
											"  配置名        指定要调用的配置包的名称。区分大小写。\n"
											"                默认值是“Default”。\n"
											"  -D名称[=值]   定义一个宏。\n"
											"                命令行内定义的宏比配置包内定义的宏优先级高。\n"
											"\n"
											"  -s目录        指定源文件根目录。\n"
											"                默认值是当前目录。\n"
											"  -i目录        指定中间文件根目录。\n"
											"                默认值是“.Intermediate-配置名”。\n"
											"  -d目录        指定最终文件根目录。\n"
											"                默认值是“.Built-配置名”。\n"
											"\n"
											"  -j十进制整数  指定多进程构建时使用的进程数。0 为自动选择.\n"
											"                默认值是 0。\n"
											"  -c            执行清理操作，删除所有目标文件（不构建）。\n"
											"  -R            重新构建所有文件。\n"
											"\n"
											"  -X[序列号]    排除依赖项目（仅供内部使用）。"_wso },

	{ L"ENVIRONMENT_MANIFEST"_wso,			L"----- 构建环境清单 -----\n"
											L"工作目录：%0\n"
											L"项目文件：%1\n"
											L"配置名　：%2\n"
											L"源目录　：%3\n"
											L"中间目录：%4\n"
											L"目标目录：%5\n"
											L"进程数　：%6"_wso },

	{ L"MCF_EXCEPTION"_wso,					L"MCF Build 遇到一个错误：\n"
											"  错误描述：%0\n"
											"  错误代码：%1（%2）"_wso },
	{ L"STD_EXCEPTION"_wso,					L"MCF Build 遇到异常“%0”。请联系作者寻求协助。"_wso },
	{ L"UNKNOWN_EXCEPTION"_wso,				L"MCF Build 遇到无法处理的异常。请联系作者寻求协助。"_wso },

	{ L"INVALID_COMMAND_LINE_OPTION"_wso,	L"命令行选项“%0”无效。"_wso },
	{ L"CREATE_PIPE_FAILED"_wso,			L"创建管道失败。"_wso },
	{ L"SET_PIPE_INFO_FAILED"_wso,			L"设置管道句柄信息失败。"_wso },
	{ L"CREATE_PROCESS_FAILED"_wso,			L"以命令行“%0”创建进程失败。"_wso },
	{ L"OPEN_DIRECTORY_FAILED"_wso,			L"打开目录“%0”失败。"_wso },
	{ L"OPEN_FILE_FAILED"_wso,				L"打开文件“%0”失败。"_wso },
	{ L"FILE_TOO_LARGE"_wso,				L"文件“%0”太大。"_wso },
	{ L"DELETE_FILE_FAILED"_wso,			L"删除文件“%0”失败。"_wso },
	{ L"SET_WORKING_DIR_FAILED"_wso,		L"设置工作目录至“%0”失败。"_wso },
};

}

namespace MCFBuild {

MCF::WideString FormatString(const MCF::WideStringObserver &wsoRaw){
	if(Model::GetInstance().DoesUseRawOutput()){
		return MCF::WideString(wsoRaw);
	}

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
		return L"MissingText: "_ws +  wsoId;
	}

	MCF::WideString wcsRet;
	bool bExpectingIndex = false;
	for(const auto wc : itDesc->second){
		if(bExpectingIndex){
			bExpectingIndex = false;

			if(wc == L'%'){
				wcsRet.Push(L'%');
				continue;
			}

			std::ptrdiff_t nIndex = -2;
			if((L'0' <= wc) || (wc <= L'9')){
				nIndex = wc - L'0';
			} else if((L'A' <= wc) || (wc <= L'Z')){
				nIndex = wc - L'A' + 10;
			} else if((L'a' <= wc) || (wc <= L'z')){
				nIndex = wc - L'a' + 36;
			}
			if((std::size_t)(nIndex + 1) < vecDelims.GetSize()){
				const auto uInsertBegin = vecDelims[(std::size_t)nIndex] + 1;
				const auto uInsertEnd = vecDelims[(std::size_t)nIndex + 1];
				wcsRet.Append(wsoRaw.GetBegin() + uInsertBegin, uInsertEnd - uInsertBegin);
			}
			continue;
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
