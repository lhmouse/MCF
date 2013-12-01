// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <list>
#include "JobScheduler.hpp"
#include "ProcessProxy.hpp"
using namespace MCFBuild;

namespace {
	void CopyStdOutErr(const ProcessProxy::ExitInfo Result, const std::wstring &wcsPrefix){
		const auto wcsStdOut = U8sToWcs(Result.strStdOut);
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
		const auto wcsStdErr = U8sToWcs(Result.strStdErr);
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
	}

	void PutFileContents(const std::wstring &wcsFile, const std::string &strContents){
		struct FileCloser {
			constexpr HANDLE Null(){
				return INVALID_HANDLE_VALUE;
			}
			void Close(HANDLE hObj){
				::CloseHandle(hObj);
			}
		};

		const auto hFile = MCF::UniqueHandle<HANDLE, FileCloser>(::CreateFileW(wcsFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, NULL));
		if(!hFile.IsGood()){
			const DWORD dwError = ::GetLastError();
			throw Exception(dwError, L"创建文件“" + wcsFile + L"”失败。");
		}

		std::size_t uBytesTotal = 0;
		DWORD dwBytesWritten;
		do {
			if(::WriteFile(hFile, strContents.data() + uBytesTotal, (DWORD)(strContents.size() - uBytesTotal), &dwBytesWritten, nullptr) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception(dwError, L"写入文件“" + wcsFile + L"”时出错。");
			}
			uBytesTotal += dwBytesWritten;
		} while(dwBytesWritten != 0);
	}

