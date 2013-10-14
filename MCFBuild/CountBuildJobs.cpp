// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
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
		const FOLDER_TREE &SrcTree,
		const std::wstring &wcsPrefix,
		const PROJECT &Project
	){
		for(const auto &val : SrcTree.mapSubFolders){
			CountCompilableFiles(lstCompilableFiles, val.second, wcsPrefix + val.first + L'\\', Project);
		}
		for(const auto &val : SrcTree.mapFiles){
			const auto &wcsSrcFilePath = val.first;
			const auto &llTimestamp = val.second;

			std::wstring wcsPath(wcsPrefix + wcsSrcFilePath);
			const std::wstring wcsExtension(GetFileExtension(wcsSrcFilePath));
			if(wcsExtension.empty()){
				Error(L"    警告：已跳过没有扩展名的文件“" + wcsPath + L"”。");
				continue;
			}
			if(Project.mapCompilers.find(wcsExtension) == Project.mapCompilers.end()){
				Error(L"    警告：没有合适的编译器用于构建文件“" + wcsPath + L"”，已跳过。");
				continue;
			}

			lstCompilableFiles.emplace_back(std::move(wcsPath), llTimestamp);
		}
	}

	long long GetFileTimestamp(const std::wstring &wcsPath){
		long long llWriteTime = LLONG_MIN;

		const HANDLE hFile = ::CreateFileW(wcsPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
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

	std::wstring MakeDependencyCheckerCmdLine(const std::wstring &wcsBase, const std::wstring &wcsInFile, const std::wstring &wcsHint){
		std::wstring wcsCommandLine;
		wcsCommandLine.reserve(wcsBase.size() + wcsInFile.size());
		wcsCommandLine.append(wcsBase);

		const std::size_t uInPos = wcsCommandLine.find(L"$IN");
		if(uInPos == std::wstring::npos){
			Error(L"    警告：" + wcsHint + L"的命令行中没有指定 $$IN 参数。（这是否是故意的？）");
		} else {
			wcsCommandLine.replace(uInPos, 3, wcsInFile);
		}

		return std::move(wcsCommandLine);
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
					const std::wstring wcsHint(L"用于源文件“" + wcsPath + L"”的依赖检查器");

					ProcessProxy Process;
					Process.Fork(MakeDependencyCheckerCmdLine(wcsDependency, L'\"' + wcsPath + L'\"', wcsHint));
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
						throw Exception(ERROR_PROCESS_ABORTED, wcsHint + L"返回非零状态码。");
					}

					const auto wcsStdOut = U8sToWcs(pResult->strStdOut);
					const std::size_t uColonPos = wcsStdOut.find(L':');
					if(uColonPos == std::wstring::npos){
						throw Exception(ERROR_PROCESS_ABORTED, wcsHint + L"返回无法识别的数据。");
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
							} else {
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
		long long llMaxObjFileTimestamp = LLONG_MIN;

		Output(L"  正在统计可编译文件列表...");
		CountCompilableFiles(lstCompilableFiles, SrcTree, wcsSrcRoot, Project);
		Output(L"    共有 %lu 个可编译文件。", (unsigned long)lstCompilableFiles.size());

		if(bRebuildAll){
			Output(L"  已配置为全部重新生成。");

			while(!lstCompilableFiles.empty()){
				auto &wcsSrcFilePath = lstCompilableFiles.front().first;

				std::wstring wcsObjFilePath(wcsDstRoot);
				wcsObjFilePath.append(wcsSrcFilePath.cbegin() + wcsSrcRoot.size(), wcsSrcFilePath.cend());
				wcsObjFilePath.push_back(L'.');
				wcsObjFilePath.push_back(L'o');

				if(bVerbose){
					Output(L"    将编译：" + wcsSrcFilePath);
				}
				BuildJobs.lstFilesToCompile.emplace_back(std::move(wcsSrcFilePath));
				BuildJobs.lstFilesToLink.emplace_back(std::move(wcsObjFilePath));

				lstCompilableFiles.pop_front();
			}
		} else {
			Output(L"  正在分析依赖关系...");
			CheckDependencies(lstCompilableFiles, Project, ulProcessCount);

			Output(L"  正在比对文件时间戳...");
			std::list<std::wstring> lstObjFilesUnneededToRebuild;
			while(!lstCompilableFiles.empty()){
				auto &wcsSrcFilePath = lstCompilableFiles.front().first;
				auto &llTimestamp = lstCompilableFiles.front().second;

				std::wstring wcsObjFilePath(wcsDstRoot);
				wcsObjFilePath.append(wcsSrcFilePath.cbegin() + wcsSrcRoot.size(), wcsSrcFilePath.cend());
				wcsObjFilePath.push_back(L'.');
				wcsObjFilePath.push_back(L'o');

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
						Output(L"    已跳过：" + wcsSrcFilePath);
					}
					lstObjFilesUnneededToRebuild.emplace_back(std::move(wcsObjFilePath));
				}

				lstCompilableFiles.pop_front();
			}
			const long long llOutputTimestamp = GetFileTimestamp(Project.wcsOutputPath);
			bool bNeedLinking = false;
			if(!BuildJobs.lstFilesToCompile.empty() || (llMaxObjFileTimestamp >= llOutputTimestamp)){
				Output(L"    自上次构建以来，部分源文件已更改，需重新编译并链接。");
				bNeedLinking = true;
			} else if(Project.llProjectFileTimestamp >= llOutputTimestamp){
				Output(L"    自上次构建以来，项目文件已更改，需重新链接。");
				bNeedLinking = true;
			}
			if(bNeedLinking){
				BuildJobs.lstFilesToLink.splice(BuildJobs.lstFilesToLink.end(), lstObjFilesUnneededToRebuild);
			} else {
				Output(L"    文件“" + Project.wcsOutputPath + L"”已为最新。");
				BuildJobs.lstFilesToLink.clear();
			}
		}

		Output(L"  总计：");
		Output(L"    将编译 %lu 个文件。", (unsigned long)BuildJobs.lstFilesToCompile.size());
		Output(L"    将链接 %lu 个文件。", (unsigned long)BuildJobs.lstFilesToLink.size());

		return std::move(BuildJobs);
	}
}
