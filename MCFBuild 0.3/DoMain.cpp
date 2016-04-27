// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"

namespace MCFBuild {
	extern void DoBuild(
		const wvstring &wcsProjFile,
		const wvstring &wcsConfig,
		const wvstring &wcsSrcRoot,
		const wvstring &wcsDstRoot,
		const wvstring &wcsOutputPath,
		unsigned long ulProcessCount,
		const std::map<wvstring, wvstring> &mapVars,
		bool bVerbose,
		bool bRebuildAll
	);

	void DoMain(std::size_t argc, const wchar_t *const *argv){
		bool bShowHelp = false;
		wvstring wcsProjFile;
		wvstring wcsConfig;
		wvstring wcsSwitchToPath;
		wvstring wcsSrcRoot;
		wvstring wcsDstRoot;
		wvstring wcsOutputPath;
		unsigned long ulProcessCount = 0;
		std::map<wvstring, wvstring> mapVars;
		bool bVerbose = false;
		bool bRebuildAll = false;

		for(std::size_t i = 1; i < argc; ++i){
			const wchar_t *const pwszParam = argv[i];
			if(pwszParam[0] != L'-'){
				wcsConfig = pwszParam;
				continue;
			}
			switch(pwszParam[1]){
			case L'h':
				if(pwszParam[2] != 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”无效。"};
				}
				bShowHelp = true;
				break;
			case L'p':
				if(pwszParam[2] == 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少路径。"};
				}
				wcsProjFile = pwszParam + 2;
				break;
			case L'C':
				if(pwszParam[2] == 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少路径。"};
				}
				wcsSwitchToPath = pwszParam + 2;
				break;
			case L's':
				if(pwszParam[2] == 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少路径。"};
				}
				wcsSrcRoot = pwszParam + 2;
				break;
			case L'd':
				if(pwszParam[2] == 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少路径。"};
				}
				wcsDstRoot = pwszParam + 2;
				break;
			case L'o':
				if(pwszParam[2] == 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少路径。"};
				}
				wcsOutputPath = pwszParam + 2;
				break;
			case L'j':
				{
					if(pwszParam[2] == 0){
						throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少数值。"};
					}
					wchar_t *pwszEnd;
					ulProcessCount = std::wcstoul(pwszParam + 2, &pwszEnd, 10);
					if(pwszEnd[0] != 0){
						throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”数值无效。"};
					}
				} break;
			case L'D':
				{
					if(pwszParam[2] == 0){
						throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少变量名。"};
					}
					const wchar_t *const pwchEquPos = std::wcschr(pwszParam + 2, L'=');
					if(pwchEquPos == nullptr){
						mapVars.emplace(wvstring(pwszParam + 2), wvstring(1, L'1'));
					} else if(pwchEquPos == pwszParam + 2){
						throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”缺少变量名。"};
					} else {
						mapVars.emplace(wvstring(pwszParam + 2, pwchEquPos), wvstring(pwchEquPos + 1));
					}
				} break;
			case L'v':
				if(pwszParam[2] != 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”无效。"};
				}
				bVerbose = true;
				break;
			case L'R':
				if(pwszParam[2] != 0){
					throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”无效。"};
				}
				bRebuildAll = true;
				break;
			default:
				throw Exception{ERROR_INVALID_PARAMETER, wvstring(L"参数“") + pwszParam + L"”无效。"};
			}
		}

		if(bShowHelp){
			Output(L"命令行：");
			Output(L"MCFBuild [-h] [-p项目文件] [配置包] [-C临时切换至路径]");
			Output(L"         [-s源文件根目录] [-d目标文件根目录] [-o输出文件路径]");
			Output(L"         [-j进程数] [-D变量名[=值]] [-v] [-R]");
			Output();
			Output(L"  -h                显示此帮助信息。");
			Output(L"  -p项目文件        指定保存有项目配置信息的文件。");
			Output(L"                    如果使用相对路径，MCFBuild 将在当前目录中搜索。");
			Output(L"                    此参数的默认值是 [源文件根目录]\\MCFBuild.mcfproj。");
			Output(L"  配置包            指定使用的配置包。配置包必须在项目文件中定义。");
			Output(L"                    此参数的默认值是 Default。");
			Output(L"  -C临时切换至路径  在构建以前切换工作目录到这个路径。");
			Output(L"  -s源文件根目录    指定所有源文件的位置。");
			Output(L"                    MCFBuild 以该目录为根节点递归构建所有文件。");
			Output(L"                    此参数的默认值是当前目录。");
			Output(L"  -d目标文件根目录  指定生成的目标文件（object files）的根目录。");
			Output(L"                    MCFBuild 在此目录中建立一个和源文件目录结构相同的目录树。");
			Output(L"                    此参数的默认值是当前目录下的 .Built-[配置包]。");
			Output(L"  -o输出文件路径    指定最终生成的二进制文件相对于当前目录的路径。");
			Output(L"                    如果省略此参数，将使用项目文件中定义的缺省输出路径。");
			Output(L"                    如果项目文件中没有定义缺省输出路径，则必须指定此参数。");
			Output(L"  -j进程数          指定多进程编译时的并发进程数。");
			Output(L"                    如果设为 0，MCFBuild 将根据当前 CPU 的逻辑核心数自动选择合适");
			Output(L"                    的值。");
			Output(L"                    此参数的默认值是 0。");
			Output(L"  -D变量名[=值]     定义一个变量。如果指定的配置包中有同名变量，处理规则和配置包");
			Output(L"                    继承时遇到同名变量的处理规则一致。");
			Output(L"  -v                显示生成细节。");
			Output(L"  -R                重新生成所有文件。");
			Output();
			Output(L"除路径以外，如无特殊说明，其他参数都是大小写敏感的。");
			return;
		}

		if(!wcsSwitchToPath.empty()){
			if(::SetCurrentDirectoryW(wcsSwitchToPath.c_str()) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"切换到目录“" + wcsSwitchToPath + L"”失败。"};
			}
		}