	void BuildPreCompiledHeader(
		const BUILD_JOBS &BuildJobs,
		const PROJECT &Project,
		bool bVerbose
	){
		if(Project.PreCompiledHeader.wcsSourceFile.empty()){
			Output(L"    不使用预编译头，已跳过。");
			return;
		}
		if(BuildJobs.wcsGCHSrc.empty()){
			Output(L"    预编译头已为最新，已跳过。");
			return;
		}

		std::map<std::wstring, std::wstring> mapCommandLineReplacements;
		mapCommandLineReplacements.emplace(L"IN", L'\"' + BuildJobs.wcsGCHSrc + L'\"');
		mapCommandLineReplacements.emplace(L"OUT", L'\"' + BuildJobs.wcsGCHStub + L".gch\"");

		const auto wcsCommandLine = MakeCommandLine(
			Project.PreCompiledHeader.wcsCommandLine,
			mapCommandLineReplacements,
			L"      "
		);
		if(bVerbose){
			Output(L"    命令行：" + wcsCommandLine);
		} else {
			Output(L"    文件：" + BuildJobs.wcsGCHStub + L".gch");
		}

		std::size_t uBackSlashPos = BuildJobs.wcsGCHStub.rfind(L'\\');
		if(uBackSlashPos != std::wstring::npos){
			TouchFolder(BuildJobs.wcsGCHStub.substr(0, uBackSlashPos));
		}

		ProcessProxy Process;
		Process.Fork(wcsCommandLine);
		const auto pResult = Process.Join();
		CopyStdOutErr(*pResult, L"  1>  ");
		if(pResult->nExitCode != 0){
			throw Exception(ERROR_PROCESS_ABORTED, L"预编译器返回非零状态码。");
		}

		Output(L"    桩文件：" + BuildJobs.wcsGCHStub);

		std::string u8sStubContents("#warning Failed to load precompiled header file.\n");
		u8sStubContents.append("#include \"");
		const std::string u8sGCHSrc(WcsToU8s(BuildJobs.wcsGCHSrc));
		for(auto iter = u8sGCHSrc.cbegin(); iter != u8sGCHSrc.cend(); ++iter){
			const char ch = *iter;
			if(ch == '\\'){
				u8sStubContents.push_back('\\');
			}
			u8sStubContents.push_back(ch);
		}
		u8sStubContents.push_back('\"');
		u8sStubContents.push_back('\n');
		PutFileContents(BuildJobs.wcsGCHStub, u8sStubContents);
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
			const std::wstring wcsSrcFile(*iterSrcFile);
			const std::wstring wcsObjFile(*iterObjFile);

			Scheduler.PushJob([wcsSrcFile, wcsObjFile, &BuildJobs, &Project, bVerbose](std::size_t uThreadIndex){
				std::map<std::wstring, std::wstring> mapCommandLineReplacements;
				if(!BuildJobs.wcsGCHStub.empty()){
					mapCommandLineReplacements.emplace(L"GCH", L"-include \"" + BuildJobs.wcsGCHStub + L'\"');
				}
				mapCommandLineReplacements.emplace(L"IN", L'\"' + wcsSrcFile + L'\"');
				mapCommandLineReplacements.emplace(L"OUT", L'\"' + wcsObjFile + L'\"');

				const auto wcsCommandLine = MakeCommandLine(
					Project.mapCompilers.at(GetFileExtension(wcsSrcFile)).wcsCommandLine,
					mapCommandLineReplacements,
					L"    "
				);
				if(bVerbose){
					Output(L"    命令行：" + wcsCommandLine);
				} else {
					Output(L"    文件：" + wcsSrcFile);
				}

				const std::size_t uBackSlashPos = wcsObjFile.rfind(L'\\');
				if(uBackSlashPos != std::wstring::npos){
					TouchFolder(wcsObjFile.substr(0, uBackSlashPos));
				}

				ProcessProxy Process;
				Process.Fork(wcsCommandLine);
				const auto pResult = Process.Join();
				{
					LOCK_THROUGH(GetPrintLock());

					wchar_t awchPrefix[8];
					std::swprintf(awchPrefix, COUNTOF(awchPrefix), L"  %-2lu> ", (unsigned long)uThreadIndex);
					CopyStdOutErr(*pResult, awchPrefix);
				}
				if(pResult->nExitCode != 0){
					throw Exception(ERROR_PROCESS_ABORTED, L"编译器返回非零状态码。");
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
				throw Exception(dwError, L"删除文件“" + Project.wcsOutputPath + L"”失败。");
			}
		}

		unsigned long ulPartIndex = 1;

		const auto ExecuteLinker = [](const std::wstring &wcsCommandLine) -> void {
			ProcessProxy Process;
			Process.Fork(wcsCommandLine);
			const auto pResult = Process.Join();
			CopyStdOutErr(*pResult, L"  1>  ");
			if(pResult->nExitCode != 0){
				throw Exception(ERROR_PROCESS_ABORTED, L"链接器返回非零状态码。");
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
			} while(!lstFilesToLink.empty() && (wcsInputFiles.size() < 28 * 0x400));	// Windows 命令行长度上限为 32K 个字符。
			wcsInputFiles.pop_back();

			std::map<std::wstring, std::wstring> mapCommandLineReplacements;
			mapCommandLineReplacements.emplace(L"IN", wcsInputFiles);

			if(!lstFilesToLink.empty()){
				Output(L"    正在执行部分链接...");

				wchar_t achPartialPostfix[32];
				std::swprintf(achPartialPostfix, COUNTOF(achPartialPostfix), L".part%lu.o", ulPartIndex++);
				std::wstring wcsPartialObjPath(Project.wcsOutputPath + achPartialPostfix);

				mapCommandLineReplacements.emplace(L"OUT", L'\"' + wcsPartialObjPath + L'\"');

				const std::wstring wcsCmdLine(MakeCommandLine(
					Project.Linkers.wcsPartial,
					mapCommandLineReplacements,
					L"      "
				));
				if(bVerbose){
					Output(L"    命令行：" + wcsCmdLine);
				}

				std::size_t uBackSlashPos = wcsPartialObjPath.rfind(L'\\');
				if(uBackSlashPos != std::wstring::npos){
					TouchFolder(wcsPartialObjPath.substr(0, uBackSlashPos));
				}
				ExecuteLinker(wcsCmdLine);
				Output(L"      已生成中间文件“" + wcsPartialObjPath + L"”。");

				lstFilesToLink.emplace_back(std::move(wcsPartialObjPath));
			} else {
				Output(L"    正在执行完全链接...");

				mapCommandLineReplacements.emplace(L"OUT", L'\"' + Project.wcsOutputPath + L'\"');

				const std::wstring wcsCmdLine(MakeCommandLine(
					Project.Linkers.wcsFull,
					mapCommandLineReplacements,
					L"      "
				));
				if(bVerbose){
					Output(L"    命令行：" + wcsCmdLine);
				}

				std::size_t uBackSlashPos = Project.wcsOutputPath.rfind(L'\\');
				if(uBackSlashPos != std::wstring::npos){
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
		BuildPreCompiledHeader(BuildJobs, Project, bVerbose);

		Output(L"  正在编译...");
		Compile(BuildJobs, Project, ulProcessCount, bVerbose);

		Output(L"  正在链接...");
		Link(BuildJobs, Project, bVerbose);

		Output(L"  文件“" + Project.wcsOutputPath + L"”构建成功。");
	}
}
