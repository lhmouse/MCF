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

#define DEF(id, contents)	\
	{ L ## id ## _wso, L ## contents ## _wso }

const std::map<MCF::WideStringObserver, MCF::WideStringObserver> g_mapStrings = {
										//2       10        20        30        40        50        60        70        80
	DEF("MCFBUILD_LOGO",				"MCF 构建实用工具 [版本 %0]\n"
										"Copyleft 2014, LH_Mouse. All wrongs reserved.\n"
										),
	DEF("MCFBUILD_USAGE",				"\n"
										"命令行参数可以以 - 或 / 开头，其效果是一样的。\n"
										"任何不以 - 或 / 开头的命令行参数都会被当作配置名。\n"
										"\n"
										"命令行选项（按相关性顺序）：\n"
										"\n"
										"  -h 或 -?      显示此帮助信息。\n"
										"  -M            使用原始格式输出信息。"
										"\n"
										"  -v            显示详细资料。\n"
										"\n"
										"  -C目录        在进行任何其他操作之前，切换工作目录到此路径。\n"
										"\n"
										"  -f文件        指定要调用的项目文件路径。\n"
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
										"  -j十进制整数  指定多进程构建时使用的进程数。0 为自动选择。\n"
										"                默认值是 0。\n"
										"  -c            执行清理操作，删除所有目标文件（不构建）。\n"
										"  -R            重新构建所有文件。\n"
										),

	DEF("ENVIRONMENT_MANIFEST",			"\n----- 构建环境清单 -----\n"
										"工作目录：%0\n"
										"项目文件：%1\n"
										"配置名　：%2\n"
										"源目录　：%3\n"
										"中间目录：%4\n"
										"目标目录：%5\n"
										"进程数　：%6\n"
										),
	DEF("BUILD_STARTED",				"\n----- 构建开始 -----\n"),
	DEF("BUILD_SUCCEEDED",				"\n----- 构建成功 -----\n"),
	DEF("BUILD_FAILED",					"\n----- 构建失败 -----\n"),

	DEF("LOADING_PROJECT_FILE",			"正在读取项目文件“%0”...\n"),
	DEF("PARSING_SOURCE_DIR_TREE",		"正在解析源文件目录树...\n"),
	DEF("GENERATING_FILE_LIST",			"正在生成文件列表...\n"),

	DEF("MCF_EXCEPTION",				"MCF Build 遇到一个异常：\n"
										"  错误描述：%0\n"
										"  错误代码：%1（%2）\n"
										),
	DEF("STD_EXCEPTION",				"MCF Build 遇到异常“%0”。\n请联系作者寻求协助。\n"),
	DEF("UNKNOWN_EXCEPTION",			"MCF Build 遇到无法处理的异常。\n请联系作者寻求协助。\n"),

	DEF("INVALID_COMMAND_LINE_OPTION",	"命令行选项“%0”无效。使用选项 -h 获得帮助。"),
	DEF("CREATE_PIPE_FAILED",			"创建管道失败。"),
	DEF("SET_PIPE_INFO_FAILED",			"设置管道句柄信息失败。"),
	DEF("EXECUTE_COMMAND_FAILED",		"执行命令“%1”失败。"),
	DEF("OPEN_DIRECTORY_FAILED",		"打开目录“%0”失败。"),
	DEF("OPEN_FILE_FAILED",				"打开文件“%0”失败。"),
	DEF("FILE_TOO_LARGE",				"文件“%0”太大。"),
	DEF("DELETE_FILE_FAILED",			"删除文件“%0”失败。"),
	DEF("SET_WORKING_DIR_FAILED",		"设置工作目录至“%0”失败。"),
	DEF("PROJ_FILE_INVALID",			"项目文件解析失败，错误 %0。"),
};

}

namespace MCFBuild {

MCF::WideString FormatString(const MCF::WideStringObserver &wsoRaw){
	if(Model::GetInstance().DoesUseRawOutput()){
		return MCF::WideString(wsoRaw);
	}

	MCF::VVector<std::size_t> vecDelims;
	for(std::size_t i = 0; i < wsoRaw.GetSize(); ++i){
		if(wsoRaw[i] == 0){
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
