// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "PreCompiled.hpp"
#include <climits>
#include <map>
#include <algorithm>
#include <iterator>
#include "JobScheduler.hpp"
#include "ProcessProxy.hpp"
using namespace MCFBuild;

namespace {
	void CountCompilableFiles(
		std::list<std::pair<std::wstring, long long>> &lstCompilableFiles,
		const std::wstring &wcsSrcRoot,
		const FOLDER_TREE &SrcTree,
		const std::wstring &wcsPrefix,
		const PROJECT &Project
	){
		for(const auto &val : SrcTree.mapSubFolders){
			CountCompilableFiles(lstCompilableFiles, wcsSrcRoot, val.second, wcsSrcRoot + wcsPrefix + val.first + L'\\', Project);
		}
		for(const auto &val : SrcTree.mapFiles){
			const auto &wcsSrcFilePath = val.first;
			const auto &llTimestamp = val.second;

			std::wstring wcsRelativePath(wcsPrefix + wcsSrcFilePath);

			const std::wstring wcsPath(wcsSrcRoot + wcsRelativePath);
			const std::wstring wcsExtension(GetFileExtension(wcsSrcFilePath));
			if(wcsExtension.empty()){
				Error(L"    警告：已忽略没有扩展名的文件“" + wcsPath + L"”。");
				continue;
			}
			if(Project.mapCompilers.find(wcsExtension) == Project.mapCompilers.end()){
				Error(L"    警告：没有合适的编译器用于构建文件“" + wcsPath + L"”，已忽略。");
				continue;
			}

			lstCompilableFiles.emplace_back(std::move(wcsRelativePath), llTimestamp);
		}
	}

