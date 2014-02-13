// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <list>
#include "UniqueHandle.hpp"
#include "JobScheduler.hpp"
using namespace MCFBuild;

namespace {
	void CopyStdOutErr(const PROCESS_EXIT_INFO &vResult, const wvstring &wcsPrefix){
		const auto wcsStdOut = U8sToWcs(vResult.strStdOut);
		if(!wcsStdOut.empty()){
			LOCK_THROUGH(GetPrintLock());

			std::size_t uCur = 0;
			for(;;){
				wvstring wcsLine(wcsPrefix);
				const std::size_t uLFPos = wcsStdOut.find(L'\n', uCur);
				if(uLFPos == wvstring::npos){
					wcsLine.append(wcsStdOut.cbegin() + uCur, wcsStdOut.cend());
					Output(wcsLine);
					break;
				}
				wcsLine.append(wcsStdOut.cbegin() + uCur, wcsStdOut.cbegin() + uLFPos);
				Output(wcsLine);
				uCur = uLFPos + 1;
			}
		}
		const auto wcsStdErr = U8sToWcs(vResult.strStdErr);
		if(!wcsStdErr.empty()){
			LOCK_THROUGH(GetPrintLock());

			std::size_t uCur = 0;
			for(;;){
				wvstring wcsLine(wcsPrefix);
				const std::size_t uLFPos = wcsStdErr.find(L'\n', uCur);
				if(uLFPos == wvstring::npos){
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

	void PutFileContents(const wvstring &wcsFile, const vstring &strContents){
		struct FileCloser {
			constexpr HANDLE operator()(){
				return INVALID_HANDLE_VALUE;
			}
			void operator()(HANDLE hObj){
				::CloseHandle(hObj);
			}
		};

		const auto hFile = MCF::UniqueHandle<HANDLE, FileCloser>(::CreateFileW(wcsFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, NULL));
		if(!hFile.IsGood()){
			const DWORD dwError = ::GetLastError();
			throw Exception{dwError, L"创建文件“" + wcsFile + L"”失败。"};
		}

		std::size_t uBytesTotal = 0;
		DWORD dwBytesWritten;
		do {
			if(::WriteFile(hFile, strContents.data() + uBytesTotal, (DWORD)(strContents.size() - uBytesTotal), &dwBytesWritten, nullptr) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"写入文件“" + wcsFile + L"”时出错。"};
			}
			uBytesTotal += dwBytesWritten;
		} while(dwBytesWritten != 0);
	}

	void BuildPreCompiledHeaders(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bVerbose
	){
		if(Project.mapPreCompiledHeaders.empty()){
			Output(L"    不使用预编译头，已跳过。");
			return;
		}

		JobScheduler Scheduler;

		for(const auto &GCHItem : BuildJobs.mapGCHs){
			if(GCHItem.second.wcsSourceFile.empty()){
				Output(L"    预编译头文件“" + GCHItem.second.wcsStubFile + L".gch”已为最新，已跳过。");
				continue;
			}

			Scheduler.PushJob([=](std::size_t uThreadIndex){
				std::map<wvstring, wvstring> mapCommandLineReplacements;
				mapCommandLineReplacements.emplace(L"IN", L'\"' + GCHItem.second.wcsSourceFile + L'\"');
				mapCommandLineReplacements.emplace(L"OUT", L'\"' + GCHItem.second.wcsStubFile + L".gch\"");

				const auto wcsCommandLine = MakeCommandLine(
					GCHItem.second.wcsCommandLine,
					mapCommandLineReplacements,
					L"      "
				);

				if(bVerbose){
					Output(L"    命令行：" + wcsCommandLine);
				} else {
					Output(L"    源文件：" + GCHItem.second.wcsSourceFile);
				}
				Output(L"    桩文件：" + GCHItem.second.wcsStubFile);

				std::size_t uBackSlashPos = GCHItem.second.wcsStubFile.rfind(L'\\');
				if(uBackSlashPos != wvstring::npos){
					TouchFolder(GCHItem.second.wcsStubFile.substr(0, uBackSlashPos));
				}

				const auto ExitInfo = Execute(wcsCommandLine);

				wchar_t awchPrefix[8];
				std::swprintf(awchPrefix, COUNTOF(awchPrefix), L"  %-2lu> ", (unsigned long)uThreadIndex);
				CopyStdOutErr(ExitInfo, awchPrefix);

				if(ExitInfo.dwExitCode != 0){
					throw Exception{ERROR_PROCESS_ABORTED, L"预编译器返回非零状态码。"};
				}

				vstring u8sStubContents("#warning Failed to load precompiled header file.\n");
				u8sStubContents.append("#include \"");
				const vstring u8sGCHSrc(WcsToU8s(GCHItem.second.wcsSourceFile));
				for(auto iter = u8sGCHSrc.cbegin(); iter != u8sGCHSrc.cend(); ++iter){
					const char ch = *iter;
					if(ch == '\\'){
						u8sStubContents.push_back('\\');
					}
					u8sStubContents.push_back(ch);
				}
				u8sStubContents.push_back('\"');
				u8sStubContents.push_back('\n');
				PutFileContents(GCHItem.second.wcsStubFile, u8sStubContents);
			});
		}

		Scheduler.Commit(ulProcessCount, L"    已构建预编译头：");
	}

	void Compile(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bVerbose
	){
		if(BuildJobs.lstFilesToCompile.empty()){
			Output(L"    没有需要编译的文件，已跳过。");
			return;
		}

		JobScheduler Scheduler;

		auto iterSrcFile = BuildJobs.lstFilesToCompile.cbegin();
		auto iterObjFile = BuildJobs.lstFilesToLink.cbegin();
		do {
			Scheduler.PushJob([=](std::size_t uThreadIndex){
				std::map<wvstring, wvstring> mapCommandLineReplacements;
				for(const auto &GCHItem : BuildJobs.mapGCHs){
					mapCommandLineReplacements.emplace(
						L"GCH." + GCHItem.first,
						L"-include \"" + GCHItem.second.wcsStubFile + L'\"'
					);
				}
				mapCommandLineReplacements.emplace(L"IN", L'\"' + *iterSrcFile + L'\"');
				mapCommandLineReplacements.emplace(L"OUT", L'\"' + *iterObjFile + L'\"');

				const auto wcsCommandLine = MakeCommandLine(
					Project.mapCompilers.at(GetFileExtension(*iterSrcFile)).wcsCommandLine,
					mapCommandLineReplacements,
					L"    "
				);
				if(bVerbose){
					Output(L"    命令行：" + wcsCommandLine);
				} else {
					Output(L"    文件：" + *iterSrcFile);
				}

				const std::size_t uBackSlashPos = iterObjFile->rfind(L'\\');
				if(uBackSlashPos != wvstring::npos){
					TouchFolder(iterObjFile->substr(0, uBackSlashPos));
				}

				const auto ExitInfo = Execute(wcsCommandLine);

				wchar_t awchPrefix[8];
				std::swprintf(awchPrefix, COUNTOF(awchPrefix), L"  %-2lu> ", (unsigned long)uThreadIndex);
				CopyStdOutErr(ExitInfo, awchPrefix);

				if(ExitInfo.dwExitCode != 0){
					throw Exception{ERROR_PROCESS_ABORTED, L"编译器返回非零状态码。"};
				}
			});

			++iterSrcFile;
			++iterObjFile;
		} while(iterSrcFile != BuildJobs.lstFilesToCompile.cend());

		Scheduler.Commit(ulProcessCount, L"    已编译文件：");
	}

	void Link(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		bool bVerbose
	){
		if(BuildJobs.lstFilesToLink.empty()){
			Output(L"    没有需要链接的文件，已跳过。");
			return;
		}

		if(::DeleteFileW(Project.wcsOutputPath.c_str()) == FALSE){
			const DWORD dwError = ::GetLastError();
			if(dwError != ERROR_FILE_NOT_FOUND){
				throw Exception{dwError, L"删除文件“" + Project.wcsOutputPath + L"”失败。"};
			}
		}

		unsigned long ulPartIndex = 1;

		const auto ExecuteLinker = [](const wvstring &wcsCommandLine) -> void {
			const auto ExitInfo = Execute(wcsCommandLine);

			CopyStdOutErr(ExitInfo, L"  1>  ");
			if(ExitInfo.dwExitCode != 0){
				throw Exception{ERROR_PROCESS_ABORTED, L"链接器返回非零状态码。"};
			}
		};

		std::list<wvstring> lstFilesToLink(BuildJobs.lstFilesToLink);
		for(;;){
			std::list<wvstring> lstFilesLinked;

			wvstring wcsInputFiles;
			do {
				wcsInputFiles.push_back(L'\"');
				wcsInputFiles.append(lstFilesToLink.front());
				wcsInputFiles.push_back(L'\"');
				wcsInputFiles.push_back(L' ');

				lstFilesLinked.splice(lstFilesLinked.end(), lstFilesToLink, lstFilesToLink.begin());
			} while(!lstFilesToLink.empty() && (wcsInputFiles.size() < 28 * 0x400));	// Windows 命令行长度上限为 32K 个字符。
			wcsInputFiles.pop_back();

			std::map<wvstring, wvstring> mapCommandLineReplacements;
			mapCommandLineReplacements.emplace(L"IN", wcsInputFiles);

			if(!lstFilesToLink.empty()){
				Output(L"    正在执行部分链接...");

				wchar_t achPartialPostfix[32];
				std::swprintf(achPartialPostfix, COUNTOF(achPartialPostfix), L".part%lu.o", ulPartIndex++);
				wvstring wcsPartialObjPath(Project.wcsOutputPath + achPartialPostfix);

				mapCommandLineReplacements.emplace(L"OUT", L'\"' + wcsPartialObjPath + L'\"');

				const wvstring wcsCmdLine(MakeCommandLine(
					Project.Linkers.wcsPartial,
					mapCommandLineReplacements,
					L"      "
				));
				if(bVerbose){
					Output(L"    命令行：" + wcsCmdLine);
				}

				std::size_t uBackSlashPos = wcsPartialObjPath.rfind(L'\\');
				if(uBackSlashPos != wvstring::npos){
					TouchFolder(wcsPartialObjPath.substr(0, uBackSlashPos));
				}
				ExecuteLinker(wcsCmdLine);
				Output(L"      已生成中间文件“" + wcsPartialObjPath + L"”。");

				lstFilesToLink.emplace_back(std::move(wcsPartialObjPath));
			} else {
				Output(L"    正在执行完全链接...");

				mapCommandLineReplacements.emplace(L"OUT", L'\"' + Project.wcsOutputPath + L'\"');

				const wvstring wcsCmdLine(MakeCommandLine(
					Project.Linkers.wcsFull,
					mapCommandLineReplacements,
					L"      "
				));
				if(bVerbose){
					Output(L"    命令行：" + wcsCmdLine);
				}

				std::size_t uBackSlashPos = Project.wcsOutputPath.rfind(L'\\');
				if(uBackSlashPos != wvstring::npos){
					TouchFolder(Project.wcsOutputPath.substr(0, uBackSlashPos));
				}
				ExecuteLinker(wcsCmdLine);

				break;
			}
		}
	}
}

namespace MCFBuild {
	void CompileAndLink(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		unsigned long ulProcessCount,
		bool bVerbose
	){
		Output(L"  正在构建预编译头...");
		BuildPreCompiledHeaders(BuildJobs, Project, ulProcessCount, bVerbose);

		Output(L"  正在编译...");
		Compile(BuildJobs, Project, ulProcessCount, bVerbose);

		Output(L"  正在链接...");
		Link(BuildJobs, Project, bVerbose);

		Output(L"  文件“" + Project.wcsOutputPath + L"”构建成功。");
	}
}
