// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <climits>
#include <map>
#include <algorithm>
#include <iterator>
#include "JobScheduler.hpp"
#include "ProcessProxy.hpp"
#include "DependencyDatabase.hpp"
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
			CountCompilableFiles(lstCompilableFiles, wcsSrcRoot, val.second, wcsPrefix + val.first + L'\\', Project);
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

	bool CheckDependencies(
		std::list<std::pair<std::wstring, long long>> &lstCompilableFiles,
		const std::wstring &wcsSrcRoot,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		const std::wstring &wcsDatabasePath,
		bool bVerbose
	){
		DependencyDatabase OldDatabase;
		OldDatabase.LoadFromFile(wcsDatabasePath);

		DependencyDatabase NewDatabase;
		CriticalSection csNewDatabaseLock;

		JobScheduler Scheduler;
		for(auto &val : lstCompilableFiles){
			const auto &wcsPath = val.first;
			auto &llTimestamp = val.second;

			const std::wstring wcsExtension(GetFileExtension(wcsPath));
			const auto &wcsDependency = Project.mapCompilers.at(wcsExtension).wcsDependency;
			if(!wcsDependency.empty()){
				Scheduler.PushJob([&](std::size_t uThreadIndex){
					const auto llSourceTimestamp = GetFileTimestamp(wcsPath);

					DependencyDatabase::Dependencies Dependencies{ LLONG_MIN, { } };

					const auto iterDBItem = OldDatabase.GetRawMap().find(wcsPath);
					bool bCached = false;
					if(iterDBItem != OldDatabase.GetRawMap().end()){
						Dependencies = std::move(iterDBItem->second);
						OldDatabase.GetRawMap().erase(iterDBItem);
						bCached = true;
					}
					if(bCached && (Dependencies.m_llTimestamp >= llSourceTimestamp)){
						if(bVerbose){
							Output(L"      已缓存：" + wcsPath);
						}
					} else {
						if(bVerbose){
							Output(L"      重分析：" + wcsPath);
						}

						std::map<std::wstring, std::wstring> mapCommandLineReplacements;
						mapCommandLineReplacements.emplace(L"IN", L'\"' + wcsSrcRoot + wcsPath + L'\"');

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

						if(iterRead != wcsStdOut.cend()){
							Dependencies.m_llTimestamp = llSourceTimestamp;

							std::wstring wcsDependencyFile;
							const auto Submit = [&]() -> void {
								if(wcsDependencyFile.empty()){
									return;
								}
								if(bFirst){
									wcsDependencyFile.clear();
									bFirst = false;
								} else {
									for(auto &ch : wcsDependencyFile){
										if(ch == L'/'){
											ch = L'\\';
										}
									}
									std::wstring wcsTemp;
									wcsTemp.swap(wcsDependencyFile);

									Dependencies.m_setDependencyFiles.insert(std::move(wcsTemp));
								}
							};

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
									Submit();
									break;
								case L'\\':
									uCountToIgnore = 1;
									switch(chNext){
									case L'\n':
										Submit();
										break;
									default:
										wcsDependencyFile.push_back(L'\\');
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
										wcsDependencyFile.push_back(chNext);
										break;
									}
									break;
								case L'$':
									uCountToIgnore = 1;
									wcsDependencyFile.push_back(L'$');
									if(chNext != L'$'){
										wcsDependencyFile.push_back(chNext);
									}
									break;
								case L'\n':
									iterRead = wcsStdOut.cend();
									break;
								default:
									wcsDependencyFile.push_back(chCur);
									break;
								}

								if(iterRead == wcsStdOut.cend()){
									Submit();
									break;
								}
							}
						}
					}
					for(const auto &wcsDependencyFile : Dependencies.m_setDependencyFiles){
						llTimestamp = std::max(llTimestamp, GetFileTimestamp(wcsDependencyFile));
					}

					{
						LOCK_THROUGH(csNewDatabaseLock);

						if(!bCached){
							llTimestamp = LLONG_MAX;
						}
						NewDatabase.GetRawMap().emplace(wcsPath, std::move(Dependencies));
					}
				});
			}
		}
		Scheduler.Commit(ulProcessCount, L"      已分析文件：");

		NewDatabase.SaveToFile(wcsDatabasePath);

		if(bVerbose){
			for(const auto &DBItem : OldDatabase.GetRawMap()){
				Output(L"      已删除：" + DBItem.first);
			}
		}

		return !OldDatabase.GetRawMap().empty();
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

		if(!Project.PreCompiledHeader.wcsSourceFile.empty()){
			Output(L"  正在检测预编译头...");

			std::wstring wcsGCHSrc(wcsSrcRoot + Project.PreCompiledHeader.wcsSourceFile);
			std::wstring wcsGCHStub(wcsDstRoot + Project.PreCompiledHeader.wcsSourceFile);

			const long long llGCHSrcTimestamp = GetFileTimestamp(wcsGCHSrc);
			if(llGCHSrcTimestamp == LLONG_MIN){
				Error(L"    警告：访问预编译头源文件“" + wcsGCHSrc + L"”失败，将不使用预编译头构建。");
			} else {
				if(!bRebuildAll){
					const long long llGCHTimestamp = GetFileTimestamp(wcsGCHStub + L".gch");
					if(Project.llProjectFileTimestamp >= llGCHTimestamp){
						Output(L"    项目文件已经更改，需要全部重新构建。");
						bRebuildAll = true;
					} else if(llGCHSrcTimestamp >= llGCHTimestamp){
						Output(L"    预编译头源文件已经更改，需要全部重新构建。");
						bRebuildAll = true;
					}
				}
				if(bRebuildAll){
					if(bVerbose){
						Output(L"    预编译头源文件：" + wcsGCHSrc);
					} else {
						Output(L"    将构建预编译头。");
					}
					BuildJobs.wcsGCHSrc = std::move(wcsGCHSrc);
				} else {
					Output(L"    预编译头已为最新。");
				}
				BuildJobs.wcsGCHStub = std::move(wcsGCHStub);
			}
		}

		Output(L"  正在统计需要重新编译的文件列表...");

		const std::wstring wcsDatabasePath(wcsDstRoot + L"Dependencies.db");
		std::size_t uBackSlashPos = wcsDatabasePath.rfind(L'\\');
		if(uBackSlashPos != std::wstring::npos){
			TouchFolder(wcsDatabasePath.substr(0, uBackSlashPos));
		}
		if(bRebuildAll){
			Output(L"    正在删除旧的依赖关系数据库...");
			if(::DeleteFileW(wcsDatabasePath.c_str()) == FALSE){
				const DWORD dwError = ::GetLastError();
				if(dwError != ERROR_FILE_NOT_FOUND){
					throw Exception(dwError, L"删除文件“" + wcsDatabasePath + L"”失败。");
				}
			}
		}

		Output(L"    正在分析源文件依赖关系...");
		const bool bSourceFilesDeleted = CheckDependencies(lstCompilableFiles, wcsSrcRoot, Project, ulProcessCount, wcsDatabasePath, bVerbose);

		long long llMaxObjFileTimestamp;
		if(bRebuildAll){
			Output(L"    已配置为全部重新构建。");
			llMaxObjFileTimestamp = LLONG_MAX;
		} else {
			Output(L"    正在比对文件时间戳...");
			llMaxObjFileTimestamp = LLONG_MIN;
		}
		std::list<std::wstring> lstObjFilesUnneededToRebuild;
		while(!lstCompilableFiles.empty()){
			auto &wcsSrcFile = lstCompilableFiles.front().first;
			auto &llTimestamp = lstCompilableFiles.front().second;

			std::wstring wcsSrcFilePath(wcsSrcRoot + wcsSrcFile);
			std::wstring wcsObjFilePath(wcsDstRoot + wcsSrcFile + L'.' + L'o');

			long long llObjFileTimestamp;
			bool bNeedCompiling = false;
			if(bRebuildAll){
				bNeedCompiling = true;
			} else {
				llObjFileTimestamp = GetFileTimestamp(wcsObjFilePath);
				if(std::max(Project.llProjectFileTimestamp, llTimestamp) >= llObjFileTimestamp){
					bNeedCompiling = true;
				}
			}
			if(bNeedCompiling){
				if(bVerbose){
					Output(L"      将编译：" + wcsSrcFilePath);
				}
				BuildJobs.lstFilesToCompile.emplace_back(std::move(wcsSrcFilePath));
				BuildJobs.lstFilesToLink.emplace_back(std::move(wcsObjFilePath));
			} else {
				if(llMaxObjFileTimestamp < llObjFileTimestamp){
					llMaxObjFileTimestamp = llObjFileTimestamp;
				}
				if(bVerbose){
					Output(L"      已最新：" + wcsSrcFilePath);
				}
				lstObjFilesUnneededToRebuild.emplace_back(std::move(wcsObjFilePath));
			}

			lstCompilableFiles.pop_front();
		}

		bool bNeedLinking = false;
		if(!BuildJobs.lstFilesToCompile.empty()){
			Output(L"    部分源文件已更改，需重新链接。");
			bNeedLinking = true;
		} else if(llMaxObjFileTimestamp >= GetFileTimestamp(Project.wcsOutputPath)){
			Output(L"    部分目标文件已更改，需重新链接。");
			bNeedLinking = true;
		} else if(bSourceFilesDeleted){
			Output(L"    部分源文件已删除，需重新链接。");
			bNeedLinking = true;
		}
		if(bNeedLinking){
			BuildJobs.lstFilesToLink.splice(BuildJobs.lstFilesToLink.end(), lstObjFilesUnneededToRebuild);
		} else {
			BuildJobs.lstFilesToLink.clear();
		}

		Output(L"    将编译 %lu 个文件。", (unsigned long)BuildJobs.lstFilesToCompile.size());
		Output(L"    将链接 %lu 个文件。", (unsigned long)BuildJobs.lstFilesToLink.size());

		return std::move(BuildJobs);
	}
}
