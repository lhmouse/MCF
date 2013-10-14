// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"

namespace MCFBuild {
	extern PROJECT SetupEnv(
		const std::wstring &wcsProjFile,
		const std::wstring &wcsConfig,
		const std::map<std::wstring, std::wstring> &mapVars,
		const std::wstring &wcsOutputPath,
		bool bVerbose
	);
	extern FOLDER_TREE GetSourceTree(
		const std::wstring &wcsPath,
		const PROJECT &Project,
		bool bVerbose
	);
	extern BUILD_JOBS CountBuildJobs(
		const std::wstring &wcsSrcRoot,
		const FOLDER_TREE &SrcTree,
		const std::wstring &wcsDstRoot,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bRebuildAll,
		bool bVerbose
	);
	extern void Compile(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bVerbose
	);
	extern void Link(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		bool bVerbose
	);

	void DoBuild(
		const std::wstring &wcsProjFile,
		const std::wstring &wcsConfig,
		const std::wstring &wcsSrcRoot,
		const std::wstring &wcsDstRoot,
		const std::wstring &wcsOutputPath,
		unsigned long ulProcessCount,
		const std::map<std::wstring, std::wstring> &mapVars,
		bool bVerbose,
		bool bRebuildAll
	){
		Output(L"正在配置构建环境...");
		const auto Project = SetupEnv(wcsProjFile, wcsConfig, mapVars, wcsOutputPath, bVerbose);
		Output(L"----------------------------------------");
		Output(L"正在分析源文件目录树...");
		const auto SourceTree = GetSourceTree(wcsSrcRoot, Project, bVerbose);
		Output(L"----------------------------------------");
		Output(L"正在使用 %lu 个进程统计构建任务...", ulProcessCount);
		const auto BuildJobs = CountBuildJobs(wcsSrcRoot, SourceTree, wcsDstRoot, Project, ulProcessCount, bRebuildAll, bVerbose);
		Output(L"----------------------------------------");
		Output(L"正在使用 %lu 个进程编译...", ulProcessCount);
		Compile(BuildJobs, Project, ulProcessCount, bVerbose);
		Output(L"----------------------------------------");
		Output(L"正在链接...");
		Link(BuildJobs, Project, bVerbose);
	}
}
