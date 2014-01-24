// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include "UniqueHandle.hpp"
#include <string>
#include <cstdarg>
#include <memory>
#include <vector>
#include <algorithm>
#include <windows.h>
using namespace MCFBuild;

namespace {
	CriticalSection g_PrintLock;
}

namespace MCFBuild {
	int Print(const wchar_t *s, std::size_t len, bool to_stderr){
		int nRet = -1;
		const HANDLE hOut = ::GetStdHandle(to_stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		if(hOut != NULL){
			if(len == (std::size_t)-1){
				len = std::wcslen(s);
			}
			wvstring wcsTemp;
			wcsTemp.reserve(len + 1);
			wcsTemp.append(s, len);
			wcsTemp.push_back(L'\n');

			DWORD dwBytesWritten;
			CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
			if(::GetConsoleScreenBufferInfo(hOut, &ConsoleScreenBufferInfo) != FALSE){
				LOCK_THROUGH(g_PrintLock);

				const std::size_t uColumnsM1 = (std::size_t)ConsoleScreenBufferInfo.srWindow.Right;
				wvstring wcsBlankLine(uColumnsM1 + 1, L' ');
				wcsBlankLine.front() = L'\r';
				wcsBlankLine.back() = L'\r';
				WriteConsoleW(hOut, wcsBlankLine.c_str(), wcsBlankLine.size(), &dwBytesWritten, nullptr);

				nRet = ::WriteConsoleW(hOut, wcsTemp.c_str(), wcsTemp.size(), &dwBytesWritten, nullptr);
			} else {
				const vstring u8sConvtered = WcsToU8s(wcsTemp);
				if(!u8sConvtered.empty()){
					LOCK_THROUGH(g_PrintLock);

					nRet = ::WriteFile(hOut, u8sConvtered.c_str(), u8sConvtered.size(), &dwBytesWritten, nullptr);
				}
			}
			::FlushFileBuffers(hOut);
		}
		return nRet;
	}
	int VPrint(const wchar_t *fmt, bool to_stderr, ...){
		std::va_list ap;

		wchar_t achBuffer[256];

		va_start(ap, to_stderr);
		const int nCount = std::vswprintf(achBuffer, COUNTOF(achBuffer), fmt, ap);
		va_end(ap);

		if(nCount >= 0){
			return Print(achBuffer, (std::size_t)nCount, to_stderr);
		}

		std::unique_ptr<wchar_t> pwchBuffer(new wchar_t[nCount + 1]);

		va_start(ap, to_stderr);
		const int nNewCount = std::vswprintf(pwchBuffer.get(), nCount + 1, fmt, ap);
		va_end(ap);

		if(nNewCount < 0){
			return nNewCount;
		}
		return Print(pwchBuffer.get(), (std::size_t)nNewCount, to_stderr);
	}
	int PrintCR(const wchar_t *s, std::size_t len){
		int nRet = -1;
		const HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut != NULL){
			DWORD dwMode;
			if(::GetConsoleMode(hOut, &dwMode) != FALSE){
				if(len == (std::size_t)-1){
					len = std::wcslen(s);
				}
				wvstring wcsTemp;
				wcsTemp.reserve(len + 1);
				wcsTemp.append(s, len);
				wcsTemp.push_back(L'\r');

				LOCK_THROUGH(g_PrintLock);

				DWORD dwBytesWritten;
				nRet = ::WriteConsoleW(hOut, wcsTemp.c_str(), wcsTemp.size(), &dwBytesWritten, nullptr);
			}
			::FlushFileBuffers(hOut);
		}
		return nRet;
	}
	int VPrintCR(const wchar_t *fmt, ...){
		std::va_list ap;

		wchar_t achBuffer[256];

		va_start(ap, fmt);
		const int nCount = std::vswprintf(achBuffer, COUNTOF(achBuffer), fmt, ap);
		va_end(ap);

		if(nCount >= 0){
			return PrintCR(achBuffer, (std::size_t)nCount);
		}

		std::unique_ptr<wchar_t> pwchBuffer(new wchar_t[nCount + 1]);

		va_start(ap, fmt);
		const int nNewCount = std::vswprintf(pwchBuffer.get(), nCount + 1, fmt, ap);
		va_end(ap);

		if(nNewCount < 0){
			return nNewCount;
		}
		return PrintCR(pwchBuffer.get(), (std::size_t)nNewCount);
	}