	long long GetFileTimestamp(const std::wstring &wcsPath){
		long long llWriteTime = LLONG_MIN;

		const auto hFile = ::CreateFileW(wcsPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
		if(hFile != INVALID_HANDLE_VALUE){
			union {
				FILETIME ft;
				long long ll;
			} u;

			if(::GetFileTime(hFile, nullptr, nullptr, &u.ft) != FALSE){
				llWriteTime = u.ll;
			}

			::CloseHandle(hFile);
		}

		return llWriteTime;
	}

	void CheckDependencies(
		std::list<std::pair<std::wstring, long long>> &lstCompilableFiles,
		const PROJECT &Project,
		unsigned long ulProcessCount
	){
		JobScheduler Scheduler;
		for(auto &val : lstCompilableFiles){
			const auto &wcsPath = val.first;
			auto &llTimestamp = val.second;

			const std::wstring wcsExtension(wcsPath.substr(wcsPath.rfind(L'.') + 1));
			const auto &wcsDependency = Project.mapCompilers.at(wcsExtension).wcsDependency;
			if(!wcsDependency.empty()){
				Scheduler.PushJob([&wcsPath, &llTimestamp, &wcsDependency, &Project](std::size_t uThreadIndex){
					std::map<std::wstring, std::wstring> mapCommandLineReplacements;
					mapCommandLineReplacements.emplace(L"IN", L'\"' + wcsPath + L'\"');

					ProcessProxy Process;
					Process.Fork(MakeCommandLine(
						wcsDependency,
						mapCommandLineReplacements,
						L"    "
					));
					const auto pResult = Process.Join();

					wchar_t awchPrefix[8];
					std::swprintf(awchPrefix, COUNTOF(awchPrefix), L"%-2u> ", (unsigned int)uThreadIndex);
					const auto wcsStdErr = U8sToWcs(pResult->strStdErr);
					{
						LOCK_THROUGH(GetPrintLock());

						if(!wcsStdErr.empty()){
							std::size_t uCur = 0;
							for(;;){
								std::wstring wcsLine(awchPrefix);
								const std::size_t uLFPos = wcsStdErr.find(L'\n', uCur);
								if(uLFPos == std::wstring::npos){
									wcsLine.append(wcsStdErr.cbegin() + uCur, wcsStdErr.cend());
									Error(wcsLine);
									break;
								}
								wcsLine.append(wcsStdErr.cbegin() + uCur, wcsStdErr.cbegin() + uLFPos);
								Error(wcsLine);
								uCur = uLFPos + 1;
							}
						}
					}

					if(pResult->nExitCode != 0){
						throw Exception(ERROR_PROCESS_ABORTED, L"依赖检查器返回非零状态码。");
					}

					const auto wcsStdOut = U8sToWcs(pResult->strStdOut);
					const std::size_t uColonPos = wcsStdOut.find(L':');
					if(uColonPos == std::wstring::npos){
						throw Exception(ERROR_PROCESS_ABORTED, L"依赖检查器返回无法识别的数据。");
					}
					auto iterRead = wcsStdOut.cbegin() + uColonPos + 1;
					bool bFirst = true;

					const auto UpdateDependency = [&](std::wstring &wcsPath) -> void {
						if(bFirst){
							bFirst = false;
						} else {
							for(wchar_t &ch : wcsPath){
								if(ch == L'/'){
									ch = L'\\';
								}
							}
							const auto llDependencyTimestamp = GetFileTimestamp(wcsPath);
							if(llTimestamp < llDependencyTimestamp){
								llTimestamp = llDependencyTimestamp;
							}
						}
					};

					if(iterRead != wcsStdOut.cend()){
						std::wstring wcsFile;
						wchar_t chNext = *iterRead;
						unsigned int uCountToIgnore = 0;
						for(;;){
							const wchar_t chCur = chNext;
							chNext = *++iterRead;

							if(uCountToIgnore != 0){
								--uCountToIgnore;
								continue;
							}
							switch(chCur){
							case L' ':
								if(!wcsFile.empty()){
									UpdateDependency(wcsFile);
									wcsFile.clear();
								}
								break;
							case L'\\':
								uCountToIgnore = 1;
								switch(chNext){
								case L'\n':
									if(!wcsFile.empty()){
										UpdateDependency(wcsFile);
										wcsFile.clear();
									}
									break;
								default:
									wcsFile.push_back(L'\\');
								case L' ':
								case L'$':
								case L'%':
								case L'?':
								case L'*':
								case L'[':
								case L']':
								case L'\\':
								case L'~':
								case L'#':
									wcsFile.push_back(chNext);
									break;
								}
								break;
							case L'$':
								uCountToIgnore = 1;
								wcsFile.push_back(L'$');
								if(chNext != L'$'){
									wcsFile.push_back(chNext);
								}
								break;
							case L'\n':
								iterRead = wcsStdOut.cend();
								break;
							default:
								wcsFile.push_back(chCur);
								break;
							}

							if(iterRead == wcsStdOut.cend()){
								if(!wcsFile.empty()){
									UpdateDependency(wcsFile);
								}
								break;
							}
						}
					}
				});
			}
		}
		Scheduler.Commit(ulProcessCount, L"    已分析文件：");
	}
}

namespace MCFBuild {
	BUILD_JOBS CountBuildJobs(
		const std::wstring &wcsSrcRoot,
		const FOLDER_TREE &SrcTree,
		const std::wstring &wcsDstRoot,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bRebuildAll,
		bool bVerbose
	){
		BUILD_JOBS BuildJobs;

		std::list<std::pair<std::wstring, long long>> lstCompilableFiles;

		Output(L"  正在统计可编译文件列表...");
		CountCompilableFiles(lstCompilableFiles, wcsSrcRoot, SrcTree, std::wstring(), Project);
		Output(L"    共有 %lu 个可编译文件。", (unsigned long)lstCompilableFiles.size());

		if(bRebuildAll){
			Output(L"  已配置为全部重新构建。");
		}

		if(!Project.PreCompiledHeader.wcsFile.empty()){
			std::wstring wcsGCHSrc(wcsSrcRoot + Project.PreCompiledHeader.wcsFile);
			std::wstring wcsGCHStub(wcsDstRoot + Project.PreCompiledHeader.wcsFile);

			bool bBuildGCH = false;

			const long long llGCHSrcTimestamp = GetFileTimestamp(wcsGCHSrc);
			if(llGCHSrcTimestamp == LLONG_MIN){
				Error(L"  警告：访问预编译头源文件“" + wcsGCHSrc + L"”失败，将不使用预编译头构建。");
			} else if(bRebuildAll){
				bBuildGCH = true;
			} else if(std::max(Project.llProjectFileTimestamp, llGCHSrcTimestamp) >= GetFileTimestamp(wcsGCHStub + L".gch")){
				Output(L"  预编译头文件已经更改，需要全部重新构建。");
				bBuildGCH = true;
				bRebuildAll = true;
			} else {
				Output(L"  预编译头文件已为最新。");
			}
			if(bBuildGCH){
				BuildJobs.wcsGCHSrc = std::move(wcsGCHSrc);
			}

			BuildJobs.wcsGCHStub = std::move(wcsGCHStub);
		}

		long long llMaxObjFileTimestamp = LLONG_MIN;
		if(bRebuildAll){
			Output(L"  正在生成目标文件列表...");
			while(!lstCompilableFiles.empty()){
				const auto &wcsSrcFile = lstCompilableFiles.front().first;

				std::wstring wcsSrcFilePath(wcsSrcRoot + wcsSrcFile);
				std::wstring wcsObjFilePath(wcsDstRoot + wcsSrcFile + L'.' + L'o');

				if(bVerbose){
					Output(L"    将编译：" + wcsSrcFilePath);
				}
				BuildJobs.lstFilesToCompile.emplace_back(std::move(wcsSrcFilePath));
				BuildJobs.lstFilesToLink.emplace_back(std::move(wcsObjFilePath));

				lstCompilableFiles.pop_front();
			}
		} else {
			Output(L"  正在分析源文件依赖关系...");
			CheckDependencies(lstCompilableFiles, Project, ulProcessCount);

			Output(L"  正在比对文件时间戳...");
			std::list<std::wstring> lstObjFilesUnneededToRebuild;
			while(!lstCompilableFiles.empty()){
				auto &wcsSrcFile = lstCompilableFiles.front().first;
				auto llTimestamp = std::max(Project.llProjectFileTimestamp, lstCompilableFiles.front().second);

				std::wstring wcsSrcFilePath(wcsSrcRoot + wcsSrcFile);
				std::wstring wcsObjFilePath(wcsDstRoot + wcsSrcFile + L'.' + L'o');

				const long long llObjFileTimestamp = GetFileTimestamp(wcsObjFilePath);
				if(llTimestamp >= llObjFileTimestamp){
					if(bVerbose){
						Output(L"    将编译：" + wcsSrcFilePath);
					}
					BuildJobs.lstFilesToCompile.emplace_back(std::move(wcsSrcFilePath));
					BuildJobs.lstFilesToLink.emplace_back(std::move(wcsObjFilePath));
				} else {
					if(llMaxObjFileTimestamp < llObjFileTimestamp){
						llMaxObjFileTimestamp = llObjFileTimestamp;
					}

					if(bVerbose){
						Output(L"    已忽略：" + wcsSrcFilePath);
					}
					lstObjFilesUnneededToRebuild.emplace_back(std::move(wcsObjFilePath));
				}

				lstCompilableFiles.pop_front();
			}
			bool bNeedLinking = false;
			if(!BuildJobs.lstFilesToCompile.empty()){
				Output(L"    部分源文件已经更改，需要重新编译并链接。");
				bNeedLinking = true;
			} else if(llMaxObjFileTimestamp >= GetFileTimestamp(Project.wcsOutputPath)){
				Output(L"    部分目标文件已经更改，需要重新链接。");
				bNeedLinking = true;
			}
			if(bNeedLinking){
				Output(L"  正在生成目标文件列表...");
				BuildJobs.lstFilesToLink.splice(BuildJobs.lstFilesToLink.end(), lstObjFilesUnneededToRebuild);
			} else {
				Output(L"  输出文件“" + Project.wcsOutputPath + L"”已为最新。");
				BuildJobs.lstFilesToLink.clear();
			}
		}

		Output(L"  统计结果：");
		if(!BuildJobs.wcsGCHSrc.empty()){
			Output(L"    将构建预编译头。");
		}
		Output(L"    将编译 %lu 个文件。", (unsigned long)BuildJobs.lstFilesToCompile.size());
		Output(L"    将链接 %lu 个文件。", (unsigned long)BuildJobs.lstFilesToLink.size());

		return std::move(BuildJobs);
	}
}
