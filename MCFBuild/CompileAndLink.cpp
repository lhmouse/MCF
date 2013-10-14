// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <list>
#include <vector>
#include "JobScheduler.hpp"
#include "ProcessProxy.hpp"
using namespace MCFBuild;

namespace {
	void TouchFolder(const std::wstring &wcsPath){
		const DWORD dwAttributes = GetFileAttributesW(wcsPath.c_str());
		if(dwAttributes == INVALID_FILE_ATTRIBUTES){
			const DWORD dwError = ::GetLastError();
			switch(dwError){
			case ERROR_PATH_NOT_FOUND:
				{
					const std::size_t uBackSlashPos = wcsPath.rfind(L'\\');
					if(uBackSlashPos == std::wstring::npos){
						throw Exception(dwError, L"创建目录“" + wcsPath + L"”失败。");
					}
					TouchFolder(wcsPath.substr(0, uBackSlashPos));
				}
				// 没有 break。
			case ERROR_FILE_NOT_FOUND:
				if(::CreateDirectoryW(wcsPath.c_str(), nullptr) == FALSE){
					const DWORD dwError = ::GetLastError();
					if(dwError != ERROR_ALREADY_EXISTS){
						throw Exception(dwError, L"创建目录“" + wcsPath + L"”失败。");
					}
				}
				break;
			default:
				throw Exception(dwError, L"获取目录“" + wcsPath + L"”的属性失败。");
			}
		} else {
			if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
				throw Exception(ERROR_PATH_NOT_FOUND, L"路径“" + wcsPath + L"”不是一个目录。");
			}
		}
	}

	std::wstring MakeCommandLine(const std::wstring &wcsBase, const std::wstring &wcsInFile, const std::wstring &wcsOutFile, const std::wstring &wcsHint){
		std::wstring wcsCommandLine;
		wcsCommandLine.reserve(wcsBase.size() + wcsInFile.size() + wcsOutFile.size());
		wcsCommandLine.append(wcsBase);

		const std::size_t uInPos = wcsCommandLine.find(L"$IN");
		if(uInPos == std::wstring::npos){
			Error(L"  警告：" + wcsHint + L"的命令行中没有指定 $$IN 参数。（这是否是故意的？）");
		} else {
			wcsCommandLine.replace(uInPos, 3, wcsInFile);
		}
		const std::size_t uOutPos = wcsCommandLine.find(L"$OUT");
		if(uOutPos == std::wstring::npos){
			Error(L"  警告：" + wcsHint + L"的命令行中没有指定 $$OUT 参数。（这是否是故意的？）");
		} else {
			wcsCommandLine.replace(uOutPos, 4, wcsOutFile);
		}

		return std::move(wcsCommandLine);
	}
}