		if(wcsConfig.empty()){
			wcsConfig = L"Default";
		}

		if(wcsSrcRoot.empty()){
			wcsSrcRoot.push_back(L'.');
		}
		FixPath(wcsSrcRoot);
		if(wcsSrcRoot.back() != L'\\'){
			wcsSrcRoot.push_back(L'\\');
		}

		if(wcsProjFile.empty()){
			wcsProjFile = wcsSrcRoot + L"MCFBuild.mcfproj";
		}
		FixPath(wcsProjFile);

		if(wcsDstRoot.empty()){
			wcsDstRoot = L".\\.Built-" + wcsConfig;
		}
		FixPath(wcsDstRoot);
		if(wcsDstRoot.back() != L'\\'){
			wcsDstRoot.push_back(L'\\');
		}

		if(wcsOutputPath.empty()){
			wcsOutputPath = wcsDstRoot;
		} else {
			const bool bEndsWithSlash = (wcsOutputPath.back() == L'\\') || (wcsOutputPath.back() == L'/');
			FixPath(wcsOutputPath);
			if(bEndsWithSlash){
				wcsOutputPath.push_back(L'\\');
			}
		}

		if(ulProcessCount == 0){
			SYSTEM_INFO SystemInfo;
			::GetSystemInfo(&SystemInfo);
			ulProcessCount = SystemInfo.dwNumberOfProcessors + 1;
		}
		if(ulProcessCount > 16){
			ulProcessCount = 16;
		}

		Output(L"项目文件　　　：" + wcsProjFile);
		Output(L"配置包　　　　：" + wcsConfig);
		Output(L"源文件根目录　：" + wcsSrcRoot);
		Output(L"目标文件根目录：" + wcsDstRoot);

		if(wcsOutputPath.back() == L'\\'){
			Output(L"输出文件路径　：<使用项目缺省值>");
		} else {
			Output(L"输出文件路径　：" + wcsOutputPath);
		}

		Output(L"进程数　　　　：%lu", ulProcessCount);

		if(!mapVars.empty()){
			Output(L"定义的变量：");
			for(const auto &var : mapVars){
				Output(L"  " + var.first + L" = " + var.second);
			}
		}
		Output(L"========================================");

		DoBuild(wcsProjFile, wcsConfig, wcsSrcRoot, wcsDstRoot, wcsOutputPath, ulProcessCount, mapVars, bVerbose, bRebuildAll);
	}
}