	CRITICAL_SECTION &GetPrintLock(){
		return g_PrintLock;
	}

	void FixPath(wvstring &wcsSrc){
		if(!wcsSrc.empty()){
			const DWORD dwSize = ::GetFullPathNameW(wcsSrc.c_str(), 0, nullptr, nullptr);
			wvstring wcsTemp(dwSize, 0);
			wcsTemp.resize(::GetFullPathNameW(wcsSrc.c_str(), wcsTemp.size(), &wcsTemp[0], nullptr));
			wcsSrc = std::move(wcsTemp);
		}
	}

	wvstring GetFileExtension(const wvstring &wcsPath){
		wvstring::const_iterator iterPrev = wcsPath.cend();
		auto iter = iterPrev;
		while(iter != wcsPath.cbegin()){
			switch(*--iterPrev){
			case L'.':
				return wvstring(iter, wcsPath.cend());
			case L'\\':
			case L' ':
				break;
			}
			iter = iterPrev;
		}
		return wvstring();
	}

	wvstring MakeCommandLine(
		const wvstring &wcsBase,
		const std::map<wvstring, wvstring> &mapReplacements,
		const wchar_t *pwszPrefix
	){
		wvstring wcsRet;

		auto iterRead = wcsBase.cbegin();
		while(iterRead != wcsBase.cend()){
			const wchar_t ch = *(iterRead++);
			if(ch != L'%'){
				wcsRet.push_back(ch);
				continue;
			}

			auto iterNameEnd = iterRead;
			for(;;){
				if(iterNameEnd == wcsBase.cend()){
					throw Exception{ERROR_INVALID_DATA, L"“" + wcsBase + L"”中的 % 不匹配。"};
				}
				if(*iterNameEnd == L'%'){
					break;
				}
				++iterNameEnd;
			}
			if(iterNameEnd == iterRead){
				wcsRet.push_back(L'%');
				++iterRead;
				continue;
			}

			const wvstring wcsMacroName(iterRead, iterNameEnd);
			iterRead = ++iterNameEnd;

			const auto iterMacro = mapReplacements.find(wcsMacroName);
			if(iterMacro == mapReplacements.cend()){
				Error((pwszPrefix == nullptr) ? wvstring() : wvstring(pwszPrefix) + L"警告：宏“" + wcsMacroName + L"”没有定义，已替换为空字符串。");
				continue;
			}

			wcsRet.append(iterMacro->second);
		}

		return std::move(wcsRet);
	}