namespace MCFBuild {
	void Compile(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bVerbose
	){
		if(BuildJobs.lstFilesToCompile.empty()){
			Output(L"  没有需要编译的文件。");
			return;
		}

		JobScheduler Scheduler;

		auto iterSrcFile = BuildJobs.lstFilesToCompile.cbegin();
		auto iterObjFile = BuildJobs.lstFilesToLink.cbegin();
		do {
			Scheduler.PushJob([iterSrcFile, iterObjFile, &Project, bVerbose](std::size_t uThreadIndex){
				const std::wstring wcsHint(L"用于源文件“" + *iterSrcFile + L"”的编译器");

				const auto wcsCommandLine = MakeCommandLine(
					Project.mapCompilers.at(GetFileExtension(*iterSrcFile)).wcsCommandLine,
					L'\"' + *iterSrcFile + L'\"',
					L'\"' + *iterObjFile + L'\"',
					wcsHint
				);
				if(bVerbose){
					Output(L"  " + wcsCommandLine);
				} else {
					Output(L"  " + *iterSrcFile);
				}

				const std::size_t uBackSlashPos = iterObjFile->rfind(L'\\');
				if(uBackSlashPos != std::wstring::npos){
					TouchFolder(iterObjFile->substr(0, uBackSlashPos));
				}

				ProcessProxy Process;
				Process.Fork(wcsCommandLine);
				const auto pResult = Process.Join();

				wchar_t awchPrefix[8];
				std::swprintf(awchPrefix, COUNTOF(awchPrefix), L"%-2u> ", (unsigned int)uThreadIndex);
				{
					LOCK_THROUGH(GetPrintLock());

					const auto wcsStdOut = U8sToWcs(pResult->strStdOut);
					if(!wcsStdOut.empty()){
						std::size_t uCur = 0;
						for(;;){
							std::wstring wcsLine(awchPrefix);
							const std::size_t uLFPos = wcsStdOut.find(L'\n', uCur);
							if(uLFPos == std::wstring::npos){
								wcsLine.append(wcsStdOut.cbegin() + uCur, wcsStdOut.cend());
								Output(wcsLine);
								break;
							}
							wcsLine.append(wcsStdOut.cbegin() + uCur, wcsStdOut.cbegin() + uLFPos);
							Output(wcsLine);
							uCur = uLFPos + 1;
						}
					}
					const auto wcsStdErr = U8sToWcs(pResult->strStdErr);
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
			});

			++iterSrcFile;
			++iterObjFile;
		} while(iterSrcFile != BuildJobs.lstFilesToCompile.cend());

		Scheduler.Commit(ulProcessCount, L"  已编译文件：");
	}

	void Link(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		bool bVerbose
	){
		if(BuildJobs.lstFilesToLink.empty()){
			Output(L"  没有需要链接的文件。");
			return;
		}

		::DeleteFileW(Project.wcsOutputPath.c_str());

		unsigned long ulPartIndex = 1;

		const auto ExecuteLinker = [](const std::wstring &wcsCommandLine, const std::wstring &wcsHint) -> void {
			ProcessProxy Process;
			Process.Fork(wcsCommandLine);
			const auto pResult = Process.Join();

			const std::wstring wcsPrefix(L"1>  ");

			const auto wcsStdOut = U8sToWcs(pResult->strStdOut);
			if(!wcsStdOut.empty()){
				std::size_t uCur = 0;
				for(;;){
					std::wstring wcsLine(wcsPrefix);
					const std::size_t uLFPos = wcsStdOut.find(L'\n', uCur);
					if(uLFPos == std::wstring::npos){
						wcsLine.append(wcsStdOut.cbegin() + uCur, wcsStdOut.cend());
						Output(wcsLine);
						break;
					}
					wcsLine.append(wcsStdOut.cbegin() + uCur, wcsStdOut.cbegin() + uLFPos);
					Output(wcsLine);
					uCur = uLFPos + 1;
				}
			}
			const auto wcsStdErr = U8sToWcs(pResult->strStdErr);
			if(!wcsStdErr.empty()){
				std::size_t uCur = 0;
				for(;;){
					std::wstring wcsLine(wcsPrefix);
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

			if(pResult->nExitCode != 0){
				throw Exception(ERROR_PROCESS_ABORTED, wcsHint + L"返回非零状态码。");
			}
		};

		std::list<std::wstring> lstFilesToLink(BuildJobs.lstFilesToLink);
		for(;;){
			std::list<std::wstring> lstFilesLinked;

			std::wstring wcsInputFiles;
			do {
				wcsInputFiles.push_back(L'\"');
				wcsInputFiles.append(lstFilesToLink.front());
				wcsInputFiles.push_back(L'\"');
				wcsInputFiles.push_back(L' ');

				lstFilesLinked.splice(lstFilesLinked.end(), lstFilesToLink, lstFilesToLink.begin());
			} while(!lstFilesToLink.empty() && (wcsInputFiles.size() < 8 * 0x400));
			wcsInputFiles.pop_back();

			if(!lstFilesToLink.empty()){
				Output(L"  正在执行部分链接...");

				const std::wstring wcsHint(L"部分链接器");

				wchar_t achPartialPostfix[32];
				std::swprintf(achPartialPostfix, COUNTOF(achPartialPostfix), L".part%lu.o", ulPartIndex++);
				std::wstring wcsPartialObjPath(Project.wcsOutputPath + achPartialPostfix);

				const std::wstring wcsCmdLine(MakeCommandLine(
					Project.Linkers.wcsPartial,
					wcsInputFiles,
					L'\"' + wcsPartialObjPath + L'\"',
					wcsHint
				));
				if(bVerbose){
					Output(L"    " + wcsCmdLine);
				}

				std::size_t uBackSlashPos = wcsPartialObjPath.rfind(L'\\');
				if(uBackSlashPos != std::wstring::npos){
					TouchFolder(wcsPartialObjPath.substr(0, uBackSlashPos));
				}
				ExecuteLinker(wcsCmdLine, wcsHint);
				Output(L"    已生成中间文件“" + wcsPartialObjPath + L"”。");

				lstFilesToLink.emplace_back(std::move(wcsPartialObjPath));
			} else {
				Output(L"  正在执行完全链接...");

				const std::wstring wcsHint(L"完全链接器");

				const std::wstring wcsCmdLine(MakeCommandLine(
					Project.Linkers.wcsFull,
					wcsInputFiles,
					L'\"' + Project.wcsOutputPath + L'\"',
					wcsHint
				));
				if(bVerbose){
					Output(L"    " + wcsCmdLine);
				}

				std::size_t uBackSlashPos = Project.wcsOutputPath.rfind(L'\\');
				if(uBackSlashPos != std::wstring::npos){
					TouchFolder(Project.wcsOutputPath.substr(0, uBackSlashPos));
				}
				ExecuteLinker(wcsCmdLine, wcsHint);
				Output(L"    输出文件“" + Project.wcsOutputPath + L"”构建成功。");

				break;
			}
		}
	}
}