	void TouchFolder(const wvstring &wcsPath){
		const DWORD dwAttributes = GetFileAttributesW(wcsPath.c_str());
		if(dwAttributes == INVALID_FILE_ATTRIBUTES){
			const DWORD dwError = ::GetLastError();
			switch(dwError){
			case ERROR_PATH_NOT_FOUND:
				{
					const std::size_t uBackSlashPos = wcsPath.rfind(L'\\');
					if(uBackSlashPos == wvstring::npos){
						throw Exception{dwError, L"创建目录“" + wcsPath + L"”失败。"};
					}
					TouchFolder(wcsPath.substr(0, uBackSlashPos));
				}
				// 没有 break。
			case ERROR_FILE_NOT_FOUND:
				if(::CreateDirectoryW(wcsPath.c_str(), nullptr) == FALSE){
					const DWORD dwError = ::GetLastError();
					if(dwError != ERROR_ALREADY_EXISTS){
						throw Exception{dwError, L"创建目录“" + wcsPath + L"”失败。"};
					}
				}
				break;
			default:
				throw Exception{dwError, L"获取目录“" + wcsPath + L"”的属性失败。"};
			}
		} else {
			if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
				throw Exception{ERROR_PATH_NOT_FOUND, L"路径“" + wcsPath + L"”不是一个目录。"};
			}
		}
	}

	PROCESS_EXIT_INFO Execute(const wvstring &wcsCmdLine){
		struct HandleCloser {
			constexpr HANDLE operator()(){
				return NULL;
			}
			void operator()(HANDLE hObj){
				::CloseHandle(hObj);
			}
		};

		PROCESS_EXIT_INFO ExitInfo;

		std::vector<wchar_t> vecCmdLine;
		vecCmdLine.reserve(wcsCmdLine.size() + 1);
		vecCmdLine.insert(vecCmdLine.end(), wcsCmdLine.cbegin(), wcsCmdLine.cend());
		vecCmdLine.push_back(0);

		// 0 = 读， 1 = 写。
		const auto CreateInputPipe = [](){
			std::pair<MCF::UniqueHandle<HANDLE, HandleCloser>, MCF::UniqueHandle<HANDLE, HandleCloser>> HandlesRet;
			HANDLE hRead, hWrite;
			if(::CreatePipe(&hRead, &hWrite, nullptr, 0) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"创建无名管道失败。"};
			}
			HandlesRet.first.Reset(hRead);
			HandlesRet.second.Reset(hWrite);
			if(::SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"设置管道句柄信息失败。"};
			}
			return std::move(HandlesRet);
		};
		auto StdOutPipe(CreateInputPipe());
		auto StdErrPipe(CreateInputPipe());

		STARTUPINFOW StartupInfo;
		StartupInfo.cb			= sizeof(StartupInfo);
		StartupInfo.lpReserved	= nullptr;
		StartupInfo.lpDesktop	= nullptr;
		StartupInfo.lpTitle		= nullptr;
		StartupInfo.dwFlags		= STARTF_USESTDHANDLES | DETACHED_PROCESS;
		StartupInfo.cbReserved2	= 0;
		StartupInfo.lpReserved2	= nullptr;
		StartupInfo.hStdInput	= INVALID_HANDLE_VALUE;
		StartupInfo.hStdOutput	= StdOutPipe.second;
		StartupInfo.hStdError	= StdErrPipe.second;

		PROCESS_INFORMATION ProcessInfo;
		if(::CreateProcessW(nullptr, vecCmdLine.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo) == FALSE){
			const DWORD dwError = ::GetLastError();
			throw Exception{dwError, L"使用命令行“" + wcsCmdLine + L"”创建进程失败。"};
		}
		::CloseHandle(ProcessInfo.hThread);
		MCF::UniqueHandle<HANDLE, HandleCloser> hProcess(ProcessInfo.hProcess);

		bool bQuitNow = false;
		do {
			if(::WaitForSingleObject(hProcess, 5) != WAIT_TIMEOUT){
				::GetExitCodeProcess(hProcess, &ExitInfo.dwExitCode);
				bQuitNow = true;
			}

			const auto ReadPipe = [](vstring &strSink, HANDLE hSource) -> void {
				char achBuffer[0x400];
				DWORD dwBytesAvail;
				DWORD dwBytesRead;

				for(;;){
					if(::PeekNamedPipe(hSource, nullptr, 0, nullptr, &dwBytesAvail, nullptr) == FALSE){
						break;
					}
					if(dwBytesAvail == 0){
						break;
					}
					if(::ReadFile(hSource, achBuffer, sizeof(achBuffer), &dwBytesRead, nullptr) == FALSE){
						break;
					}
					strSink.append(achBuffer, dwBytesRead);
				}
			};
			ReadPipe(ExitInfo.strStdOut, StdOutPipe.first);
			ReadPipe(ExitInfo.strStdErr, StdErrPipe.first);
		} while(!bQuitNow);

		const auto ZapCRLFs = [](vstring &strSource){
			std::size_t uWritePos = 0;
			char chNext = strSource[0];
			for(std::size_t i = 0; i < strSource.size(); ++i){
				const char chCur = chNext;
				chNext = strSource[i + 1];	// 对于 string 这是合法的。
				if((chCur == '\r') && (chNext == '\n')){
					continue;
				}
				strSource[uWritePos++] = chCur;
			}
			strSource.resize(uWritePos);
		};
		ZapCRLFs(ExitInfo.strStdOut);
		ZapCRLFs(ExitInfo.strStdErr);

		return std::move(ExitInfo);
	}
}
